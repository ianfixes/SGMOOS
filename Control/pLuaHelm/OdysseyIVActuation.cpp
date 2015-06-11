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
// C++ Implementation: OdysseyIVActuation
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
#include "OdysseyIVActuation.h"
using namespace std;

//this is a percentage
#define MIN_THRUST_TO_MAKE_RTU_WORTHWHILE 3
#define CRUISE_MODE_ANGLE 10
#define CRUISE_MODE_THRUST_THRESH 5

COdysseyIVActuation::COdysseyIVActuation()
: CMOOSActuationMap()
{
    m_nValidDOFs = 39 ; // Surge, Sway, Heave, & Yaw (111001)
    m_ActuatorList.push_back("RTU");
    m_ActuatorList.push_back("STARBOARD_VECTORED");
    m_ActuatorList.push_back("PORT_VECTORED");
    m_ActuatorList.push_back("BOW_XBODY");
    m_ActuatorList.push_back("STERN_XBODY");

    m_dfBowSternRatio = 1.0;
    m_dfMaxThrust = 1600.0;

    m_dfPropLinearScalingFactor = 160.0;
}


COdysseyIVActuation::~COdysseyIVActuation()
{

}

bool COdysseyIVActuation::ReadConfig(CProcessConfigReader& Config)
{
    Config.GetConfigurationParam("BowSternRatio", m_dfBowSternRatio);
    return true;
}

STRING_LIST COdysseyIVActuation::Registrations() const
{
    STRING_LIST ret;
    ret.push_back("NAV_PITCH");
    return ret;
}

bool COdysseyIVActuation::Engage(const CPathAction& DesiredAction, MSG_MAP &msgs, MOOS_Notifier notify)
{

    bool bIsCruisingMode = true;

    // Let's get the forces we want out of the map;
    double dfDesiredSurge, dfDesiredHeave, dfDesiredYaw, dfDesiredSway;
    double dfBowForce, dfSternForce, dfStbdForce, dfPortForce;
    double dfNow = MOOSTime();

    //double dfLinearScaling = 160.;
    

    //Surge / Heave controlled by starboard & port thrusters + RTU
    //lets read in all of our DOF's, setting to zero if not set.
    dfDesiredSurge = DesiredAction.GetDesiredForce(SURGE);
    dfDesiredHeave = DesiredAction.GetDesiredForce(HEAVE);
    dfDesiredYaw   = DesiredAction.GetDesiredForce(YAW);
    dfDesiredSway  = DesiredAction.GetDesiredForce(SWAY);

    bIsCruisingMode &= fabs(dfDesiredSurge) > CRUISE_MODE_THRUST_THRESH;


    dfStbdForce = dfPortForce = sqrt(pow(dfDesiredHeave, 2) + pow(dfDesiredSurge, 2));

    if(dfStbdForce >= MIN_THRUST_TO_MAKE_RTU_WORTHWHILE) //don't want it spinning erratically.
    {
        double dfRTUAngle = MOOSRad2Deg(atan2(dfDesiredHeave, dfDesiredSurge));

        double dfAbsRTU = fabs(dfRTUAngle);
        bIsCruisingMode &=  dfAbsRTU < CRUISE_MODE_ANGLE;

        MSG_MAP::iterator pitch;

        //pick up value of pitch
        if((pitch = msgs.find("NAV_PITCH")) != msgs.end())
        {
            m_NavPitch = (pitch->second);
        }

        //if pitch info is fresh, compensate
        if((dfNow - m_NavPitch.GetTime()) < 5.0)
        {
            dfRTUAngle -= MOOSRad2Deg(m_NavPitch.GetDouble());
        }

        notify.withDouble("DESIRED_RTU", dfRTUAngle);
    }
    
    //YAW
    if(bIsCruisingMode)
    {
        dfStbdForce += dfDesiredYaw;
        dfPortForce -= dfDesiredYaw;
    }
    else
    {
        dfBowForce   = dfDesiredYaw;
        dfSternForce = -1.0*dfDesiredYaw;
    }

    //SWAY
    dfBowForce   -= dfDesiredSway;
    dfSternForce -= dfDesiredSway;

    dfBowForce *= m_dfBowSternRatio;
    dfSternForce /= m_dfBowSternRatio;

    notify.withDouble("DESIRED_BOW_XBODY", Force2PropRPM(dfBowForce));
    notify.withDouble("DESIRED_STERN_XBODY", Force2PropRPM(dfSternForce));
    notify.withDouble("DESIRED_STARBOARD_VECTORED", Force2PropRPM(dfStbdForce));
    notify.withDouble("DESIRED_PORT_VECTORED", Force2PropRPM(dfPortForce));

    return true;
}

