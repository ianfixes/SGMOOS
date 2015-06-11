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

#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif



// SGHelm.cpp: implementation of the CSGHelm class.
//
//////////////////////////////////////////////////////////////////////
#include <MOOSLIB/MOOSLib.h>
//#include <MOOSGenLib/MOOSGenLib.h>
#include <MOOSTaskLib.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include "ReefExplorerActuation.h"
#include "OdysseyIVActuation.h"
#include "KatrinaBoatActuation.h"

using namespace std;
#include "SGHelm.h"

#define MAX_TASKS 200

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSGHelm::CSGHelm()
{
    m_bInitialised  =   false;

    //some sensible defaults (missionfile can overwrite this)
    SetAppFreq(5);
    SetCommsFreq(15);

    m_bManualOverRide = true;
    m_dfLastActionTime = -1000.0;
    m_dfActionTimeout = 3.0;

    m_bCommandMySQLLogging = false;
    m_bLogPIDs = false;
}

CSGHelm::~CSGHelm()
{
    delete m_Actuation;

    for(int pid=0; pid<6; pid++)
        delete m_PID[pid];
}


bool CSGHelm::OnNewMail(MOOSMSG_LIST &NewMail)
{
    string sMessage;
    CMOOSMsg Msg;

    UpdateMOOSVariables(NewMail);

    if(m_Comms.PeekMail(NewMail,"MOOS_MANUAL_OVERIDE",Msg))
    {
        if(Msg.IsSkewed(MOOSTime()))
        {
            sMessage = "MOOS_MANUAL_OVERIDE Skewed";
        }
        else 
        {
            if(MOOSStrCmp(Msg.m_sVal,"TRUE"))
            {
                
                sMessage = "Manual Overide is on";
                m_bManualOverRide = true;
                m_sCurrentMission = "Manual Override";

                if(m_bCommandMySQLLogging)
                    m_Comms.Notify("MYSQL_CMD", "Stop");

            }
            else if(MOOSStrCmp(Msg.m_sVal,"FALSE"))
            {
                if(m_Missions.find(m_sNextMission) == m_Missions.end())
                {
                    sMessage = MOOSFormat("No mission named %s loaded\n");
                }
                else
                {
                    sMessage = "Manual Overide is off";

                    m_sCurrentMission = m_sNextMission;
                    m_bManualOverRide = false;

                    if(m_bCommandMySQLLogging)
                        m_Comms.Notify("MYSQL_CMD", MOOSFormat("Start, %s", m_sCurrentMission.c_str()));
                }
            }
            MOOSDebugWrite(sMessage);
        } 
    }
    else if(m_Comms.PeekMail(NewMail,"RESTART_HELM",Msg))
    {
        if(Msg.IsSkewed(MOOSTime()))
        {
            sMessage = "RESTART_HELM Skewed";
        }
        else 
        {
            if(MOOSStrCmp(Msg.m_sVal,"TRUE"))
            {
                RestartHelm();
            }
        }       
    }
    else
    {
        if(!m_bManualOverRide)
            m_Missions[m_sCurrentMission].OnNewMail(NewMail);
    }

    if(!sMessage.empty())
    {
        MOOSDebugWrite(sMessage);
        MOOSTrace(sMessage + "\n");
    }

    if(m_Comms.PeekMail(NewMail,"NEXT_MISSION",Msg))
    {
        double dfNow = MOOSTime();
        if(!Msg.IsSkewed(dfNow))
        {
            if(m_Missions.find(Msg.m_sVal) == m_Missions.end())
            {
                sMessage = MOOSFormat("Mission %s is not found", Msg.m_sVal.c_str());
            }
            else
            {
                m_sNextMission = Msg.m_sVal;
                sMessage = MOOSFormat("Mission %s is on deck", Msg.m_sVal.c_str());

                
                CMOOSMsg MissionName(MOOS_NOTIFY,"MISSION_FILE", m_Missions[m_sNextMission].GetFileName());
                m_Comms.Post(MissionName);
                
                CMOOSMsg MissionChart(MOOS_NOTIFY,"MISSION_CHART",m_Missions[m_sNextMission].GetChart());
                m_Comms.Post(MissionChart);

            }
            MOOSDebugWrite(sMessage);
            MOOSTrace(sMessage + "\n");
        }
    }

    for(int i=0; i<6; i++)
    {
        if(m_PID[i] != NULL)
            m_PID[i]->OnNewMail(NewMail);
    }

    return true;
}


