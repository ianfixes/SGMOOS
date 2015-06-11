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
// C++ Implementation: KatrinaBoatActuation
//
// Description: 
//
//
// Author: Justin Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <math.h>
#include <MOOSGenLib/MOOSGenLib.h>

#include "KatrinaBoatActuation.h"
using namespace std;

CKatrinaBoatActuation::CKatrinaBoatActuation()
 : CMOOSActuationMap()
{
    
    m_nValidDOFs = 33; // surge & yaw (100001)
    
    m_ActuatorList.push_back("STARBOARD_MOTOR");
    m_ActuatorList.push_back("PORT_MOTOR");
}


CKatrinaBoatActuation::~CKatrinaBoatActuation()
{
}

bool CKatrinaBoatActuation::Engage(const CPathAction& DesiredAction, MSG_MAP &msgs, MOOS_Notifier notify)
{
    // Let's get the forces we want out of the map;
    double dfDesiredSurge, dfDesiredYaw, dfPort, dfStarboard, dfNow;
    

    //lets read in all of our DOF's
    // Note: they default to zero if not set.
    dfDesiredSurge = DesiredAction.GetDesiredForce(SURGE);
    dfDesiredYaw = DesiredAction.GetDesiredForce(YAW);

    // Thruster Magnitudes
    dfStarboard = dfPort = dfDesiredSurge;
   
    // Positive Yaw is counter-clockwise; 
    dfStarboard += dfDesiredYaw / 2.0;
    dfPort -= dfDesiredYaw / 2.0;
    
    dfNow = MOOSTime();
    notify.withDouble("STARBOARD_MOTOR", dfStarboard);
    notify.withDouble("PORT_MOTOR", dfPort);

    return true;
}

