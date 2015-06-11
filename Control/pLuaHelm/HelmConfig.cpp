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

    SGMOOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SGMOOS.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/
//
// LuaHelmConfig.cpp - the CLuaHelmConfig class
//
/////////////////////////////////////

#include "HelmConfig.h"

CHelmConfig::CHelmConfig(string sMissionFile, CProcessConfigReader* myMissionReader)
{
    m_MissionReader = myMissionReader;
    m_sMissionFile = sMissionFile;
    
    //mark memory as blank
    Nullify();
}

CHelmConfig::~CHelmConfig()
{
    Cleanup();
}

//Freeing runtime-allocated variables
void CHelmConfig::Cleanup()
{
    if (m_LuaSurface) delete m_LuaSurface;
    if (m_LuaSafety) delete m_LuaSafety;
    if (m_Actuation) delete m_Actuation;

    for(WhichDOF pid = WhichDOF_min; pid <= WhichDOF_max; pid++)
    {
        if (m_PID[pid]) delete m_PID[pid];
    }

}

//clear any dangling pointers
void CHelmConfig::Nullify()
{
    m_LuaSurface = NULL;
    m_LuaSafety  = NULL;
    m_Actuation  = NULL;
    
    for(WhichDOF pid = WhichDOF_min; pid <= WhichDOF_max; pid++)
    {
        m_PID[pid] = NULL;
    }
}

//restart a Lua environment with a different API
void CHelmConfig::UpdateLuaAPI(LUA_API_MAP api, void (*OnError)(string, string))
{
    m_LuaSurface->SwitchAPI(api, OnError);
    m_LuaSafety->SwitchAPI(api, OnError);
    
    m_LuaSurface->Init();
    m_LuaSafety->Init();
}

//set up a lua environment with a mission in it
//
//sID is your own name for the environment
//api is the lsit of C functions that will be available to the environment
//error function
//sMissionParam is the name of the param in the moos config file with the mission name as its value
CLuaMission* CHelmConfig::LoadMission(string sID, 
                                         LUA_API_MAP api, 
                                         void (*OnError)(string, string),
                                         string sMissionParam)
{
    string sMission;
    string sMissionPath;
    CLuaMission* ret;

    //check moos config for mission file name (missionparam would be safety or surface)
    if (!m_MissionReader->GetConfigurationParam(sMissionParam, sMission))
    {
        MOOSTrace(" x Missing Param: %s\n", sMissionParam.c_str());
        return NULL;
    }
    ret = new CLuaMission(sID, api, OnError, m_sLuaLibDir);

    sMissionPath = LuaMissionFilePath(sMission);

    int loadcode = ret->Load(sMissionPath);
    if (0 != loadcode) //not success
    {
        MOOSTrace(" x Error with lua file at %s: %s, %s\n", 
                  sMissionPath.c_str(), 
                  CLuaMission::LuaErrorDesc(loadcode).c_str(),
                  ret->LastLoadError().c_str());

        //clean up and abort
        delete ret;
        return NULL;
    }

    if (!ret->IsValid())
    {
        MOOSTrace(" x Mission from %s does not contain proper functions\n", sMission.c_str());
        delete ret;
        return NULL;
    }

    if (!ret->Init())
    {
        MOOSTrace(" x Mission from %s failed its own Init()\n", sMission.c_str());
        delete ret;
        return NULL;
    }

    return ret;
}

//reload the configuration of the helm from the moos config file
// spit out helpful error messages about any missing variables
bool CHelmConfig::Reload(LUA_API_MAP api, void (*OnError)(string, string))
{
    bool bRet = true;
    string sTmp;

    //free any memory and clear pointers
    Cleanup();
    Nullify();

    //need to reload mission file
    m_MissionReader->SetFile(m_sMissionFile.c_str());

    if (!m_MissionReader->GetConfigurationParam("LuaLibDir", m_sLuaLibDir))
    {
        MOOSTrace(" x Missing param: LuaLibDir\n");
        bRet = false;
    }

    if (!m_MissionReader->GetConfigurationParam("LuaMissionDir", m_sLuaMissionDir))
    {
        MOOSTrace(" x Missing param: LuaMissionDir\n");
        bRet = false;
    }

    if (NULL == (m_LuaSafety = LoadMission("SAFETY", api, OnError, "LuaMissionSafety")))
    {
        //prints its own error messages
        bRet = false;
    }

    if (NULL == (m_LuaSurface = LoadMission("SURFACE", api, OnError, "LuaMissionSurface")))
    {
        //prints its own error messages
        bRet = false;
    }

    if (!m_MissionReader->GetConfigurationParam("LogPIDs", m_bLogPIDs))
    {
        MOOSTrace(" x Missing param: LogPIDs\n");
        bRet = false;
    }
   
    if (!m_MissionReader->GetConfigurationParam("CommandLogging", m_bCommandLogging))
    {
        MOOSTrace(" x Missing param: CommandLogging\n");
    }

    //need to specify other params if we want to command mysql
    if (m_bCommandLogging)
    {
        if (!m_MissionReader->GetConfigurationParam("LoggingCmdVar", m_sLoggingCmdVar))
        {
            MOOSTrace(" x Missing param: LoggingCmdVar\n");
            bRet = false;
        }

        if (!m_MissionReader->GetConfigurationParam("LogStartCmd", m_sCmd_LogStart))
        {
            MOOSTrace(" x Missing param: LogStartCmd\n");
            bRet = false;
        }

        if (!m_MissionReader->GetConfigurationParam("LogStopCmd", m_sCmd_LogStop))
        {
            MOOSTrace(" x Missing param: LogStopCmd\n");
            bRet = false;
        }
        
        if (!m_MissionReader->GetConfigurationParam("LogIdleCmd", m_sCmd_LogIdle))
        {
            MOOSTrace(" x Missing Param: LogIdleCmd\n");
            bRet = false;
        }
    }
        
    STRING_LIST Lines, Reg;

    //set up PID gains and limits
    if(!SetupPIDs()) 
    {
        MOOSTrace(" x Failed to SetupPIDs()\n");
        bRet = false;
    }

    if (!LoadActuation())
    {
        MOOSTrace(" x LoadActuation() failed!\n");
	bRet = false;
    }

    return bRet;
}


