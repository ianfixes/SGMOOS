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
// MaintainPosition.cpp: implementation of the CSupervisoryXY class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "SupervisoryXY.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSupervisoryXY::CSupervisoryXY()
{
    m_sClassName="SupervisoryXY";
}

CSupervisoryXY::~CSupervisoryXY()
{

}

bool CSupervisoryXY::GetRegistrations(STRING_LIST &List)
{

    //Absolute Position Changes
    List.push_back("MANUAL_X");
    List.push_back("MANUAL_Y");
    List.push_back("MANUAL_WAYPOINT");

    //Relative position changes
    List.push_back("MANUAL_MOVE_WAYPOINT_FORWARD");
    List.push_back("MANUAL_MOVE_WAYPOINT_STARBOARD");

    //always call base class version
    CMaintainPosition::GetRegistrations(List);

    return true;
}

bool CSupervisoryXY::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;

    CFaceXYPoint::OnNewMail(NewMail);

    bool bNewXY = false;
    double dfNow = MOOSTime();

    if(PeekMail(NewMail,"MANUAL_X",Msg))
    {
        if(!Msg.IsSkewed(dfNow))
            m_XDOF.SetDesired(Msg.m_dfVal);
    }

    if(PeekMail(NewMail,"MANUAL_Y",Msg))
    {
        if(!Msg.IsSkewed(dfNow))
           m_YDOF.SetDesired(Msg.m_dfVal);
    }

    if(PeekMail(NewMail,"MANUAL_WAYPOINT",Msg))
    {
        if(!Msg.IsSkewed(dfNow))
            ParsePositionString(Msg.m_sVal);
    }

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
        }
    }

    return true;
}

bool CSupervisoryXY::SetParam(string sParam, string sVal)
{
    return CSGMOOSBehaviour::SetParam(sParam,sVal);
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

/*
*   On startup, the behavior should initialize its desired position
*   to be the current position.  We want it to stay put.
*/

bool CSupervisoryXY::Initialise()
{

    if(m_XDOF.IsValid())
        m_XDOF.SetDesired(m_XDOF.GetCurrent());

    if(m_YDOF.IsValid())
        m_YDOF.SetDesired(m_YDOF.GetCurrent());

    return m_bInitialised = m_XDOF.IsSet() && m_YDOF.IsSet();
}

