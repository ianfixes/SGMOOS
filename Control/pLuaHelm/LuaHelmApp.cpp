/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Ian Katz & others at MIT Sea Grant.
    contact: ijk5@mit.edu

    This file is part of SGMOOS.

    SGMOOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SGMOOS.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/
//
// LuaHelmApp.cpp - implementation of the CLuaHelm class
//
//////////////////////////////////////////////////////////////////////

#include "LuaHelmApp.h"
//using namespace std;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



/**
 * An app class for controlling the vehicle with lua scripts
 */
CLuaHelmApp::CLuaHelmApp() : CMOOSApp()
{
    //m_bInitialised  =   false;

    //Use CMOOSApp's command message filtering feature
    EnableCommandMessageFiltering(true);

    //some sensible defaults (missionfile can overwrite this)
    SetAppFreq(5);
    SetCommsFreq(15);
    SortMailByTime(true);

    SetThisHelm(this);

    //TODO: re enable lastActionTime and ActionTimeout?
    //m_dfLastActionTime = -1000.0;
    //m_dfActionTimeout = 3.0;

    m_ConfigCurrent   = NULL;
    m_ConfigNext      = NULL;
    m_MissionCurrent  = NULL;
    m_MissionNext     = NULL;

    m_bMissionReady    = false;
    m_bCommandHandled  = true;

    m_sMissionToLoad = "";
}

CLuaHelmApp::~CLuaHelmApp()
{
    if (m_ConfigCurrent) delete m_ConfigCurrent;
    if (m_ConfigNext) delete m_ConfigNext;
    if (m_MissionCurrent) delete m_MissionCurrent;
    if (m_MissionNext) delete m_MissionNext;
}


//this is a kludge for the state machine because C++ doesn't support
// pointers to class member functions.  luckily, we only have one 
// helm object at a time.
CLuaHelmApp* CLuaHelmApp::thisHelm;
void CLuaHelmApp::SetThisHelm(CLuaHelmApp* t)
{
    CLuaHelmApp::thisHelm = t;
}

//build the master state machine that controls helm operation
// this sets up the state names and -- more importantly -- the commands
//  that control the state changes
void CLuaHelmApp::CreateMachineMain()
{
    CState* init = new CState("Init", &CLuaHelmApp::LogIdle, "Idle");

    CState* idle = new CState("Idle", &CLuaHelmApp::NoOp, "Idle");
    idle->AddTransition("mark",        &CLuaHelmApp::NoOp,     "Init");
    idle->AddTransition("execute",     &CLuaHelmApp::ExecuteIdle,  "Ready");
    idle->AddTransition("reconfigure", &CLuaHelmApp::LoadCfg,  "ApplyConf");
    
    CState* apply = new CState("ApplyConf", &CLuaHelmApp::ApplyCfg, "Idle");
                               
    CState* ready = new CState("Ready", &CLuaHelmApp::LogActive, "PrepRun");

    CState* preprun = new CState("PrepRun", &CLuaHelmApp::PrepareRun, "Run");

    CState* run = new CState("Run", &CLuaHelmApp::IterateRun, "PrepSurface");
    run->AddTransition("repeat",   &CLuaHelmApp::LogActive,          "PrepRun");
    run->AddTransition("stop",     &CLuaHelmApp::ReRegSurface,       "PrepSurface");
    run->AddTransition("execute",  &CLuaHelmApp::ExecuteRun,         "Continue");
    run->AddTransition("abort",    &CLuaHelmApp::UnregisterMission,  "Init");
    
    CState* cont = new CState("Continue", &CLuaHelmApp::InitRegister, "Ready");

    CState* prepsurface = new CState("PrepSurface", &CLuaHelmApp::PrepareSurface, "Surface");
    
    CState* surface = new CState("Surface", &CLuaHelmApp::IterateSurface, "Init");
    surface->AddTransition("execute",  &CLuaHelmApp::ExecuteSurf,       "Ready");
    surface->AddTransition("abort",    &CLuaHelmApp::UnregisterSurface, "Init");

    //LOAD 'EM UP
    m_MachineMain.AddState(init);
    m_MachineMain.AddState(idle);
    m_MachineMain.AddState(apply);
    m_MachineMain.AddState(ready);
    m_MachineMain.AddState(preprun);
    m_MachineMain.AddState(run);
    m_MachineMain.AddState(cont);
    m_MachineMain.AddState(prepsurface);
    m_MachineMain.AddState(surface);
    
    m_MachineMain.Start("Init");
}

/**
 * Helm Initialization
 * 
 * Make sure all config params are available, print an error if not
 * load all member variables 
 * init the camera driver
 */
