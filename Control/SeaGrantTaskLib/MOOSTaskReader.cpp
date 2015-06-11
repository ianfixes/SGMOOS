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
//////////////////////////    END_GPL    //////////////////////////////////
// MOOSTaskReader.cpp: implementation of the CMOOSTaskReader class.
//
//////////////////////////////////////////////////////////////////////


#include <MOOSLIB/MOOSLib.h>
#include "MOOSTaskLib.h"

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSTaskReader::CMOOSTaskReader()
{

}

CMOOSTaskReader::~CMOOSTaskReader()
{

}

bool CMOOSTaskReader::Run(const char *sFile, CProcessConfigReader* pMissionFileReader, TASK_LIST &Tasks)
{


    if(!SetFile(sFile))
    {
        MOOSTrace("Cannot open file (%s) containing task specification!!!\n", sFile);
        return false;
    }

    Reset();
    CSGMOOSBehaviour * pNewTask = NULL;

    if(!GetFile()->is_open())
    {
        MOOSTrace("CMOOSTaskReader::failed to open task file %s\n",sFile);
        return false;
    }

    while(!GetFile()->eof())
    {
        string sLine = GetNextValidLine();   

        string sTok,sVal;
        if(GetTokenValPair(sLine,sTok,sVal))
        {
            if(MOOSStrCmp(sTok,"TASK"))
            {
                pNewTask = MakeNewTask(sVal);    

                if(pNewTask == NULL)
                {    
                    MOOSTrace("New task is null\n");
                    return false;
                }

                //maybe it needs the mission file to get parameters etc
                pNewTask->SetMissionFileReader(pMissionFileReader);

                if(StuffTask(pNewTask))
                {
                    Tasks.push_front(pNewTask);
                }
                else
                {
                    MOOSTrace("task config failed\n");
                    delete pNewTask;
                    return false;
                }
            }
            else if(MOOSStrCmp(sTok,"NAME"))
            {
                m_sName = sVal;
            }
        }
    }


    MOOSTrace("TaskReader makes %d tasks\n",Tasks.size());

    return true;
}



bool CMOOSTaskReader::StuffTask(CSGMOOSBehaviour *pTask)
{
    
    string sBracket = GetNextValidLine();
    
    if(sBracket!="{")
    {
        MOOSTrace("CMOOSTaskReader::StuffTask ->no opening bracket\n");
        delete pTask;
        return false;
    }

    string sLine;
    while((sLine=GetNextValidLine())!="}")
    {
        string sTok,sVal;
        
        if(GetTokenValPair(sLine, sTok, sVal))
        {
            if(!pTask->SetParam(sTok,sVal))
            {
                MOOSTrace("Ignoring unknown param: %s\n",sLine.c_str());
                //return false;               
            }
        }
        else
        {
            MOOSTrace("failed parse...line = %s\n",sLine.c_str());
            return false;
        }
    }


    return true;
}

CSGMOOSBehaviour * CMOOSTaskReader::MakeNewTask(string sTaskType)
{
    //add new task types  here..
    CSGMOOSBehaviour * pNewTask = NULL;

    MOOSToUpper(sTaskType);


    if(sTaskType == "GOTOWAYPOINT")
    {
        pNewTask = new CGoToWayPoint;
    }
    else if(sTaskType == "TRACKLINE")
    {
        pNewTask = new CTrackLineTask;
    }
    else if(sTaskType == "CONSTANTHEADING")
    {
        pNewTask = new CConstantHeadingTask;
    }
    else if(sTaskType == "MAINTAINHEADING")
    {
        pNewTask = new CMaintainHeading;
    }
    else if(sTaskType == "MAINTAINDEPTH")
    {
        pNewTask = new CMaintainDepth;
    }
    else if(sTaskType == "MAINTAINALTITUDE")
    {
        pNewTask = new CMaintainAltitude;
    }
    else if(sTaskType == "GOTODEPTH")
    {
        pNewTask = new CGoToDepth;
    }
    else if(sTaskType == "GOTOALTITUDE")
    {
        pNewTask = new CGoToAltitude;
    }
    else if(sTaskType == "MAINTAINPOSITION")
    {
        pNewTask = new CMaintainPosition;
    }
    else if(sTaskType == "ENDMISSION")
    {
        pNewTask = new CEndMission;
    }
    else if(sTaskType == "TIMER")
    {
        pNewTask = new CTimerTask;
    }
    else if(sTaskType == "VERTICALPROFILE")
    {
        pNewTask = new CVerticalProfile;
    }
    else if(sTaskType == "MANUALCONTROL")
    {
        pNewTask = new CManualControl;
    }
    else if(sTaskType == "FUNCTIONGENERATOR")
    {
        pNewTask = new CFunctionGenerator;
    }
    else if(sTaskType == "SUPERVISORYXY")
    {
        pNewTask = new CSupervisoryXY;
    }
    else if(sTaskType == "XYPATTERN")
    {
        pNewTask = new CXYPatternTask;
    }
    else if(sTaskType == "HOLDINGPATTERN")
    {
        pNewTask = new CHoldingPattern;
    }
    else if(sTaskType == "SURVEY")
    {
        pNewTask = new CSurveyTask;
    }
    else if(sTaskType == "SAFETYLIMITS")
    {
        pNewTask = new CSafetyLimits;
    }
    else if(sTaskType == "CAMERATASK")
    {
        pNewTask = new CCameraTask;
    }
    else if(sTaskType == "SINGLEACTION")
    {
        pNewTask = new CSingleAction;
    }
	else if(sTaskType == "FACEVERTICALPIPE")
    {
        pNewTask = new CFaceVerticalPipe;
    }
	else if(sTaskType == "MAINTAINPIPEDISTANCE")
    {
        pNewTask = new CMaintainPipeDistance;
    }
	else if(sTaskType == "PIPERELHEADING")
    {
        pNewTask = new CMaintainPipeRelHeading;
    }
    else if(sTaskType == "RTUMONITOR")
    {
        pNewTask = new CRTUMonitor;
    }
    else
    {
        MOOSTrace("Task Type \"%s\" is unknown\n",sTaskType.c_str());
    }
        
    return pNewTask;
}
