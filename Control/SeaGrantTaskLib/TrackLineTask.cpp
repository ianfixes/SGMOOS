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
// TrackLineTask.cpp: implementation of the CTrackLineTask class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif
#include <math.h>
#include "TrackLineTask.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define MINIMUM_LEAD_DISTANCE 1.0
CTrackLineTask::CTrackLineTask() : CGoToWayPoint()
{
    m_sClassName = "TrackLine";
    m_dfLead = MINIMUM_LEAD_DISTANCE ;
    m_dfX1 = 0;
    m_dfY1 = 0;
    m_dfX2 = 0;
    m_dfY2 = 0;

    m_eMode = NOTSET;
    m_eMethod = SLIDING_WAYPOINT;

    m_bOriginSet = false;

}

CTrackLineTask::~CTrackLineTask()
{
}


bool CTrackLineTask::Run(CPathAction &DesiredAction)
{

    if(!m_bInitialised) return Initialise();

    if(m_eMethod == SLIDING_WAYPOINT)
    {
        if(!CalculateLocalGoal()) return false;

        m_Notifications.push_front(CMOOSMsg(MOOS_NOTIFY,"TRACKLINE_X", m_XDOF.GetDesired()));
        m_Notifications.push_front(CMOOSMsg(MOOS_NOTIFY,"TRACKLINE_Y", m_YDOF.GetDesired()));
    }

    CGoToWayPoint::Run(DesiredAction);

    double dfTrackErr = GetTracklineError();
    m_Notifications.push_front(CMOOSMsg(MOOS_NOTIFY,"TRACKLINE_ERROR", dfTrackErr));

    if(m_eMethod == SWAY_TO_TRACK)
    {
        if(fabs(m_YawDOF.GetError()) < 90)
        {
            DesiredAction.SetClosedLoop(SWAY, dfTrackErr, 0.0, m_nPriority, m_sName.c_str());
        }
    }
    return false;
}

bool CTrackLineTask::GetRegistrations(STRING_LIST &List)
{

    //always call base class version
    return CGoToWayPoint::GetRegistrations(List);
}

bool CTrackLineTask::SetParam(string sParam, string sVal)
{

    MOOSToUpper(sParam);
    MOOSToUpper(sVal);



    //this is for us...
    if(MOOSStrCmp(sParam,"P1"))
    {
        string sTmpX = MOOSChomp(sVal,",");
        string sTmpY = MOOSChomp(sVal,",");

        m_dfX1 =   atof(sTmpX.c_str());
        m_dfY1 =   atof(sTmpY.c_str());
	
        m_bOriginSet = true;
	//update the location
//	m_sLocation += MOOSFormat("--P1=%4.2f,%4.2f",m_dfX1,m_dfY1);
    }
    else if(MOOSStrCmp(sParam,"P2"))
    {

        string sTmpX = MOOSChomp(sVal,",");
        string sTmpY = MOOSChomp(sVal,",");

        m_dfX2 =   atof(sTmpX.c_str());
        m_XDOF.SetDesired(m_dfX2);

        m_dfY2 =   atof(sTmpY.c_str());
        m_YDOF.SetDesired(m_dfY2);
        m_bIsXYPointSet = true;
//	m_sLocation += MOOSFormat("--P2=%4.2f,%4.2f",m_dfX2,m_dfY2);

    }
    else if(MOOSStrCmp(sParam,"LEAD"))
    {
        //useful for later...
        m_dfLead = atof(sVal.c_str());
        if(m_dfLead<MINIMUM_LEAD_DISTANCE)
            m_dfLead = MINIMUM_LEAD_DISTANCE;

    }
    else if(MOOSStrCmp(sParam,"TRACKLINE_METHOD"))
    {
        SetMethod(sVal);
    }
    else  if(!CGoToWayPoint::SetParam(sParam,sVal))
    {
        //hmmm - it wasn't for us at all: base class didn't understand either
        MOOSTrace("Param \"%s\" not understood in Task %s!\n",sParam.c_str(), m_sName.c_str());
        return false;
    }

    return true;

}


double CTrackLineTask::GetWaypointDistance()
{
    return sqrt(pow(m_dfX2-m_XDOF.GetCurrent(),2)+pow(m_dfY2-m_YDOF.GetCurrent(),2));
}