bool CLuaHelmApp::OnStartUp()
{
    bool bRet = true;

    MOOSTrace("Creating main state machine...");
    CreateMachineMain();
    MOOSTrace("Done.\n");

    if (!m_MachineMain.IsValid())
    {
        bRet = false;
        MOOSTrace("Main state machine is not valid");
    }

    //we need to have talked to the DB to get our skew
    //time before this as tasks need a start time
    //this is unusual behaviour for a MOOS process but
    //does mean things are safe...
    int nCount = 0;
    while(!m_Comms.IsConnected())
    {
        MOOSTrace("Waiting for connect: try %d\n", nCount);
        MOOSPause(1000);
        if(nCount++ > 30)
        {
            MOOSTrace("Cannot initialise helm without connecting to Server\n");
            MOOSTrace("Waited 30 seconds..quiting\n");
            return false;
        }
    }

    //read config and swap it into the current config
    MOOSTrace("Reading initial config\n");
    if (!ReloadConfig())
    {
        bRet = false;
    }
    else
    {
        MOOSTrace("On-deck config loaded successfully\n");
        SwapConfig();
        MOOSTrace("On-deck config swapped to current\n"); 
        
        RescanMissions();
    }

    m_bInitialised = true;

    
    if (!bRet)
    {
        MOOSTrace("\n------------- pLuaHelm's failure is now complete.\n\n");
    }

    return bRet;
}

//at bare minimum, these are the registrations needed by this app
STRING_SET CLuaHelmApp::CoreRegistrations()
{
    STRING_SET s;

    string v = thisHelm->GetAppName() + "_CMD";
    MOOSToUpper(v);

    s.insert(v);
    s.insert("LUAHELM_NEXTMISSION");

    return s;
}

//go through the directory of missions and make a list of them
void CLuaHelmApp::RescanMissions()
{
    struct dirent **namelist;
    string output = "";
    int n;
    
    n = scandir(m_ConfigCurrent->LuaMissionDir().c_str(), &namelist, 0, alphasort);
    if (n < 0)
    {
        string oops = MOOSFormat("Error scanning directory '%s'",
                                 m_ConfigCurrent->LuaMissionDir().c_str());
        
        DTrace("LUAHELM_LOADERMESSAGE", oops);
    }
    else 
    {
        int i = 0;
        while(i < n) 
        {
            //skip dotfiles
            if ('.' != namelist[i]->d_name[0])
            {
                //filter for .lua files, and remove extension
                string full = namelist[i]->d_name;
                if (".lua" == full.substr(full.length() - 4))
                {
                    output += full.substr(0, full.length() - 4) + ",";
                }
            }
            ++i;
        }

        //free the dirent list
        while (n--)
        {
            free(namelist[n]);
        }
        free(namelist);
    }
    
    //post output and move on
    m_Comms.Notify("LUAHELM_MISSIONS", output);
    
}

bool CLuaHelmApp::OnConnectToServer()
{
    //cmd variable already registered

    //core vars
    STRING_SET cr = CoreRegistrations();
    for(STRING_SET::iterator it = cr.begin(); it != cr.end(); it++)
    {
        m_Comms.Register(*it, 0.1);
    }

    //Register for component variables
    MOOSTrace("\n    Checking config for variable registration requests\n");

    if (m_ConfigCurrent)
    {
        STRING_SET Reg = m_ConfigCurrent->Registrations();
        for(STRING_SET::iterator r = Reg.begin(); r != Reg.end(); r++)
        {
            m_Comms.Register(*r, 0.1);
        }
    }
    MOOSTrace("    pLuaHelm's registrations are done\n  Paul Newman says: ");
    return true;
}

//work with the state machine we set up earlier
// if we are in a state where a mission gets run, give that mission a tick!
bool CLuaHelmApp::Iterate()
{

    if(!m_bInitialised)
    {
        MOOSTrace("CLuaHelmApp::Iterate() FAIL : Helm not initialised\n");
        return false;
    }

    //call loadmission.  
    //the chances of this hitting simultaneously with the "activate" command
    //  are assumed to be minimal.  this would clobber the loadermessage, nbd.
    if ("" != m_sMissionToLoad)
    {
        if (LoadMission(m_sMissionToLoad))
        {
            m_Comms.Notify("LUAHELM_LOADERMESSAGE", "On-deck mission loaded successfully");
        }
        m_sMissionToLoad = "";
    }


    string sEvent;

    //execute main state machine  
    if (m_bCommandHandled)
    {
        sEvent = "[ tick ]";
    }
    else if ("activate" == m_sCommand)
    {
        //activate mission
        if (m_MissionNext && m_MissionNext->IsValid())
        {
            DTrace("LUAHELM_LOADERMESSAGE", "Activated on-deck mission");
            m_bMissionReady = true;
        }
        else
        {
            DTrace("LUAHELM_LOADERMESSAGE", "Invalid on-deck mission, not activated");
        }
        sEvent = "[ tick ]";
    }
    else if ("rescan" == m_sCommand)
    {

        RescanMissions();

        sEvent = "[ tick ]";
        
    }
    else 
    {
        MOOSTrace("BOIIING\n");
        sEvent = m_sCommand;
    }
    

    //feel like we should publish the command, but ... it's input
    m_MachineMain.Iterate(sEvent);
    m_bCommandHandled = true;
    

    //publish current state
    m_Comms.Notify("LUAHELM_STATE",      m_MachineMain.CurrentState());


    //enumerate next actions and publish
    string sValidActions = "rescan,";
    
    //if we can activate a mission
    if (!m_bMissionReady && m_MissionNext && m_MissionNext->IsValid())
    {
        sValidActions += "activate,";
    }

    //from state machine
    STRING_LIST theActions = m_MachineMain.ValidActions();
    for (STRING_LIST::iterator a = theActions.begin(); a != theActions.end(); a++)
    {
        sValidActions += *a;
        sValidActions += ",";
    }
    
    m_Comms.Notify("LUAHELM_NEXTACTIONS", sValidActions);

    //we have processed our mail for sure, so clear it.
    m_NewMail.clear();

    return true;
}



