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
// FaceXYPoint.cpp: implementation of the CFaceXYPoint class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "FaceXYPoint.h"
#include "SeaGrantGlobalHelper.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CFaceXYPoint::CFaceXYPoint() : CMaintainHeading()
{
    m_sClassName = "Face XY Point";
    m_bIsXYPointSet = false;
    m_bUseCurrent = false;
}

CFaceXYPoint::~CFaceXYPoint()
{

}

//returns false if we haven't received data in a while..bad news!
bool CFaceXYPoint::RegularMailDelivery(double dfTimeNow)
{
    bool bOk = CMaintainHeading::RegularMailDelivery(dfTimeNow);
    bOk &= !m_XDOF.IsStale(dfTimeNow,GetStartTime());
    bOk &= !m_YDOF.IsStale(dfTimeNow,GetStartTime());
    return bOk;
}

bool CFaceXYPoint::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;

    CMaintainHeading::OnNewMail(NewMail);

    bool bNewXY = false;
    double dfNow = MOOSTime();
    if(PeekMail(NewMail,"NAV_X",Msg))
    { 
        if(!Msg.IsSkewed(dfNow))
        {
            m_XDOF.SetCurrent(Msg.m_dfVal, Msg.m_dfTime);
            bNewXY = true;
        }
        else
        {
            MOOSTrace("%s: Skewed NAV_X (%f)\n", m_sName.c_str(), dfNow-Msg.m_dfTime);
        }

    }

    if(PeekMail(NewMail,"NAV_Y",Msg))
    { 
        if(!Msg.IsSkewed(dfNow))
        {
            m_YDOF.SetCurrent(Msg.m_dfVal, Msg.m_dfTime);
            bNewXY = true;
        }
        else
        {
            MOOSTrace("%s: Skewed NAV_Y (%f)\n", m_sName.c_str(), dfNow-Msg.m_dfTime);        
        }
    }
    
    if(bNewXY)
    {
        //calculate vector heading angle to goal
        double dfDesiredYaw = -atan2(m_XDOF.GetError(),m_YDOF.GetError());
        m_YawDOF.SetDesired(dfDesiredYaw);
    }

    return true;
}

bool CFaceXYPoint::Run(CPathAction &DesiredAction)
{

    if(!m_bInitialised)
        return Initialise();

    if(CMaintainHeading::Run(DesiredAction))
    {
        PublishWaypointInfo();
        return true;
    }
    return false;
}


// Useful for other behaviour which use this one;
double CFaceXYPoint::GetWaypointDistance()
{
    return sqrt(m_XDOF.GetError()*m_XDOF.GetError() +
                m_YDOF.GetError()*m_YDOF.GetError());
}

bool CFaceXYPoint::GetRegistrations(STRING_LIST &List)
{

    List.push_back("NAV_X");
    List.push_back("NAV_Y");

    //always call base class version
    CMaintainHeading::GetRegistrations(List);

    return true;
}

bool CFaceXYPoint::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);

    if(!m_bTopLevel) return true;

    if(CSGMOOSBehaviour::SetParam(sParam,sVal)) return true;

    //this is for us...    
    if(sParam == "LOCATION")
    {
        m_bIsXYPointSet = ParsePositionString(sVal);
        return true;
    }

    return false;
}


bool CFaceXYPoint::Initialise()
{

    if(!m_bIsXYPointSet && m_bUseCurrent)
    {
        if(m_XDOF.IsValid())
            m_XDOF.SetDesired(m_XDOF.GetCurrent());

        if(m_YDOF.IsValid())
            m_YDOF.SetDesired(m_YDOF.GetCurrent());

        m_bIsXYPointSet = m_XDOF.IsSet() && m_YDOF.IsSet();
    }

    m_bInitialised = m_bIsXYPointSet;

    return m_bInitialised;
}

void CFaceXYPoint::PublishWaypointInfo()
{
    //Tell the world what you want.
    CMOOSMsg XGoal(MOOS_NOTIFY,"WAYPOINT_X", m_XDOF.GetDesired());
    m_Notifications.push_front(XGoal);

    CMOOSMsg YGoal(MOOS_NOTIFY,"WAYPOINT_Y", m_YDOF.GetDesired());
    m_Notifications.push_front(YGoal);

    CMOOSMsg Distance(MOOS_NOTIFY,"WAYPOINT_DISTANCE", GetWaypointDistance());
    m_Notifications.push_front(Distance);
}

bool CFaceXYPoint::ParsePositionString(string sPos)
{

    if(MOOSStrCmp(sPos,"USECURRENT"))
    {
        m_bUseCurrent=true;
        return false;
    }
    string sTmpX = MOOSChomp(sPos,",");
    string sTmpY = MOOSChomp(sPos,",");
    
    if(sTmpX.empty()||sTmpY.empty())
    {
        MOOSTrace("error in parsing waypoint location from %s\n",sPos.c_str());
        return false;
    }
    
    double dfX = atof(sTmpX.c_str());
    m_XDOF.SetDesired(dfX);
    
    double dfY =   atof(sTmpY.c_str());
    m_YDOF.SetDesired(dfY);
    
    return true;
}

void CFaceXYPoint::GetChartNodeContents(STRING_LIST& contents) const
{
    contents.push_back(MOOSFormat("Waypoint: (%7.2f,%7.2f)", m_XDOF.GetDesired(), m_YDOF.GetDesired()));
}
