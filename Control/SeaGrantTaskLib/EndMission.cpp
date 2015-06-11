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
// EndMission.cpp: implementation of the CEndMission class.
//
//////////////////////////////////////////////////////////////////////

#include "EndMission.h"
//#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEndMission::CEndMission()
{
    
    m_sClassName = "End Mission";
/*
    m_dfEndRudder = 0;
    m_dfEndElevator = 0;
    m_dfEndThrust = 0;
    m_nPriority = 0;
*/

}

CEndMission::~CEndMission()
{

}
/*
//returns false if we haven't received data in a while..bad news!
bool CEndMission::RegularMailDelivery(double dfTimeNow)
{
    //always return true (never stop!)
    return true;
}
*/

bool CEndMission::Run(CPathAction &DesiredAction)
{


    for(int dof=0; dof<6; dof++)
        DesiredAction.SetOpenLoop((WhichDOF)dof, 0.0, m_nPriority, m_sName.c_str());

/*
    CMOOSMsg Override(MOOS_NOTIFY, "MOOS_MANUAL_OVERIDE", "TRUE");
    m_Notifications.push_front(Override);
*/
    return true;
}

/*
bool CEndMission::OnNewMail(MOOSMSG_LIST &NewMail)
{

    //always call base class version
    CMOOSBehaviour::OnNewMail(NewMail);


    return true;
}

bool CEndMission::GetRegistrations(STRING_LIST &List)
{

    //always call base class version
    CMOOSBehaviour::GetRegistrations(List);


    return true;
}

bool CEndMission::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);


    if(!CMOOSBehaviour::SetParam(sParam,sVal))
    {
        //this is for us...
        if(sParam=="ENDELEVATOR")
        {
            m_dfEndElevator=MOOSDeg2Rad(atof(sVal.c_str()));
        }
        else if(sParam=="ENDRUDDER")
        {
            m_dfEndRudder =MOOSDeg2Rad(atof(sVal.c_str()));
        }
        else if(sParam=="ENDTHRUST")
        {
            m_dfEndThrust =atof(sVal.c_str());
        }
        else
        {
            //hmmm - it wasn't for us at all: base class didn't understand either
            MOOSTrace("Param \"%s\" not understood!\n",sParam.c_str());
            return false;
        }
    }


    return true;
}
*/