bool CLuaHelmApp::OnCommandMsg(CMOOSMsg CmdMsg)
{
    if(CmdMsg.IsSkewed(MOOSTime()))
    {
        MOOSTrace("Got Skewed Command");
        return false;
    }

    //set a flag that says we received a command during this tick
    m_bCommandHandled = false;
    m_sCommand = CmdMsg.m_sVal;
    MOOSTrace("Received command: %s\n", m_sCommand.c_str());

    return true;
}



bool CLuaHelmApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
    string sMessage;
    CMOOSMsg Msg;

    //this is oldest-first, so push onto the back of our mail list
    for (MOOSMSG_LIST::iterator it = NewMail.begin(); it != NewMail.end(); ++it) 
    {
        //MOOSTrace("Got mail %s at %f\n", it->GetName().c_str(), it->GetTime());
        m_NewMail.push_back(*it);
    }


    //actuation map is simpler, just a simple map of mail to its latest val
    STRING_LIST::iterator s;
    STRING_LIST regs = m_ConfigCurrent->ActuationMap()->Registrations();

    m_ActuatorMail.clear();
    for (s = regs.begin(); s != regs.end(); s++)
    {
        if (m_Comms.PeekMail(NewMail, *s, Msg))
        {
            m_ActuatorMail[*s] = Msg;
        }
    }

    //process the next mission if we have it
    if(m_Comms.PeekMail(NewMail,"LUAHELM_NEXTMISSION", Msg))
    {
        double dfNow = MOOSTime();

        if(!Msg.IsSkewed(dfNow))
        {
            m_sMissionToLoad = Msg.GetString();
        }
    }
    
    //update PIDs
    m_ConfigCurrent->MailPIDs(NewMail);

    return true;
}

//print an error to the console, and note that the lua environment is in error
void CLuaHelmApp::Lua_OnErr(string source, string message)
{
    string msg = MOOSFormat("Error message from %s:\n    %s\n",
                             source.c_str(), message.c_str());

    DTrace("LUAHELM_LOADERMESSAGE", msg);

    thisHelm->m_bLuaAPIerror = true;
}

//print error to console only
void CLuaHelmApp::Lua_OnErrSandbox(string source, string message)
{
    MOOSTrace("Error message (sandbox) from %s:\n%s\n", 
              source.c_str(), message.c_str());
}

//DOUBLE TRACE... log error to console AND our moos var
void CLuaHelmApp::DTrace(string sMoosvar, string sMessage)
{
    thisHelm->m_Comms.Notify(sMoosvar, sMessage);
    MOOSTrace("%s\n", sMessage.c_str());
}

//lua can call this function with multiple args, which get printed to screen
int CLuaHelmApp::API_Trace(lua_State* L)
{
    int argc = lua_gettop(L);

    MOOSTrace("[lua] ");
    for (int n = 1; n <= argc; ++n)
    {
        MOOSTrace("%s ", lua_tostring(L, n));
    }
    MOOSTrace("\n");

    lua_pushnumber(L, LAPI_SUCCESS);
    return 1;
}

//lua can call this function to get the MOOSTime
int CLuaHelmApp::API_Time(lua_State* L)
{
    //return only
    lua_pushnumber(L, MOOSTime());
    lua_pushnumber(L, LAPI_SUCCESS);

    return 2;
}

//dummy function for sandbox lua API, which just says what arguments got passed
int CLuaHelmApp::SAPI_General(lua_State* L, string func)
{
    int argc = lua_gettop(L);

    printf("-- %s() called with %d arguments\n", func.c_str(), argc);

    for (int n = 1; n <= argc; ++n)
    {
        printf("-- argument %d: %s\n", n, lua_tostring(L, n));
    }

    //fake success
    lua_pushnumber(L, LAPI_SUCCESS);
    return 1;

}

