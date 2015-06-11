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
// XYPatternTask.cpp: implementation of the CXYPatternTask class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include "XYPatternTask.h"
#include "math.h"
using namespace std;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXYPatternTask::CXYPatternTask() : CTrackLineTask()
{
    m_sClassName = "XY_Pattern";
}

CXYPatternTask::~CXYPatternTask()
{
    m_Waypoints.clear();
}

bool CXYPatternTask::Initialise()
{

    m_CurrentStart = m_CurrentFinish = m_Waypoints.begin();
    m_CurrentFinish++;
    SetPoints();

    m_bInitialised = true;

    return m_bInitialised;
}


bool CXYPatternTask::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);

    if(!CTrackLineTask::SetParam(sParam,sVal))
    {

        if(sParam=="WAYPOINTS")
        {
            MOOSTrimWhiteSpace(sVal);
            while(!sVal.empty())
            {
                string sWaypoint = MOOSChomp(sVal,":");
                string sXVal = MOOSChomp(sWaypoint,",");
                CWaypoint newWaypoint(atof(sXVal.c_str()),atof(sWaypoint.c_str()));
                m_Waypoints.push_back(newWaypoint);
            }
        }
        else
        {
            //hmmm - it wasn't for us at all: base class didn't understand either
            MOOSTrace("Param \"%s\" not understood by %s!\n",sParam.c_str(),m_sName.c_str());
            return false;
        }
    }

    return true;
}

void CXYPatternTask::SetPoints()
{
    m_bHeadingAcquired = false;

    m_dfX1=m_CurrentStart->x;
    m_dfY1=m_CurrentStart->y;
    m_dfX2=m_CurrentFinish->x;
    m_dfY2=m_CurrentFinish->y;
}

void CXYPatternTask::OnWaypointArrival()
{
    m_CurrentStart = m_CurrentFinish;
    m_CurrentFinish++;

    if(m_CurrentFinish == m_Waypoints.end())
        OnComplete();
    else
        SetPoints();
}
