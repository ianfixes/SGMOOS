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
//
// MOOSMotor.h: interface for the CMOOSMotor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOOS_Motor_H)
#define AFX_MOOS_Motor_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<MOOSLIB/MOOSLib.h>
#include "MOOSMotorDriver.h"

using namespace std;


/**
 * FIXME: needs class description
 */
class CMOOSMotor : public CMOOSInstrument
{
public:

    CMOOSMotor();
    virtual ~CMOOSMotor();


protected:
    bool Iterate();
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool OnConnectToServer();
    bool OnStartUp();
    bool GetData();
    bool GetStatus();

    //driver stuff
    bool MakeDriver(string sDriverName);
    CMOOSMotorDriver* m_MotorDriver;
    
    // Pointer to driver's member function, to be called on DESIRED_*
    bool (CMOOSMotorDriver::*m_DesiredAction)(double);

    bool m_bReportPosition;
    bool m_bReportVelocity;
    bool m_bReportCurrent;
    bool m_bReverse;
    double m_dfPositionOffset;

    string m_sPreviousStatus;
    double m_dfStatusPosted;
    double m_dfStatusInterval; // in seconds

    bool m_bDoProfile;
    double m_dfProfileSum;
    int m_nNumProfiles;
};

#endif // !defined(AFX_MOOS_Motor_H)