bool CSGHelm::OnConnectToServer()
{

    double dfInterval = 0.1;

    m_Comms.Register("RESTART_HELM", dfInterval);
    m_Comms.Register("MOOS_MANUAL_OVERIDE", dfInterval);
    m_Comms.Register("NEXT_MISSION", dfInterval);

    return true;
}


bool CSGHelm::Iterate()
{
    if(m_bInitialised==false)
    {
        MOOSTrace("CSGHelm::Iterate() FAIL : Helm not initialised\n");
        return false;
    }
    
    CPathAction DesiredAction;
    bool bActionRequest = false;

    if(IsManualOveride())
    {
	DesiredAction.AllStop("ManualOverride",1);
        bActionRequest=true;
    }
    else
    {
        //run active set of behaviors to get the desired action
        bActionRequest |= m_Missions[m_sCurrentMission].Run(DesiredAction);

        //posts non-actuation moos vars to the db
        OnPostIterate();

    }

    if(bActionRequest) // Did we ask for somthing new this iteration
    {
        m_dfLastActionTime = MOOSTime();
    }
    else
    {

        if(false)
        {
            // STOP if we haven't gotten a command for a while.
            if(DesiredAction.AllStop("TimedOut"))
                m_dfLastActionTime = MOOSTime();
        }
        else
        {
            // Allow previous command to stay valid for a while.
            return true;
        }
    }

    // Run PIDs on all closed loop axes
    double dfNow = MOOSTime();
    for(int axis=0; axis<6; axis++)
    {
        // Run over all axes despite the fact that:
        // A) The axis may have been commanded using an open-loop force
        // B) The axis may not have a valid PID (== NULL)
        // We'll pass those checks along to the PathAction.

        if(DesiredAction.RunPID((WhichDOF)axis, m_PID[axis]))
        {
            //where we want to end up (informative)
            string sVarName = (string)"DESIRED_" + DOF_Names[axis];

            //how we intend to get there (actual command)
            SetMOOSVar(sVarName+(string)"_FORCE", DesiredAction.GetDesiredForce((WhichDOF)axis), dfNow);

            //which behavior requested the force (informative)
            SetMOOSVar(sVarName+(string)"_TASK", DesiredAction.GetTag((WhichDOF)axis), dfNow);
        }
    }


    //Make it so!
    if(m_Actuation->Engage(DesiredAction, m_MOOSVars))
        return PublishFreshMOOSVariables();

    else
        return false;
    
}

bool CSGHelm::Initialise()
{

    m_bInitialised = false;

    //we need to have talked to the DB to get our skew
    //time before this as tasks need a start time
    //this is unusual behaviour for a MOOS process but
    //does mean things are safe...
    int nCount = 0;
    while(!m_Comms.IsConnected())
    {
        
        MOOSPause(1000);
        if(nCount++>30)
        {
            MOOSTrace("Cannot initialise helm without connecting to Server\n");
            MOOSTrace("Waited 30 seconds..quiting\n");
            return false;
        }
    }
   
    STRING_LIST Lines, Reg;

    //set up PID gains and limits
    if(!SetupPIDs(Reg)) 
    {
        MOOSTrace("Failed to SetupPIDs()\n");
        return false;
    }


    // Read in Mission Files
    int nUnsuccessfulReads = 0;
    if(!m_MissionReader.GetConfiguration(m_sAppName,Lines))
    {
        MOOSTrace("MissionReader.GetConfiguraton() failed\n");
        return false;
    }

    string sTok, sVal;
    STRING_LIST::iterator Line;
    for(Line=Lines.begin(); Line!=Lines.end(); Line++)
    {
        m_MissionReader.GetTokenValPair(*Line, sTok, sVal);
        if(MOOSStrCmp(sTok, "TASKFILE"))
        {
            CMOOSMission NewMission;
            if(!NewMission.ReadHoofFile(sVal, &m_MissionReader))
            {
                nUnsuccessfulReads++;
            }
            else
            {
                string sName=NewMission.GetName();
                m_Missions[sName]=NewMission;
                m_Missions[sName].GetRegistrations(Reg);
                MOOSTrace("Successfully Read Mission %s\n", sName.c_str());
            }
        }
    }
    

    //Register for our variables
    for(STRING_LIST::iterator r=Reg.begin(); r!=Reg.end(); r++)
        m_Comms.Register(*r, 0.1);

    //Alert the DB which missions we successfully loaded.
    string sReportMissions;
    MISSION_MAP::iterator mission;

    for(mission=m_Missions.begin(); mission!=m_Missions.end(); mission++)
        sReportMissions.append(mission->second.GetName() + ",");

    if(!sReportMissions.empty())
        sReportMissions.erase(sReportMissions.end()-1);

    m_Comms.Notify(string("MISSIONS_LOADED"), sReportMissions, MOOSTime());

    //if we have got to here we are OK
    m_bInitialised = true;

    return m_bInitialised;
}

