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
// TrackLineTask.h: interface for the CTrackLineTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRACKLINETASK_H__BF256E8F_9FAD_4B4B_84C5_49658D420E9F__INCLUDED_)
#define AFX_TRACKLINETASK_H__BF256E8F_9FAD_4B4B_84C5_49658D420E9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GoToWayPoint.h"

/**
 * FIXME: needs class description
 */
class CTrackLineTask : public CGoToWayPoint  
{
public:

    enum Mode {TRANSIT, APPROACH, NOTSET};
    enum Method {SLIDING_WAYPOINT, SWAY_TO_TRACK};
		
	CTrackLineTask();
	virtual ~CTrackLineTask();

    virtual bool SetParam(string sParam, string sVal);
	virtual bool GetRegistrations(STRING_LIST &List);
	virtual bool Run(CPathAction &DesiredAction);


protected:
    virtual bool Initialise();	

    bool SetMode(CTrackLineTask::Mode eMode);
    bool SetMethod(string sMethod);
    virtual void PublishWaypointInfo();

	virtual bool CalculateLocalGoal();
    virtual double GetWaypointDistance();
	double GetTracklineError();

    double m_dfLead ;
    double m_dfX1;
    double m_dfY1; 
    double m_dfX2; 
    double m_dfY2; 

    bool m_bOriginSet;
    Mode m_eMode;
	Method m_eMethod;
};

#endif // !defined(AFX_TRACKLINETASK_H__BF256E8F_9FAD_4B4B_84C5_49658D420E9F__INCLUDED_)