//check that the proper number of args have been supplied 
// numargs is how many arguments we SHOULD get
//
//returns true/false to C, and sends some error code back to lua too
bool CLuaHelmApp::CheckLuaArgs(lua_State* L, string func, int numargs)
{
    int argc = lua_gettop(L);
    
    if (numargs == argc)
    {
        return true;
    }

    thisHelm->m_bLuaAPIerror = true;
    MOOSTrace("Lua/C++ xAPI_%s() expected %d args, got %d\n", func.c_str(), numargs, argc);
    lua_pushnumber(L, LAPI_ARGC_FAIL);
    return false;
}

//range check for selected DOF
bool CLuaHelmApp::CheckDOF(lua_State* L, WhichDOF nDirection)
{
    if (WhichDOF_min <= nDirection && nDirection <= WhichDOF_max)
    {
        return true;
    }

    thisHelm->m_bLuaAPIerror = true;
    MOOSTrace("Error: index out of range for DOF (got %d)\n", nDirection);
    lua_pushnumber(L, LAPI_RANGE_FAIL);
    return false;
}

//allows lua to set a closed loop force
int CLuaHelmApp::MAPI_SetClosedLoop(lua_State* L)
{
    if (!CheckLuaArgs(L, "SetClosedLoop", 5)) return 1;

    WhichDOF nDirection = (WhichDOF)lua_tonumber(L, 1);
    double dfError      = lua_tonumber(L, 2);
    double dfGoal       = lua_tonumber(L, 3);
    int nPriority       = (int)lua_tonumber(L, 4);
    string sTag         = lua_tostring(L, 5);

    if (!CheckDOF(L, nDirection)) return 1;
    
    thisHelm->m_PathAction->SetClosedLoop(nDirection, 
                                          dfError, 
                                          dfGoal, 
                                          nPriority, 
                                          sTag.c_str());


    lua_pushnumber(L, LAPI_SUCCESS);
    return 1;
}

//allows lua to set an open loop force
int CLuaHelmApp::MAPI_SetOpenLoop(lua_State* L)
{
    if (!CheckLuaArgs(L, "SetOpenLoop", 4)) return 1;

    WhichDOF nDirection = (WhichDOF)lua_tonumber(L, 1);
    double dfVal        = lua_tonumber(L, 2);
    int nPriority       = (int)lua_tonumber(L, 3);
    string sTag         = lua_tostring(L, 4);

    if (!CheckDOF(L, nDirection)) return 1;

    thisHelm->m_PathAction->SetOpenLoop(nDirection, 
                                        dfVal, 
                                        nPriority, 
                                        sTag.c_str());

    lua_pushnumber(L, LAPI_SUCCESS);
    return 1;
}

//allows lua to post a message from the mission
int CLuaHelmApp::MAPI_MissionMessage(lua_State* L)
{
    if (!CheckLuaArgs(L, "MissionMessage", 1)) return 1;

    string sMessage = lua_tostring(L, 1);
    thisHelm->m_Comms.Notify("LUAMISSION_MESSAGE", sMessage);

    lua_pushnumber(L, LAPI_SUCCESS);
    return 1;
}

//allows lua to say what state things are in
int CLuaHelmApp::MAPI_MissionState(lua_State* L)
{
    if (!CheckLuaArgs(L, "MissionState", 1)) return 1;

    string sMessage = lua_tostring(L, 1);
    thisHelm->m_Comms.Notify("LUAMISSION_STATE", sMessage);

    lua_pushnumber(L, LAPI_SUCCESS);
    return 1;
}

//allows lua to post a string to the db
int CLuaHelmApp::MAPI_PostString(lua_State* L)
{
    if (!CheckLuaArgs(L, "PostString", 2)) return 1;

    //API call is (var, val)
    string sVar = lua_tostring(L, 1);
    string sVal = lua_tostring(L, 2);

    thisHelm->m_Comms.Notify(sVar, sVal);

    lua_pushnumber(L, LAPI_SUCCESS);
    return 1;
}
    
//allows lua to post a number to the moos DB
int CLuaHelmApp::MAPI_PostDouble(lua_State* L)
{
    if (!CheckLuaArgs(L, "PostDouble", 2)) return 1;

    //API call is (var, val)
    string sVar = lua_tostring(L, 1);
    float  sVal = lua_tonumber(L, 2);

    thisHelm->m_Comms.Notify(sVar, sVal);

    lua_pushnumber(L, LAPI_SUCCESS);
    return 1;
}