//subscriptions needed by actuation map and PIDs
STRING_SET CHelmConfig::Registrations()
{
    STRING_LIST tmp;
    STRING_SET ret;

    MOOSTrace("      From actuation map\t");
    tmp = m_Actuation->Registrations();
    MOOSTrace("Done\n");

    for (WhichDOF axis = WhichDOF_min; axis <= WhichDOF_max; (int)axis++)
    {
        MOOSTrace("      From PID for %s   \t", DOF_Names[axis].c_str());
        //get PID registrations... a PID_DERIV_VAR for each DOF

        if (m_PID[axis])
        {
            m_PID[axis]->GetRegistrations(tmp);
            MOOSTrace("Done\n");
        }
        else
        {
            MOOSTrace("[NULL]\n");
        }
    }

    //put into set
    for (STRING_LIST::iterator it = tmp.begin(); it != tmp.end(); it++)
    {
        ret.insert(*it);
    }

    return ret;

}

//read PID configs from moos config
bool CHelmConfig::SetupPIDs()
{
    CPID* NewPID;

    for (WhichDOF axis = WhichDOF_min; axis <= WhichDOF_max; axis++)
    {
        MOOSTrace("   Reading PID for %s\n   ", DOF_Names[axis].c_str());
        NewPID = new CPID();
        NewPID->SetName(DOF_Names[axis]);

        if(NewPID->ReadConfig(*m_MissionReader))
        {
            // KEEP IT
            m_PID[axis] = NewPID;
            NewPID = NULL;
        }
        else
        {
            // DELETE IT
            m_PID[axis] = NULL;
            delete NewPID;
        }
        MOOSTrace("\n");
    }

    return true;
}

//read vehicle type from moos config and return actuation map
bool CHelmConfig::LoadActuation()
{
    MOOSTrace("in LoadActuation()\n");
    m_Actuation = NULL;
    string sActuationType;

    // READ THE TYPE FROM FILE (global)
    if(!m_MissionReader->GetValue("VehicleType", sActuationType))
    {
        MOOSTrace(" x Missing param: VehicleType\n");
        return false;
    }

    // ALLOCATE THE ACTUATION MAP
    if(MOOSStrCmp(sActuationType, "KatrinaBoat"))
    {
        m_Actuation = new CKatrinaBoatActuation();
    }
    else if(MOOSStrCmp(sActuationType, "OdysseyIV"))
    {
        m_Actuation = new COdysseyIVActuation();
    }
    else if(MOOSStrCmp(sActuationType, "ReefExplorer"))
    {
        m_Actuation = new CReefExplorerActuation();
    }
    else
    {
        MOOSTrace(" x Unknown actuation type: %s\n");
        return false;
    }

    // MAKE SURE ALLOCATION WENT OK
    if(m_Actuation == NULL)
    {
        MOOSTrace(" x Unable to allocate actuation class\n");
        return false;
    }

    // ALLOW THE ACTUATION SET TO READ ANY CONFIGURATION VARIABLES IT NEEDS
    if(!m_Actuation->ReadConfig(*m_MissionReader))
    {
        MOOSTrace(" x Actuation Set didn't read config block properly\n");
        return false;
    }
    
    
    return true;
}


//deliver MOOS messages to PIDs
void CHelmConfig::MailPIDs(MOOSMSG_LIST NewMail)
{
    for(WhichDOF axis = WhichDOF_min; axis <= WhichDOF_max; axis++)
    {
        if(m_PID[axis] != NULL)
        {
            m_PID[axis]->OnNewMail(NewMail);
        }
    }
}

  //TODO: decide what to do with DESIRED_X_FORCE code
/*//what to do with this?
{



    // GET & PUBLISH VALID BODY AXIS FORCES
    bitset<6> bitmask(m_Actuation->GetValidDOFs());
    for(WhichDOF axis = WhichDOF_min; axis <= WhichDOF_max; axis++)
    {
        if(bitmask.test(axis))
        {

            AddMOOSVariable(MOOSFormat("DESIRED_%s_FORCE", DOF_Names[axis].c_str()), "",
                            MOOSFormat("DESIRED_%s_FORCE", DOF_Names[axis].c_str()), 0.1);

            AddMOOSVariable(MOOSFormat("DESIRED_%s_TASK", DOF_Names[axis].c_str()), "",
                            MOOSFormat("DESIRED_%s_TASK", DOF_Names[axis].c_str()), 0.1);

        }
    }

    RegisterMOOSVariables();

}
*/
