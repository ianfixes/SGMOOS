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
// LuaHelmConfig.h - interface for the CLuaHelmConfig class
//
/////////////////////////////////////


#if !defined(AFX_LUAHELMCONFIG_H_INCLUDED_)
#define AFX_LUAHELMCONFIG_H_INCLUDED_

#include <MOOSLIB/MOOSLib.h>
#include <string>
#include <list>

#include "LuaMission.h"
#include "MOOSActuationMap.h"
#include "OdysseyIVActuation.h"
#include "KatrinaBoatActuation.h"
#include "ReefExplorerActuation.h"
#include "../SeaGrantTaskLib/MOOSTaskLib.h"


using namespace std;

class CHelmConfig 
{
public:
    CHelmConfig(string sMissionFile, CProcessConfigReader* myMissionReader);
    ~CHelmConfig();

    bool Reload(LUA_API_MAP api, void (*OnError)(string, string));
    void UpdateLuaAPI(LUA_API_MAP api, void (*OnError)(string, string));

    STRING_SET Registrations();

    void MailPIDs(MOOSMSG_LIST m);

    //inline stuff
    CLuaMission* SurfaceMission()      { return m_LuaSurface; }
    CLuaMission* SafetyMission()       { return m_LuaSafety; }
    CPID* PID(WhichDOF i)              { return m_PID[i]; }
    CMOOSActuationMap* ActuationMap()  { return m_Actuation; }

    bool LogPIDs()                     { return m_bLogPIDs; }
    bool CommandLogging()              { return m_bCommandLogging; }
    string LoggingCmdVar()             { return m_sLoggingCmdVar; }
    string CmdLogStart()               { return m_sCmd_LogStart; }
    string CmdLogStop()                { return m_sCmd_LogStop; }
    string CmdLogIdle()                { return m_sCmd_LogIdle; }
    string LuaLibDir()                 { return m_sLuaLibDir; }
    string LuaMissionDir()             { return m_sLuaMissionDir; }

    string LuaMissionFilePath(string sName)
    {
        return m_sLuaMissionDir + "/" + sName + ".lua";
    }

protected:


    CPID* m_PID[6];

    CMOOSActuationMap* m_Actuation;
    
    CLuaMission* m_LuaSurface;
    CLuaMission* m_LuaSafety;
    
    CProcessConfigReader* m_MissionReader;
    string m_sMissionFile;

    bool m_bLogPIDs;
    bool m_bCommandLogging;
    
    string m_sLoggingCmdVar;
    string m_sCmd_LogStart;
    string m_sCmd_LogStop;
    string m_sCmd_LogIdle;
    
    string m_sLuaLibDir;
    string m_sLuaMissionDir;

    void Cleanup();
    void Nullify();

    CLuaMission* LoadMission(string sID, 
                             LUA_API_MAP api, 
                             void (*OnError)(string, string),
                             string sMissionParam);

    bool SetupPIDs();
    bool LoadActuation();

};

#endif // !defined(AFX_LUAHELMCONFIG_H_INCLUDED_)