//the set of functions that comprise the sandbox
LUA_API_MAP CLuaHelmApp::LuaApiSandbox()
{
    LUA_API_MAP ret;

    ret["HAPI_Trace"]           =  &API_Trace;
    ret["HAPI_Time"]            =  &API_Time;
    ret["HAPI_SetClosedLoop"]   = &SAPI_SetClosedLoop;
    ret["HAPI_SetOpenLoop"]     = &SAPI_SetOpenLoop;
    ret["HAPI_MissionMessage"]  = &SAPI_MissionMessage;
    ret["HAPI_MissionState"]    = &SAPI_MissionState;
    ret["HAPI_PostString"]      = &SAPI_PostString;
    ret["HAPI_PostDouble"]      = &SAPI_PostDouble;

    return ret;
}

//the set of functions that actually control the vehicle
LUA_API_MAP CLuaHelmApp::LuaApiMOOS()
{
    LUA_API_MAP ret;

    ret["HAPI_Trace"]           =  &API_Trace;
    ret["HAPI_Time"]            =  &API_Time;
    ret["HAPI_SetClosedLoop"]   = &MAPI_SetClosedLoop;
    ret["HAPI_SetOpenLoop"]     = &MAPI_SetOpenLoop;
    ret["HAPI_MissionMessage"]  = &MAPI_MissionMessage;
    ret["HAPI_MissionState"]    = &MAPI_MissionState;
    ret["HAPI_PostString"]      = &MAPI_PostString;
    ret["HAPI_PostDouble"]      = &MAPI_PostDouble;

    return ret;
}

//re-read the moos config file, but put the resulting config 
//    in a separate variable from current config
bool CLuaHelmApp::ReloadConfig()
{
    if (m_ConfigNext) 
    {
        delete m_ConfigNext;
        m_ConfigNext = NULL;
    }

    m_ConfigNext = new CHelmConfig(m_sMissionFile, &m_MissionReader);

    bool ret = m_ConfigNext->Reload(LuaApiSandbox(), &CLuaHelmApp::Lua_OnErrSandbox);

    if (!ret)
    {
        DTrace("LUAHELM_MESSAGE", "New config was bad; keeping existing config\n");
        delete m_ConfigNext;
        m_ConfigNext = NULL;
    }

    return ret;
}

//get rid of the current configuration and replace it with a previously-loaded one
void CLuaHelmApp::SwapConfig()
{
    //if there is a current config, check the mysql
    if (m_ConfigCurrent)
    {
        //do any mysql changes
        if (m_ConfigCurrent->CommandLogging() && !m_ConfigNext->CommandLogging())
        {
            //stop
            m_Comms.Notify(m_ConfigCurrent->LoggingCmdVar(), m_ConfigCurrent->CmdLogStop());
        }
        
        if (!m_ConfigCurrent->CommandLogging() && m_ConfigNext->CommandLogging())
        {
            //logidle
            m_Comms.Notify(m_ConfigCurrent->LoggingCmdVar(), m_ConfigCurrent->CmdLogIdle());
        }
    }

    //unsubscribe and resusbscribe
    if (NULL != m_ConfigCurrent)
    {
        MOOSTrace("Unsubscribing from variables required by current (now old) config:\n");
        Unregister(m_ConfigCurrent->Registrations());
    }
    
    MOOSTrace("Subscribing to variables required by new config:\n");
    STRING_SET Reg = m_ConfigNext->Registrations();
    for(STRING_SET::iterator r = Reg.begin(); r != Reg.end(); r++)
    {
        //MOOSTrace("Registering for %s: %s\n", r->c_str(), 
        thisHelm->m_Comms.Register(*r, 0.1) ; //? "true" : "false");
    }


    //ooh, danger as we momentarily operate with no configuration!
    if (NULL != m_ConfigCurrent) delete m_ConfigCurrent;
    m_ConfigCurrent = m_ConfigNext;
    m_ConfigNext = NULL;
    
    //give current config's lua missions the live API
    m_ConfigCurrent->UpdateLuaAPI(LuaApiMOOS(), &CLuaHelmApp::Lua_OnErr);

}

//try to load a mission and return pass/fail
bool CLuaHelmApp::LoadMission(string sMission)
{
    
    MOOSTrace("Instructed to load mission %s\n", sMission.c_str());

    m_bMissionReady = false; //start over; mission will need activation
    if (m_MissionNext) delete m_MissionNext;
            
    m_MissionNext = new CLuaMission("MISSION", 
                                    LuaApiSandbox(), 
                                    Lua_OnErrSandbox, 
                                    m_ConfigCurrent->LuaLibDir());

    string sFile = m_ConfigCurrent->LuaMissionFilePath(sMission);
    string sPrefix = "Loading " + sFile + " failed: ";

    //success returns 0
    if (int code = m_MissionNext->Load(sFile))
    {
        return LoadMissionFail(sPrefix 
                               + CLuaMission::LuaErrorDesc(code) 
                               + ", "
                               + m_MissionNext->LastLoadError());
    }

    if (!m_MissionNext->IsValid())
    {
        return LoadMissionFail(sPrefix + "Required functions are missing");
    }

    if (!m_MissionNext->Init())
    {
        return LoadMissionFail(sPrefix + "Mission's Init() returned false");
    }

    if (!m_MissionNext->Prepare())
    {
        return LoadMissionFail(sPrefix + "Mission's Prepare() returned false");
    }

    string graph = m_MissionNext->ToGraph();
    m_Comms.Notify("LUAHELM_MISSIONCHART", graph);
    return true;
}

