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
// MaintainPosition.cpp: implementation of the CMaintainPosition class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "MaintainPosition.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMaintainPosition::CMaintainPosition()
{
    m_sClassName="Maintain Position";
    m_bIsXYPointSet = false;

    // if distance to goal is > 10m, turn towards it.
    m_dfHeadingControlRadius = 10.0; 
}

CMaintainPosition::~CMaintainPosition()
{

}

bool CMaintainPosition::GetRegistrations(STRING_LIST &List)
{

    if(m_bAllowWaypointChanges)
    {
        List.push_back("MANUAL_MOVE_WAYPOINT_FORWARD");
        List.push_back("MANUAL_MOVE_WAYPOINT_STARBOARD");
    }

    //always call base class version
    CFaceXYPoint::GetRegistrations(List);

    return true;
}

bool CMaintainPosition::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;

    CFaceXYPoint::OnNewMail(NewMail);

    bool bNewXY = false;
    double dfNow = MOOSTime();

    if(m_bAllowWaypointChanges)
    {
        if(PeekMail(NewMail,"MANUAL_MOVE_WAYPOINT_FORWARD",Msg))
        {
            if(!Msg.IsSkewed(dfNow))
            {
                double dfNewSurgePosition=Msg.m_dfVal;
                double dfCurrentYaw = m_YawDOF.GetCurrent();

                m_XDOF.SetDesired(m_XDOF.GetDesired()-dfNewSurgePosition*sin(dfCurrentYaw));
                m_YDOF.SetDesired(m_YDOF.GetDesired()+dfNewSurgePosition*cos(dfCurrentYaw));
                bNewXY=true;
            }
        }
    
        if(PeekMail(NewMail,"MANUAL_MOVE_WAYPOINT_STARBOARD",Msg))
        {
            if(!Msg.IsSkewed(dfNow))
            {
                double dfNewSwayPosition=Msg.m_dfVal;
                double dfCurrentYaw = m_YawDOF.GetCurrent();

                m_XDOF.SetDesired(m_XDOF.GetDesired()+dfNewSwayPosition*cos(dfCurrentYaw));
                m_YDOF.SetDesired(m_YDOF.GetDesired()+dfNewSwayPosition*sin(dfCurrentYaw));
                bNewXY=true;

                bNewXY=true;
            }
        }
    }
    return true;
}

bool CMaintainPosition::Run(CPathAction &DesiredAction)
{

    if(m_XDOF.IsValid() && m_YDOF.IsValid() && m_YawDOF.IsValid())
    {

        double dfXErr = m_XDOF.GetError();
        double dfYErr = m_YDOF.GetError();
        double dfYaw = m_YawDOF.GetCurrent();
        
        // Calculate Surge & Sway
        double dfSurgeError = dfYErr*cos(dfYaw) - dfXErr*sin(dfYaw);
        double dfSwayError  = dfYErr*sin(dfYaw) + dfXErr*cos(dfYaw);

        double dfDesired = 0.0;  //Not sure what to do about this... its only for logging anyway.

        DesiredAction.SetClosedLoop(SURGE, dfSurgeError, dfDesired, m_nPriority, m_sName.c_str());
        DesiredAction.SetClosedLoop(SWAY, dfSwayError, dfDesired, m_nPriority, m_sName.c_str());

        // Only control heading if outside the radius...
        if(GetWaypointDistance() > m_dfHeadingControlRadius)
        {
            CMaintainHeading::Run(DesiredAction);
        }
        PublishWaypointInfo();

    }
    return true;
}

bool CMaintainPosition::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);

    if(!CFaceXYPoint::SetParam(sParam,sVal))
    {
    
        //this is for us...
        if(MOOSStrCmp(sParam,"HeadingControlRadius"))
        {
            m_dfHeadingControlRadius=atof(sVal.c_str());
        }
        else
        {
            return false;
        }
    }
    return true;

}

