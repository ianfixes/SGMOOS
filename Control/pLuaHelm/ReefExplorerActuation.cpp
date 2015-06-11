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
// C++ Implementation: ReefExplorerActuation
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
#include "ReefExplorerActuation.h"
using namespace std;

CReefExplorerActuation::CReefExplorerActuation()
 : CMOOSActuationMap()
{
    m_nValidDOFs = 39 ; // Surge, Sway, Heave, & Yaw (111001)
    m_ActuatorList.push_back("WINCH");
    m_ActuatorList.push_back("STARBOARD_THRUST");
    m_ActuatorList.push_back("PORT_THRUST");
    m_ActuatorList.push_back("BOW_XBODY");
    m_ActuatorList.push_back("STERN_XBODY");

    m_dfWinchCommand4ZeroHeave = 0.0;
    m_dfMaxThrust = 100.;
}


CReefExplorerActuation::~CReefExplorerActuation()
{
}

bool CReefExplorerActuation::ReadConfig(CProcessConfigReader& Config)
{
    if(!Config.GetConfigurationParam("WinchZeroHeaveOffset", m_dfWinchCommand4ZeroHeave))
    {
        MOOSTrace("You'll need to set the winch command to hold still!\n");
        return false;
    }

    return true;

}

bool CReefExplorerActuation::Engage(const CPathAction& DesiredAction, MSG_MAP &msgs, MOOS_Notifier notify)
{
    // Let's get the forces we want out of the map;
    double dfDesiredSurge, dfDesiredHeave, dfDesiredYaw, dfDesiredSway;
    double dfBowCBT, dfSternCBT, dfPortThrust, dfStarboardThrust, dfDesiredWinch;

    double dfLinearScale = 10.0;
    //Surge / Heave controlled by starboard & port thrusters + RTU
    //lets read in all of our DOF's, setting to zero if not set.
    dfDesiredSurge = DesiredAction.GetDesiredForce(SURGE);
    dfDesiredHeave = DesiredAction.GetDesiredForce(HEAVE);
    dfDesiredYaw   = DesiredAction.GetDesiredForce(YAW);
    dfDesiredSway  = DesiredAction.GetDesiredForce(SWAY);

    dfDesiredWinch = dfDesiredHeave + m_dfWinchCommand4ZeroHeave;
    MOOSAbsLimit(dfDesiredWinch, 100.);
    notify.withDouble("WINCH", dfDesiredWinch);

    // Thruster Magnitudes

    dfStarboardThrust = dfPortThrust = dfDesiredSurge;
    dfStarboardThrust += dfDesiredYaw;
    dfPortThrust -= dfDesiredYaw;

    //Take the sqrt to linearize forces & scale back to 100;
    dfStarboardThrust = SignedSqrt(dfStarboardThrust) * dfLinearScale;
    dfPortThrust = SignedSqrt(dfPortThrust) * dfLinearScale;

    MOOSAbsLimit(dfStarboardThrust, 100.);
    MOOSAbsLimit(dfPortThrust, 100.);

    notify.withDouble("STARBOARD_THRUST", dfStarboardThrust);
    notify.withDouble("PORT_THRUST", dfPortThrust);


    //SWAY
    dfBowCBT   = dfDesiredSway;
    dfSternCBT = dfDesiredSway;
    
    //YAW
    dfBowCBT   -= dfDesiredYaw;
    dfSternCBT += dfDesiredYaw;

    //Take the sqrt to linearize forces & scale back to 100;
    dfBowCBT = SignedSqrt(dfBowCBT) * dfLinearScale;
    dfSternCBT = SignedSqrt(dfSternCBT) * dfLinearScale;

    MOOSAbsLimit(dfBowCBT, 100.);
    MOOSAbsLimit(dfSternCBT, 100.);

    notify.withDouble("BOW_XBODY", dfBowCBT);
    notify.withDouble("STERN_XBODY", dfSternCBT);

    return true;
}

