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
// XYPatternTask.h: interface for the CXYPatternTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XYPATTERNTASK_H__57B8999A_6EEB_4B50_BE1C_8B302AE33657__INCLUDED_)
#define AFX_XYPATTERNTASK_H__57B8999A_6EEB_4B50_BE1C_8B302AE33657__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <list>
#include <vector>
#include <MOOSGenLib/MOOSGenLib.h>
#include "TrackLineTask.h"

#define XYPATTERN_X_VAL 0
#define XYPATTERN_Y_VAL 1

#define XYPATTERN_DEFAULT_TOTAL_REPETITION 25

#define XYPATTERN_MAX_TOTAL_REPETITION 50
#define XYPATTERN_MAX_TOTAL_POSITIONS 10


/**
 * FIXME: needs class description
 */
class CWaypoint
{
  public:
	CWaypoint(){}
	CWaypoint(double X, double Y): x(X), y(Y) {}
	double x;
	double y;
};

typedef list<CWaypoint> WAYPOINT_LIST;

/**
 * FIXME: needs class description
 */
class CXYPatternTask : public CTrackLineTask
{
public:

	
	CXYPatternTask();
	virtual ~CXYPatternTask();

	virtual bool SetParam(string sParam, string sVal);


protected:

	virtual bool Initialise();
	void SetPoints();
	virtual void OnWaypointArrival();

	WAYPOINT_LIST m_Waypoints;
	WAYPOINT_LIST::iterator m_CurrentStart;
	WAYPOINT_LIST::iterator m_CurrentFinish;
	int m_nRepCounter;

};

#endif // !defined(AFX_XYPATTERNTASK_H__57B8999A_6EEB_4B50_BE1C_8B302AE33657__INCLUDED_)
