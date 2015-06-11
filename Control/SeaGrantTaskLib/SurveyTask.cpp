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
#include "SurveyTask.h"
#include <math.h>

CSurveyTask::CSurveyTask() : CXYPatternTask()
{
    m_sClassName="Survey";
    m_bInitialised= false;
    m_nTracks = -1;
    m_dfSpacing = 0.0;
}

CSurveyTask::~CSurveyTask()
{
	m_Waypoints.clear();
}

bool CSurveyTask::SetParam(string sParam, string sVal)
{

	MOOSToUpper(sParam);
	
	if(sParam == "TRACKLENGTH")
		m_dfLength = atof(sVal.c_str());
	else if (sParam == "N_TRACKS")
		m_nTracks = atoi(sVal.c_str());
	else if (sParam == "SPACING")
		m_dfSpacing = atof(sVal.c_str());
	else if (sParam == "ANGLE")
		m_dfAngle = MOOSDeg2Rad(atof(sVal.c_str()));
	else if (sParam == "STARTPOINT")
	{
		string sX = MOOSChomp(sVal, ",");
		m_StartPoint.x = atof(sX.c_str());
		m_StartPoint.y = atof(sVal.c_str());
	}
	else 
		return CXYPatternTask::SetParam(sParam, sVal);

	return true;
}

bool CSurveyTask::Initialise()
{

	CWaypoint LegStart = m_StartPoint;
	double dfAngle = m_dfAngle;

	for(int nLeg=0; nLeg<m_nTracks; nLeg++)
	{
		//Every Leg is 2 waypoints.
		m_Waypoints.push_back(LegStart);
		CWaypoint NextPoint(LegStart.x + m_dfLength * sin(dfAngle),
							LegStart.y + m_dfLength * cos(dfAngle));
		m_Waypoints.push_back(NextPoint);
		MOOSTrace("Track %d: (%f,%f) to (%f,%f)\n",nLeg+1, LegStart.x, LegStart.y, NextPoint.x, NextPoint.y);

		//Set up the next leg to go...
		double dfTurn = MOOS_ANGLE_WRAP(m_dfAngle + PI/2);
		LegStart.x = NextPoint.x + m_dfSpacing * sin(dfTurn);
		LegStart.y = NextPoint.y + m_dfSpacing * cos(dfTurn);
		dfAngle = MOOS_ANGLE_WRAP(dfAngle +PI) ;
	}
	return CXYPatternTask::Initialise();
}