bool CSGHelm::SetupPIDs(STRING_LIST& Reg)
{
    CPID* NewPID;

    for(int i=0; i<6; i++)
    {
        //TODO: handle allocation exception.
        MOOSTrace("Reading PID for %s\n", DOF_Names[i].c_str());
        NewPID = new CPID();
        NewPID->SetName(DOF_Names[i]);

        if(NewPID->ReadConfig(m_MissionReader))
        {
            // KEEP IT
            m_PID[i]=NewPID;
            NewPID = NULL;
            m_PID[i]->GetRegistrations(Reg);
        }
        else
        {
            // DELETE IT
            m_PID[i]=NULL;
            delete NewPID;
        }
    }

    return true;
}


#define UPDATE_INTERVAL 0.2

bool CSGHelm::OnPreIterate()
{
    TASK_LIST::iterator p;

    STRING_LIST NewResources;
    STRING_LIST::iterator q;
/* MISSION
   for(p = m_Tasks.begin();p!=m_Tasks.end();p++)
   {
   CMOOSBehaviour* pBehaviour = *p;

   if(pBehaviour->HasNewRegistration())
   {
   NewResources.clear();

   pBehaviour->GetRegistrations(NewResources);

   for(q = NewResources.begin();q!=NewResources.end();q++)
   {
   string sVar=*q;
   AddMOOSVariable(sVar, sVar, "", UPDATE_INTERVAL);
   MOOSTrace("Registering for: %s\n", sVar.c_str()); 
   }

   RegisterMOOSVariables();
   }
   }
*/
    return true;
}


bool CSGHelm::OnPostIterate()
{
    MISSION_MAP::iterator p;

    MOOSMSG_LIST Notifications;

    MOOSMSG_LIST::iterator q;

    //Get Task Notifications
    //descend through active behaviors and get posted variables
    m_Missions[m_sCurrentMission].GetNotifications(Notifications);

    //Get PID Notifications (for logging purposes)
    if(m_bLogPIDs)
    {
        for(int i=0; i<6; i++)
        {
            if(m_PID[i] == NULL)
                continue;
    
            m_PID[i]->GetNotifications(Notifications);
        }
    }

    // Post said Notifications
    for(q = Notifications.begin();q!=Notifications.end();q++)
    {
        if(m_Comms.IsConnected())
            m_Comms.Post(*q);
    }

    return true;
}

bool CSGHelm::OnStartUp()
{
    bool bRet = true;

    m_MissionReader.GetConfigurationParam("CommandMySQLLogging",m_bCommandMySQLLogging);
    m_MissionReader.GetConfigurationParam("LogPIDs",m_bLogPIDs);

    if (!LoadActuation())
    {
	bRet = false;
        MOOSTrace("LoadActuation() failed!\n");
    }

    if (!Initialise())
    {
        bRet = false;
        MOOSTrace("Initialise() failed!\n");
    }

    return bRet;

}

void CSGHelm::ClearMissions()
{
    MISSION_MAP::iterator m;
    for(m=m_Missions.begin(); m!=m_Missions.end(); m++)
        m->second.Destroy();
    
    m_Missions.clear();

}