bool CTrackLineTask::CalculateLocalGoal()
{
    //double dfx1 = m_dfX1;
    double dfx2 = m_dfX2;
    //double dfy1 = m_dfY1;
    double dfy2 = m_dfY2;
    double dfx12 = m_dfX2-m_dfX1;
    double dfy12 = m_dfY2-m_dfY1;
    //double dfS12 = sqrt((dfx12*dfx12+dfy12*dfy12));

    //position of vehicle
    double dfxv = m_XDOF.GetCurrent();
    double dfyv = m_YDOF.GetCurrent();


    //angle of vehicle to goal
    double dfTheta = atan2(dfy2-dfyv,dfx2-dfxv);

    //angle of trackline
    double dfLineTheta = atan2(dfy12,dfx12);    

    double dfRangeToGoal = sqrt(pow(dfy2-dfyv,2)+pow(dfx2-dfxv,2));

    //coordinate for secant line?  alls i know is that it looks like
    // drawing a line between xv,yv and xe,ye will cross the trackline
    // where range to goal is the same as the vehicle's range to goal -- ijk5
    double dfxe = dfRangeToGoal*cos(dfTheta-dfLineTheta);

    double dfTheHenrikRatio  = 0.5;


    //point on trackline that is closest to current position (ie, normal)
    // this point is dfxe away from the goal
    double dfxp = dfx2-dfxe*cos(dfLineTheta);
    double dfyp = dfy2-dfxe*sin(dfLineTheta);


    double dfXGoal, dfYGoal;
    if((dfTheHenrikRatio*dfxe) > m_dfLead)
    {   
        //PMN MODE n(v clever)

        SetMode(CTrackLineTask::TRANSIT);

        // go straight back to the trackline, dfLead ahead of normal
        dfXGoal = dfxp+m_dfLead*cos(dfLineTheta);
        dfYGoal = dfyp+m_dfLead*sin(dfLineTheta);

    }
    else
    {
       //HS Method (moderately clever ;-) )
        SetMode(CTrackLineTask::APPROACH);

        // go towards the point of the trackline that is theHenrikRatio
        // between the normal point and the end point
        dfXGoal = (1-dfTheHenrikRatio)*dfxp+dfTheHenrikRatio*dfx2;
        dfYGoal = (1-dfTheHenrikRatio)*dfyp+dfTheHenrikRatio*dfy2;
    }
        

    m_XDOF.SetDesired(dfXGoal);
    m_YDOF.SetDesired(dfYGoal);


    return true;

}

// http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html
double CTrackLineTask::GetTracklineError()
{
    double dfX0 = m_XDOF.GetCurrent();
    double dfY0 = m_YDOF.GetCurrent();
    double dfx12 = m_dfX2-m_dfX1;
    double dfy12 = m_dfY2-m_dfY1;
    double dfS12 = sqrt((dfx12*dfx12+dfy12*dfy12));

    return ((m_dfX2-m_dfX1)*(m_dfY1-dfY0) - (m_dfX1-dfX0)*(m_dfY2-m_dfY1))/dfS12;

}

bool CTrackLineTask::SetMode(CTrackLineTask::Mode eMode)
{
    if(m_eMode!=eMode)
    {
        //state change
        switch(eMode)
        {
        case APPROACH: MOOSTrace("switching to APPROACH mode\n"); break;
        case TRANSIT: MOOSTrace("switching to TRANSIT mode\n"); break;
        default:
            break;
        }
    }

    m_eMode = eMode;

    return true;
}

bool CTrackLineTask::SetMethod(string sMethod)
{
    MOOSToUpper(sMethod);
    if(sMethod=="SLIDING_WAYPOINT")
    {
        m_eMethod = SLIDING_WAYPOINT;
    }
    else if(sMethod=="SWAY_TO_TRACK")
    {
        m_eMethod = SWAY_TO_TRACK;
    }
    else
    {
        MOOSTrace("Unrecognized Trackline Method (%s), Using Default\n");
        return false;
    }
    return true;	
}

void CTrackLineTask::PublishWaypointInfo()
{
    //Tell the world what you want.
    CMOOSMsg XGoal(MOOS_NOTIFY,"WAYPOINT_X", m_dfX2);
    m_Notifications.push_front(XGoal);

    CMOOSMsg YGoal(MOOS_NOTIFY,"WAYPOINT_Y", m_dfY2);
    m_Notifications.push_front(YGoal);
    
    CMOOSMsg Distance(MOOS_NOTIFY,"WAYPOINT_DISTANCE", GetWaypointDistance());
    m_Notifications.push_front(Distance);
}

bool CTrackLineTask::Initialise()
{
    m_bHeadingAcquired = false;

    m_bInitialised = m_bIsXYPointSet && m_bOriginSet;
    return m_bInitialised;
}

