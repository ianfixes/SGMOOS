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
// MaintainDepth.cpp: implementation of the CMaintainDepth class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "MaintainDepth.h"
#include "SeaGrantGlobalHelper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMaintainDepth::CMaintainDepth()
{
    m_sClassName="Maintain Depth";
    m_bInitialised = false;
    m_DepthDOF.SetDesired(-10.0);
    m_bAllowChanges=false;
    m_bIsLimitingBehavior=false;
}

CMaintainDepth::~CMaintainDepth()
{

}

//returns false if we haven't received data in a while..bad news!
bool CMaintainDepth::RegularMailDelivery(double dfTimeNow)
{
    return !m_DepthDOF.IsStale(dfTimeNow,GetStartTime());
}

bool CMaintainDepth::Run(CPathAction &DesiredAction)
{

    if(!m_bInitialised)
    {
        Initialise();
    }
	
    if(m_DepthDOF.IsValid() && m_DepthDOF.GetDesired() > 0.0)
    {
        // POSITIVE depth is NEGATIVE heave
        double dfError = -1.0 * m_DepthDOF.GetError();
        double dfDesired = m_DepthDOF.GetDesired();

        if((m_bIsLimitingBehavior && dfError>0.0) || !m_bIsLimitingBehavior)
        {
            DesiredAction.SetClosedLoop(HEAVE, dfError, dfDesired, m_nPriority, m_sName.c_str());
        }

        //Tell the world what you want.
        CMOOSMsg DepthGoal(MOOS_NOTIFY,"DESIRED_DEPTH", dfDesired);
        m_Notifications.push_front(DepthGoal);
    }
    return true;
}

bool CMaintainDepth::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;
    double dfNow=MOOSTime();
    if(PeekMail(NewMail,"NAV_DEPTH",Msg))
    {
        if(!Msg.IsSkewed(dfNow))
        {
	    m_DepthDOF.SetCurrent(Msg.m_dfVal,Msg.m_dfTime);
        }
    }

    if(m_bAllowChanges)
        if(PeekMail(NewMail,"MANUAL_DEPTH",Msg))
        {
            if(!Msg.IsSkewed(dfNow))
            {
                m_DepthDOF.SetDesired(Msg.m_dfVal);
            }
        }

    //always call base class version
    CSGMOOSBehaviour::OnNewMail(NewMail);

    return true;
}

bool CMaintainDepth::GetRegistrations(STRING_LIST &List)
{

    List.push_front("NAV_DEPTH");
    
    if(m_bAllowChanges)
        List.push_front("MANUAL_DEPTH");


    //always call base class version
    CSGMOOSBehaviour::GetRegistrations(List);

    return true;
}


bool CMaintainDepth::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);

    if(!CSGMOOSBehaviour::SetParam(sParam,sVal))
    {
        //this is for us...
        if(MOOSStrCmp(sParam,"DEPTH"))
        {
            m_DepthDOF.SetDesired(atof(sVal.c_str()));
        }
        else if(MOOSStrCmp(sParam,"LIMIT"))
        {
            m_DepthDOF.SetDesired(atof(sVal.c_str()));
            m_bIsLimitingBehavior = true;
        }
        else if(MOOSStrCmp(sParam,"ALLOWDEPTHCHANGES"))
        {
            ParseBoolean(m_bAllowChanges, sVal);
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

bool CMaintainDepth::Initialise()
{
    m_bInitialised = true;
    return true;
}

void CMaintainDepth::GetChartNodeContents(STRING_LIST& contents) const
{
    contents.push_back(MOOSFormat("Depth: %7.2f", m_DepthDOF.GetDesired()));
}
