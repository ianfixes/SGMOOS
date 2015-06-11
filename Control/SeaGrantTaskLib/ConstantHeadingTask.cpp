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
// ConstantHeading->cpp: implementation of the CConstantHeadingTask class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "ConstantHeadingTask.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CConstantHeadingTask::CConstantHeadingTask() : CSGMOOSBehaviour()
{

    m_sClassName = "Constant Heading Task";
    m_dfYawErrorThreshold = MOOSDeg2Rad(180.0);
    SetUpComponents();
}

CConstantHeadingTask::~CConstantHeadingTask()
{
    m_Components.clear();
}

bool CConstantHeadingTask::SetUpComponents()
{
    m_Components["Heading"] = new CMaintainHeading();
    m_Components["Forward"] = new CGoForward();

    m_Components["Heading"]->NotTopLevel();
    m_Components["Forward"]->NotTopLevel();
    return true;
}

//returns false if we haven't received data in a while..bad news!
bool CConstantHeadingTask::RegularMailDelivery(double dfTimeNow)
{

    bool bHeading = m_Components["Heading"]->RegularMailDelivery(dfTimeNow);
    bool bForward = m_Components["Forward"]->RegularMailDelivery(dfTimeNow);

    if(!bHeading) MOOSTrace("Heading Failed\n");
    if(!bForward) MOOSTrace("Forward Failed\n");
    return bHeading && bForward;
}

void CConstantHeadingTask::SetName(string sName)
{
    m_sName=sName;
    m_Components["Heading"]->SetName(sName);
    m_Components["Forward"]->SetName(sName);

}

void CConstantHeadingTask::SetTime(double dfTimeNow)
{
    CSGMOOSBehaviour::SetTime(dfTimeNow);
    m_Components["Heading"]->SetTime(dfTimeNow);
    m_Components["Forward"]->SetTime(dfTimeNow);
}

bool CConstantHeadingTask::Run(CPathAction &DesiredAction)
{

    if(!m_bInitialised)
    {
        Initialise();
    }

    bool bOk = m_Components["Heading"]->Run(DesiredAction);
    
    if( (fabs(DesiredAction.GetDesiredForce(YAW)) < m_dfYawErrorThreshold)
         && DesiredAction.IsClosedLoop(YAW) )
    {
        bOk &=  m_Components["Forward"]->Run(DesiredAction);
    }

    return bOk;
}

bool CConstantHeadingTask::OnNewMail(MOOSMSG_LIST &NewMail)
{

    m_Components["Heading"]->OnNewMail(NewMail);
    m_Components["Forward"]->OnNewMail(NewMail);

    //always call base class version
    CSGMOOSBehaviour::OnNewMail(NewMail);

    return true;
}

bool CConstantHeadingTask::GetRegistrations(STRING_LIST &List)
{

    m_Components["Heading"]->GetRegistrations(List);
    m_Components["Forward"]->GetRegistrations(List);

    //always call base class version
    CSGMOOSBehaviour::GetRegistrations(List);

    return true;
}


bool CConstantHeadingTask::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);

    if(!CSGMOOSBehaviour::SetParam(sParam,sVal))
        if(!m_Components["Heading"]->SetParam(sParam,sVal))
            if(!m_Components["Forward"]->SetParam(sParam,sVal))
            {
                if(MOOSStrCmp(sParam,"TURNINPLACEANGLE"))
                {
                    m_dfYawErrorThreshold = MOOSDeg2Rad(atof(sVal.c_str()));
                    return true;
                }
                
                return false;
            }

    return true;
}

bool CConstantHeadingTask::GetNotifications(MOOSMSG_LIST & List)
{
    bool bOk = true;

    // Get My Components' Notifications
    bOk &= m_Components["Heading"]->GetNotifications(List);
    bOk &= m_Components["Forward"]->GetNotifications(List);

    // Get Mine Too
    bOk &= CSGMOOSBehaviour::GetNotifications(List);
    return bOk;
}

bool CConstantHeadingTask::Initialise()
{
    m_bInitialised = true;

    return true;
}

bool CConstantHeadingTask::OnStart()
{
    bool bOk = true;

    bOk &= m_Components["Heading"]->Start();
    bOk &= m_Components["Forward"]->Start();

    return bOk;

}
