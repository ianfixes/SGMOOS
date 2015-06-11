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
// LuaHelmApp.h - interface for the CLuaHelm class
//
/////////////////////////////////////


#if !defined(AFX_LUAHELMAPP_H_INCLUDED_)
#define AFX_LUAHELMAPP_H_INCLUDED_

#include <MOOSLIB/MOOSLib.h>
#include <string>
#include <dirent.h>
#include <malloc.h>

#include "StateMachine.h"

#include "HelmConfig.h"
#include "LuaMission.h"

using namespace std;

class CLuaHelmApp : public CMOOSApp
{
public:
    CLuaHelmApp();
    virtual ~CLuaHelmApp();
    

    /**
    *we override the OnNewMail to handle Task's sending of variable
    *with more than one value 
    */

    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool OnCommandMsg(CMOOSMsg CmdMsg);
    bool OnConnectToServer();
    bool OnStartUp();
    bool Iterate();


protected:


    CHelmConfig* m_ConfigCurrent;
    CHelmConfig* m_ConfigNext;

    CLuaMission* m_MissionCurrent;
    CLuaMission* m_MissionNext;

    bool m_bInitialised;
    bool m_bMissionReady;
    bool m_bLuaAPIerror;

    string m_sMissionToLoad;

    string m_sCommand;
    bool m_bCommandHandled;

    CPathAction* m_PathAction;
    MSG_MAP m_ActuatorMail;

    MOOSMSG_LIST m_NewMail;

    CStateMachine m_MachineMain;

    static CLuaHelmApp* thisHelm;
    static void SetThisHelm(CLuaHelmApp* me);

    static STRING_SET CoreRegistrations();
    static STRING_SET SetSubtract(STRING_SET m, STRING_SET s);

    //static functions for state transitions
    // because c++ doesn't allow us to pass pointers to instance methods
    static bool NoOp() { return true; }
    static bool LogIdle();
    static bool LogActive();
    static bool LoadCfg();
    static bool RegisterMission();
    static bool RegisterSurface();
    static bool InitRegister();
    static bool ApplyCfg();
    static bool ExecuteIdle();
    static bool ExecuteRun();
    static bool ExecuteSurf();
    static bool ReRegSurface();
    static bool Unregister(STRING_SET vars);
    static bool UnregisterMission();
    static bool UnregisterSurface();
    static bool PrepareRun();
    static bool IterateRun();
    static bool PrepareSurface();
    static bool IterateSurface();

    static bool MustEndMission(CLuaMission* mission, bool (*action)(void), string message);
    static bool MissionTickFailed(CLuaMission* mission, bool (*action)(void));



    //double-trace: print a message to the console as well as a moosvar
    static void DTrace(string sMoosvar, string sMessage);

    //static functions for both APIs
    static int API_Trace(lua_State* L);
    static int API_Time(lua_State* L);

    //static functions for Lua MOOS API
    static int MAPI_SetClosedLoop(lua_State* L);
    static int MAPI_SetOpenLoop(lua_State* L);
    static int MAPI_MissionMessage(lua_State* L);
    static int MAPI_MissionState(lua_State* L);
    static int MAPI_PostString(lua_State* L);
    static int MAPI_PostDouble(lua_State* L);

    //static functions for Lua sandbox API
    static int SAPI_SetClosedLoop(lua_State* L)   { return SAPI_General(L, "SetClosedLoop"); }
    static int SAPI_SetOpenLoop(lua_State* L)     { return SAPI_General(L, "SetOpenLoop"); }
    static int SAPI_MissionMessage(lua_State* L)  { return SAPI_General(L, "MissionMessage"); }
    static int SAPI_MissionState(lua_State* L)    { return SAPI_General(L, "MissionState"); }
    static int SAPI_PostString(lua_State* L)      { return SAPI_General(L, "PostString"); }
    static int SAPI_PostDouble(lua_State* L)      { return SAPI_General(L, "PostDouble"); }

    //print out what a function received
    static int SAPI_General(lua_State* L, string func);

    static bool CheckLuaArgs(lua_State* L, string func, int numargs);
    static bool CheckDOF(lua_State* L, WhichDOF nDirection);

    //create the master helm process state machine
    void CreateMachineMain();    

    bool ProcessPIDs();

    bool ReloadConfig();
    void SwapConfig();

    void RescanMissions();
    bool LoadMission(string sMission);
    bool LoadMissionFail(string sMessage);
    bool ProcessMissionActivation();

    LUA_API_MAP LuaApiSandbox();
    LUA_API_MAP LuaApiMOOS();

    //static functions for error handling
    static void Lua_OnErr(string source, string message);
    static void Lua_OnErrSandbox(string source, string message);

    //static functions for the actuation api
    static bool NotifyString(string sVar, string sVal) { return thisHelm->m_Comms.Notify(sVar, sVal); }
    static bool NotifyDouble(string sVar, double sVal) { return thisHelm->m_Comms.Notify(sVar, sVal); }


};

#endif // !defined(AFX_LUAHELMAPP_H_INCLUDED_)