//print out why the mission failed, and delete the mission object
bool CLuaHelmApp::LoadMissionFail(string sMessage)
{
    //NOTIFY 
    DTrace("LUAHELM_LOADERMESSAGE", sMessage);

    //DELETE the mission
    delete m_MissionNext;
    m_MissionNext = NULL;

    //always return false... we do this to save a line in calls (man are we lazy)
    return false;
}

//static wrapper for reload config function
bool CLuaHelmApp::LoadCfg()
{
    MOOSTrace("User requested reload of config\n");
    return thisHelm->ReloadConfig();
}

//static wrapper for applying config
bool CLuaHelmApp::ApplyCfg()
{
    MOOSTrace("Swapping new config into place...\n");
    thisHelm->SwapConfig();
    DTrace("LUAHELM_MESSAGE", "Swapped new config into place\n");
    return true;
}

//static wrapper for registering mission variables
bool CLuaHelmApp::RegisterMission()
{
    STRING_SET Reg;
    
    thisHelm->m_MissionCurrent->Registrations(Reg);
    thisHelm->m_ConfigCurrent->SafetyMission()->Registrations(Reg);

    for(STRING_SET::iterator r = Reg.begin(); r != Reg.end(); r++)
    {
        MOOSTrace("Registering for %s: %s\n", r->c_str(), 
                  thisHelm->m_Comms.Register(*r, 0.1) ? "true" : "false");
    }

    return true;
}

//static wrapper for registering surfacing-mission variables
bool CLuaHelmApp::RegisterSurface()
{
    STRING_SET Reg;
    
    thisHelm->m_ConfigCurrent->SurfaceMission()->Registrations(Reg);

    for(STRING_SET::iterator r = Reg.begin(); r != Reg.end(); r++)
    {
        MOOSTrace("Registering for %s: %s\n", r->c_str(), 
                  thisHelm->m_Comms.Register(*r, 0.1) ? "true" : "false");
    }

    return true;
}

//initialize mission and register for variables
bool CLuaHelmApp::InitRegister()
{
    MOOSTrace("InitRegister()\n");

    //don't check retval because it was already validated in the sandbox
    thisHelm->m_MissionCurrent->Init();
    
    return RegisterMission();
}

//go into the idle state; tell logger
bool CLuaHelmApp::LogIdle() 
{
    MOOSTrace("LogIdle()\n");

    if (thisHelm->m_ConfigCurrent->CommandLogging())
    {
        thisHelm->m_Comms.Notify(thisHelm->m_ConfigCurrent->LoggingCmdVar(), 
                                 thisHelm->m_ConfigCurrent->CmdLogIdle());
    }

    return true;
}

//go into an active mission state; tell logger
bool CLuaHelmApp::LogActive()
{
    MOOSTrace("LogActive()\n");

    if (thisHelm->m_ConfigCurrent->CommandLogging())
    {
        thisHelm->m_Comms.Notify(thisHelm->m_ConfigCurrent->LoggingCmdVar(), 
                                 thisHelm->m_ConfigCurrent->CmdLogStart() + "," 
                                 + thisHelm->m_MissionCurrent->Name());
    }

    return true;
}

//if we are switching to a new mission, handle that
bool CLuaHelmApp::ProcessMissionActivation()
{
    MOOSTrace("ProcessMissionActivation()\n");

    if (m_bMissionReady)
    {
        if (m_MissionCurrent) delete m_MissionCurrent;
        m_MissionCurrent = m_MissionNext;
        m_MissionNext = NULL;
        m_MissionCurrent->SwitchAPI(LuaApiMOOS(), &CLuaHelmApp::Lua_OnErr);

        m_Comms.Notify("MISSION_FILE", 
                       m_ConfigCurrent->LuaMissionDir() + m_MissionCurrent->Filename());
        
        m_bMissionReady = false;
    }  
    
    return NULL != m_MissionCurrent;
}

//transition from idle state to executing state
bool CLuaHelmApp::ExecuteIdle()
{
    MOOSTrace("ExecuteIdle()\n");

    //if not loaded then return false else init-run and register vars
    if (!thisHelm->ProcessMissionActivation())
    {
        DTrace("LUAHELM_MESSAGE", "No activated mission, returning to Idle state");
        return false;
    }

    return InitRegister();

}

