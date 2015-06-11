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
// ConstantHeading->cpp: implementation of the CGoToWayPoint class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "GoToWayPoint.h"
#include "FaceXYPoint.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CGoToWayPoint::CGoToWayPoint()
{
    //run constructors
    CGoForward();
    CFaceXYPoint();

    m_sClassName = "Go To Waypoint";
    m_dfCaptureRadius = 5.0;  // something sensible?
    m_dfSlowdownRadius = 0.0; // don't do slowdown by default.
    m_bHasSlowedDown=false;
    m_dfYawErrorThreshold = PI;  //let's face it, you're not going to excede this ;)
    m_bUseHeadingThreshold= false;


}

CGoToWayPoint::~CGoToWayPoint()
{
}

bool CGoToWayPoint::Run(CPathAction &DesiredAction)
{

    // Are we there yet?
    double dfDistance = GetWaypointDistance();
    if(dfDistance < m_dfSlowdownRadius && !m_bHasSlowedDown)
    {
        MOOSTrace("Slowing Down...\n");
        CGoForward::SetParam("Speed", MOOSFormat("%f", m_dfSlowdownSpeed));
        m_bHasSlowedDown=true;
    }
		
    if(dfDistance < m_dfCaptureRadius)
    {            
        MOOSTrace("Arrived!\n");
        OnWaypointArrival();
    }

    if(CFaceXYPoint::Run(DesiredAction))
    {
        if(!m_bUseHeadingThreshold || m_bHeadingAcquired)
        {
            //MOOSTrace("Surging: (%7.1f)\n", MOOSRad2Deg(m_YawDOF.GetError()));
            CGoForward::Run(DesiredAction);
        }
        else
        {
            if(fabs(m_YawDOF.GetError()) < m_dfYawErrorThreshold)
            {
                m_bHeadingAcquired = true;
                //MOOSTrace("Transitioning: (%7.1f)\n", MOOSRad2Deg(m_YawDOF.GetError()));
            }
            else
            {
                //MOOSTrace("Turning: (%7.1f)\n", MOOSRad2Deg(m_YawDOF.GetError()));

            }
        }
        return true;
    }

    return false;
}

void CGoToWayPoint::OnWaypointArrival()
{
    OnComplete();
}

bool CGoToWayPoint::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);

    if(!(CFaceXYPoint::SetParam(sParam,sVal) || CGoForward::SetParam(sParam,sVal)))
    {
        if(sParam=="RADIUS")
        {
            m_dfCaptureRadius=atof(sVal.c_str());
        }
        else if(sParam=="SLOWDOWNRADIUS")
        {
            m_dfSlowdownRadius=atof(sVal.c_str());
        }
        else if(sParam=="SLOWDOWNSPEED")
        {
            m_dfSlowdownSpeed=atof(sVal.c_str());
        }
        else if(sParam=="HEADINGTHRESHOLD")
        {
            m_dfYawErrorThreshold=MOOSDeg2Rad(atof(sVal.c_str()));
            m_bUseHeadingThreshold = true;
        }
        else
        {
            return false;
        }
    }

    return true;

}

bool CGoToWayPoint::RegularMailDelivery(double dfNow)
{
    return CFaceXYPoint::RegularMailDelivery(dfNow) && CGoForward::RegularMailDelivery(dfNow);
}

bool CGoToWayPoint::Initialise()
{
    m_bHeadingAcquired = false;
    return CFaceXYPoint::Initialise() && CGoForward::Initialise();
}

void CGoToWayPoint::GetChartNodeContents(STRING_LIST& contents) const
{
    CGoForward::GetChartNodeContents(contents);
    CFaceXYPoint::GetChartNodeContents(contents);
}