bool CSGHelm::RestartHelm()
{
    
    //tell the system via debug...
    MOOSDebugWrite("Helm Restarting");


    //need to reload mission file
    m_MissionReader.SetFile(m_sMissionFile.c_str());

    ClearMissions();
    m_Missions.clear();

    return Initialise();
}

bool CSGHelm::IsManualOveride()
{
    return m_bManualOverRide;
}

bool CSGHelm::PassSafetyCheck()
{
    //bool bFoundEndMission		= false;
    //bool bFoundOverallTimeOut	= false;
    TASK_LIST::iterator p;
/*TODO: JGE: Uncomment when EndMission & OverallTimeOut are implemented.
  for(p = m_Tasks.begin();p!=m_Tasks.end();p++)
  {
  CMOOSBehaviour * pTask = *p;

  CEndMission* pEndMission = dynamic_cast<CEndMission*>(pTask);

  if(pEndMission != NULL)
  {
  bFoundEndMission = true;    
  }
		
  COverallTimeOut* pOverallTimeOut = dynamic_cast<COverallTimeOut*>(pTask);

  if(pOverallTimeOut != NULL)
  {
  bFoundOverallTimeOut = true;
  }

  }

  return (bFoundEndMission && bFoundOverallTimeOut);
*/
    return true;
}

bool CSGHelm::LoadActuation()
{
    m_Actuation = NULL;
    string sActuationType;
    string sVarname;

    // READ THE TYPE FROM FILE (global)
    if(!m_MissionReader.GetValue("VehicleType", sActuationType))
    {
        MOOSTrace("Actuation:  vehicle type must be specified.\nAborting\n");
        return false;
    }
    // ALLOCATE THE ACTUATION MAP
    if(MOOSStrCmp(sActuationType, "KatrinaBoat"))
        m_Actuation = new CKatrinaBoatActuation();
    else if(MOOSStrCmp(sActuationType, "OdysseyIV"))
        m_Actuation = new COdysseyIVActuation();
    else if(MOOSStrCmp(sActuationType, "ReefExplorer"))
        m_Actuation = new CReefExplorerActuation();
    else
    {
        MOOSTrace("I am not familiar with this actuation type: %s\n");
        return false;
    }

    // MAKE SURE ALLOCATION WENT OK
    if(m_Actuation == NULL)
    {
        MOOSTrace("Unable to allocation actuation class\n");
        return false;
    }

    // ALLOW THE ACTUATION SET TO READ ANY CONFIGURATION VARIABLES IT NEEDS
    if(!m_Actuation->ReadConfig(m_MissionReader))
    {
        MOOSTrace("%s: Actuation Set didn't read config block poperly\n", m_sAppName.c_str());
        return false;
    }

    // ADD ACTUATOR PUBLICATIONS TO OUR MOOSVARIABLES
    STRING_LIST sPubs = m_Actuation->GetActuatorList();
    for(STRING_LIST::iterator i = sPubs.begin(); i != sPubs.end(); i++)
    {
        string sActuator = *i;
        sVarname = MOOSFormat("DESIRED_%s",sActuator.c_str());

        AddMOOSVariable(sActuator, (string)"JUNK_" + sVarname, sVarname, 0.1);
    }

    m_Actuation->Subscribe(m_MOOSVars);

    // GET & PUBLISH VALID BODY AXIS FORCES
    bitset<6> bitmask(m_Actuation->GetValidDOFs());
    m_ValidDOFs = bitmask;
    for(int i=0; i<6; i++)
    {
        if(m_ValidDOFs.test(i))
        {
            sVarname = MOOSFormat("DESIRED_%s_FORCE", DOF_Names[i].c_str());
            AddMOOSVariable(sVarname, (string)"JUNK_" + sVarname, sVarname, 0.1);

            sVarname = MOOSFormat("DESIRED_%s_TASK", DOF_Names[i].c_str());
            AddMOOSVariable(sVarname, (string)"JUNK_" + sVarname, sVarname, 0.1);

        }
    }

    RegisterMOOSVariables();
    return true;
}