//transition from executing state to another executing state
bool CLuaHelmApp::ExecuteRun()
{
    MOOSTrace("ExecuteRun()\n");
    
    //FIXME: does this go AFTER the if block???
    UnregisterMission();

    //if not loaded then return false else unregister
    if (!thisHelm->ProcessMissionActivation())
    {
        //yeah... like somehow we are running a mission but haven't activated one???
        DTrace("LUAHELM_MESSAGE", "No activated mission, resuming current mission run");
        RegisterMission();
        return false;
    }

    return true;
}

//transition from executing state to the surfacing state
bool CLuaHelmApp::ExecuteSurf()
{
    MOOSTrace("ExecuteSurf()\n");

    //FIXME: is this block necessary?  it might only be necessary to execute a new mission during surface state
    //if not loaded then return false else unregister
    if (!thisHelm->ProcessMissionActivation())
    {
        //this will NEVER happen...
        DTrace("LUAHELM_MESSAGE", "No activated mission, resuming surfacing mission");
        return false;
    }

    UnregisterSurface();
    InitRegister();

    return true;
}

//unsubscribe from mission-specific variables, keep core subscriptions around
bool CLuaHelmApp::UnregisterMission()
{
    MOOSTrace("UnregisterMission()\n");

    //TODO: allstop?

    STRING_SET regs;

    //load up regs
    thisHelm->m_ConfigCurrent->SafetyMission()->Registrations(regs);
    thisHelm->m_MissionCurrent->Registrations(regs);

    //do it, but don't remove any of the actuation map / PID stuff
    return Unregister(SetSubtract(regs, thisHelm->m_ConfigCurrent->Registrations()));
}

//unsubscribe from surface-specific variables
bool CLuaHelmApp::UnregisterSurface()
{
    STRING_SET regs;

    //load up regs
    thisHelm->m_ConfigCurrent->SurfaceMission()->Registrations(regs);

    //do it, but don't remove any of the actuation map / PID stuff
    return Unregister(SetSubtract(regs, thisHelm->m_ConfigCurrent->Registrations()));
}


//remove subtrahend from minuend.
STRING_SET CLuaHelmApp::SetSubtract(STRING_SET minuend, STRING_SET subtrahend)
{
    for (STRING_SET::iterator it = subtrahend.begin(); it != subtrahend.end(); it++)
    {
        minuend.erase(*it);
    }
   
    return minuend;
}


//unregister from the given set of variables
bool CLuaHelmApp::Unregister(STRING_SET vars)
{
    //don't want to unregister from any core vars
    SetSubtract(vars, thisHelm->CoreRegistrations());

    //acutally unregister
    for (STRING_SET::iterator it = vars.begin(); it != vars.end(); it++)
    {
        thisHelm->m_Comms.UnRegister(*it);
    }

    return true;
}

//switch registrations from mission to surface
bool CLuaHelmApp::ReRegSurface()
{
    return UnregisterMission() && RegisterSurface();
}

//prepare missions and reset error flags
bool CLuaHelmApp::PrepareRun()
{
    MOOSTrace("PrepareRun()\n");

    // already checked the mission in the sandbox, so we can assume it's still good
    thisHelm->m_bLuaAPIerror = false;

    //FIXME: what is going on here with return values?
    if (!thisHelm->m_ConfigCurrent->SafetyMission()->Prepare())
    {
        DTrace("LUAHELM_MESSAGE", "Safety mission failed to prepare");
    }
    
    if (!thisHelm->m_MissionCurrent->Prepare())
    {
        DTrace("LUAHELM_MESSAGE", "Mission failed to prepare");
        return true;
    }

    return true;
}

//check whether to end a mission, or whether the act of checking produces an error...
// then produce some side effects.
// ONLY CALL THIS FROM an IterateXXX() function
bool CLuaHelmApp::MustEndMission(CLuaMission* mission, bool (*action)(void), string message)
{
    bool tmp;

    //check mission for successful run, but check API stuff first
    tmp = mission->IsDone();
    if (thisHelm->m_bLuaAPIerror)
    {
        DTrace("LUAHELM_MESSAGE",
               "Bad call made by Lua " + mission->Identifier() + " IsDone() -- terminating");
        (action)();
        return true;
    }
    if (tmp)
    {
        DTrace("LUAHELM_MESSAGE", message);
        (action)();
        return true;
    }

    return false;
}

//check whether a mission tick produced an error
// ONLY CALL THIS FROM an IterateXXX() function
bool CLuaHelmApp::MissionTickFailed(CLuaMission* mission, bool (*action)(void))
{
    mission->Tick(thisHelm->m_NewMail);
    if (thisHelm->m_bLuaAPIerror)
    {
        DTrace("LUAHELM_MESSAGE",
               "Bad call made by Lua " + mission->Identifier() + " Tick(), -- terminating");
        (action)();
        return true;
    }

    return false;
}

