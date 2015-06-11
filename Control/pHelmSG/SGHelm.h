/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Justin Eskesen & others at MIT Sea Grant.
    contact: jge@mit.edu

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

// SGHelm.h: interface for the CSGHelm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELMAPP_H__F254D4EE_6BA8_423F_8375_4E1A78D5D43F__INCLUDED_)
#define AFX_HELMAPP_H__F254D4EE_6BA8_423F_8375_4E1A78D5D43F__INCLUDED_

#include <map>
#include <bitset>

#include "../SeaGrantTaskLib/MOOSTaskLib.h"
#include "MOOSActuationMap.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_SESSION_TIMEOUT 30
typedef map<string, STRING_LIST> PERMISSIONS_MAP;
typedef map<string, double> SESSION_TIMEOUT_MAP;

/**
 * FIXME: needs class description
 */
class CSGHelm : public CMOOSApp  
{
public:

	CSGHelm();
	virtual ~CSGHelm();
	bool Initialise();
    
protected:

    bool PassSafetyCheck();

    bool SetupPIDs(STRING_LIST& Reg);
    CPID* m_PID[6];
    bool m_bLogPIDs;

    bool IsManualOveride();
    bool RestartHelm();
    virtual bool OnStartUp();
    string MakeWayPointsString();
    CMOOSTaskReader m_TaskReader;
    bool OnPostIterate();
    bool OnPreIterate();
    bool m_bInitialised;

    // true if in manual mode...
    bool m_bManualOverRide;

    bool OnConnectToServer();
    bool Iterate();
    virtual bool OnNewMail(MOOSMSG_LIST & NewMail);
    
//    CMOOSBehaviour::CControllerGains m_Gains;
    CMOOSActuationMap* m_Actuation;
    bool LoadActuation();
    bitset<6> m_ValidDOFs;

    MISSION_MAP m_Missions;
    void ClearMissions();

    string m_sCurrentMission, m_sNextMission;
    bool m_bCommandMySQLLogging;

    double m_dfLastActionTime;
    double m_dfActionTimeout;
};

#endif // !defined(AFX_HELMAPP_H__F254D4EE_6BA8_423F_8375_4E1A78D5D43F__INCLUDED_)
