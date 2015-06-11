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
// MaintainAltitude.cpp: implementation of the CMaintainAltitude class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "MaintainAltitude.h"
#include "SeaGrantGlobalHelper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CMaintainAltitude::CMaintainAltitude()
{

    m_sClassName = "Maintain Altitude";
    m_nPriority = 3;

    m_AltitudeDOF.SetDesired(6.0);

    m_bInitialised = false;
    m_bAllowAltitudeChanges = false;

}

CMaintainAltitude::~CMaintainAltitude()
{

}

//returns false if we haven't received data in a while..bad news!
bool CMaintainAltitude::RegularMailDelivery(double dfTimeNow)
{
   
    return !m_AltitudeDOF.IsStale(dfTimeNow,GetStartTime());
    
}


bool CMaintainAltitude::Run(CPathAction &DesiredAction)
{

    if(!m_bInitialised)
    {
        Initialise();
    }

    if(m_AltitudeDOF.IsValid())
    {
        double dfError = m_AltitudeDOF.GetError();
        double dfDesired = m_AltitudeDOF.GetDesired();

        if(dfError>0.0 || !m_bIsLimitingBehavior)
        {
            DesiredAction.SetClosedLoop(HEAVE, dfError, dfDesired, m_nPriority, m_sName.c_str());

       	    //Tell the world what you want.
            CMOOSMsg AltGoal(MOOS_NOTIFY,"DESIRED_ALTITUDE", dfDesired);
            m_Notifications.push_front(AltGoal);
        }

    }
    return true;
}

bool CMaintainAltitude::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;

    if(PeekMail(NewMail,"NAV_ALTITUDE",Msg))
    {
        if(!Msg.IsSkewed(MOOSTime()))
        {
            m_AltitudeDOF.SetCurrent(Msg.m_dfVal, Msg.m_dfTime);  
        }
    }

    if(m_bAllowAltitudeChanges)
    {
        if(PeekMail(NewMail,"MANUAL_ALTITUDE",Msg))
        {
            if(!Msg.IsSkewed(MOOSTime()))
            {
                m_AltitudeDOF.SetDesired(Msg.m_dfVal);
            }
        }
    }

    //always call base class version
    CMaintainDepth::OnNewMail(NewMail);

    return true;
}

bool CMaintainAltitude::GetRegistrations(STRING_LIST &List)
{

    List.push_front("NAV_ALTITUDE");
    if(m_bAllowAltitudeChanges)
        List.push_front("MANUAL_ALTITUDE");

    //always call base class version
    CMaintainDepth::GetRegistrations(List);

    return true;
}


bool CMaintainAltitude::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);


    if(CSGMOOSBehaviour::SetParam(sParam,sVal)) return true;

    //if we get here, this is for us...
    if(sParam=="ALTITUDE" || sParam=="LIMIT")
    {
        double dfDesiredAltitude=atof(sVal.c_str());
        m_AltitudeDOF.SetDesired(dfDesiredAltitude);
        if(sParam=="LIMIT")
        {
                m_bIsLimitingBehavior=true;
        }
        
        return true;
    }
    
    if(MOOSStrCmp(sParam,"ALLOWALTITUDECHANGES"))
    {
        ParseBoolean(m_bAllowAltitudeChanges, sVal);
        return true;
    }

    //hmmm - it wasn't for us at all: base class didn't understand either
    MOOSTrace("Param \"%s\" not understood!\n",sParam.c_str());
    return false;
}

void CMaintainAltitude::GetChartNodeContents(STRING_LIST& contents) const
{
    contents.push_back(MOOSFormat("Altitude: %7.2f", m_AltitudeDOF.GetDesired()));
}