//execute an actual tick of the loaded mission
//return true if we are done running
bool CLuaHelmApp::IterateRun()
{
    MOOSTrace("IterateRun()\n");

    //note that we check for errors in the safety mission first, otherwise
    //they would not become apparent until they were actually NEEDED.

    if (MustEndMission(thisHelm->m_ConfigCurrent->SafetyMission(), 
                       &CLuaHelmApp::ReRegSurface,
                       "Safety mission has fired!")
        || 
        MustEndMission(thisHelm->m_MissionCurrent, 
                       &CLuaHelmApp::ReRegSurface,
                       "Mission says it's done!"))
    {
        return true;
    }

    //iterate by creating a path action, using lua to fill it, then processing it
    thisHelm->m_PathAction = new CPathAction;

    if (MissionTickFailed(thisHelm->m_ConfigCurrent->SafetyMission(), &CLuaHelmApp::ReRegSurface)
        ||
        MissionTickFailed(thisHelm->m_MissionCurrent, &CLuaHelmApp::ReRegSurface))
    {
        return true;
    }

    thisHelm->ProcessPIDs(); //uses path action
    delete thisHelm->m_PathAction;
    thisHelm->m_PathAction = NULL;

    return false;
}

//prepare the surface mission
bool CLuaHelmApp::PrepareSurface()
{
    MOOSTrace("PrepareSurface()\n");

    bool ret = thisHelm->m_ConfigCurrent->SurfaceMission()->Prepare();
    if (!ret)
    {
        DTrace("LUAHELM_MESSAGE", "Surface mission failed to prepare");
    }
    thisHelm->m_bLuaAPIerror = false;
    return ret;
}

//iterate a tick of the surface mission, return true when done
bool CLuaHelmApp::IterateSurface()
{
    MOOSTrace("IterateSurface()\n");


    if (MustEndMission(thisHelm->m_ConfigCurrent->SurfaceMission(), 
                       &CLuaHelmApp::UnregisterMission,
                       "Safety mission has fired!"))
    {
        return true;
    }

    //IF WE GET HERE, WE ITERATE THE MISSION(s)
    thisHelm->m_PathAction = new CPathAction;

    if (MissionTickFailed(thisHelm->m_ConfigCurrent->SafetyMission(), &CLuaHelmApp::NoOp)) // or allstop
    {
        return true;
    }

    thisHelm->ProcessPIDs(); //uses path action
    delete thisHelm->m_PathAction;
    thisHelm->m_PathAction = NULL;
    
    return false;

}


bool CLuaHelmApp::ProcessPIDs()
{
    MOOSMSG_LIST Notifications;
    MOOSMSG_LIST::iterator q;

    //Get PID Notifications (for logging purposes)
    if(m_ConfigCurrent->LogPIDs())
    {
        for(WhichDOF i = WhichDOF_min; i <= WhichDOF_max; i++)
        {
            if(m_ConfigCurrent->PID(i) != NULL)
            {
                m_ConfigCurrent->PID(i)->GetNotifications(Notifications);
            }
        }
    
        if(m_Comms.IsConnected())
        {
            for(q = Notifications.begin(); q != Notifications.end(); q++)
            {
                m_Comms.Post(*q);
            }
        }
    }
    
    // Run PIDs on all closed loop axes
    for(WhichDOF axis = WhichDOF_min; axis <= WhichDOF_max; axis++)
    {
        // Run over all axes despite the fact that:
        // A) The axis may have been commanded using an open-loop force
        // B) The axis may not have a valid PID (== NULL)
        // We'll pass those checks along to the PathAction.

        if(m_PathAction->RunPID(axis, m_ConfigCurrent->PID(axis)))
        {
            //where we want to end up (informative)
            string sVarName = (string)"DESIRED_" + DOF_Names[axis];

            //how we intend to get there (actual command)
            m_Comms.Notify(sVarName + (string)"_FORCE", m_PathAction->GetDesiredForce(axis));

            //which behavior requested the force (informative)
            m_Comms.Notify(sVarName + (string)"_TASK", m_PathAction->GetTag(axis));
        }
    }

    MOOS_Notifier n;
    n.withString = &CLuaHelmApp::NotifyString;
    n.withDouble = &CLuaHelmApp::NotifyDouble;

    //Make it so!
    if(m_ConfigCurrent->ActuationMap()->Engage(*m_PathAction, 
                                               m_ActuatorMail, 
                                               n))
    {
        return true;
    }

    //getting here is fail
    return false;
}



/*


sandbox:
-compile --> save result somewhere
=init
=prepare
=analyze(the mission file) : MISSION_CHART

-activate - put into "active". 






safety behavior
+10 seconds to mission start (countdown in diveofficer)
-run inner
+monitor RTU
+monitor safety {battery, depth, altitude}
+master timeout


*/
