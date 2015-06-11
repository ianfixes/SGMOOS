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
// EndMission.h: interface for the CEndMission class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ENDMISSION_H__FF839AC1_728B_49CF_A4CB_C658EEA9FE72__INCLUDED_)
#define AFX_ENDMISSION_H__FF839AC1_728B_49CF_A4CB_C658EEA9FE72__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MOOSBehaviour.h"

/**
 * FIXME: needs class description
 */
class CEndMission : public CSGMOOSBehaviour  
{
public:
	CEndMission();
	virtual ~CEndMission();

//	virtual bool SetParam(string sParam, string sVal);
//	        bool GetRegistrations(STRING_LIST & List);
//	        bool OnNewMail(MOOSMSG_LIST & NewMail);
	        bool Run(CPathAction &DesiredAction);
//    virtual bool RegularMailDelivery(double dfTimeNow);



protected:

/*
    double m_dfEndRudder;
    double m_dfEndElevator;
    double m_dfEndThrust;
*/

};

#endif // !defined(AFX_ENDMISSION_H__FF839AC1_728B_49CF_A4CB_C658EEA9FE72__INCLUDED_)
