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
#include "MOOSMotorDriver.h"

using namespace std;

CMOOSMotorDriver::CMOOSMotorDriver()
{
    m_nAddress=0;
    m_bAutoWrap=false;
    m_nCurrentPosition=0; // In Ticks.
    m_sDriverName = "MotorBaseClass_DoNotUse";
}

CMOOSMotorDriver::~CMOOSMotorDriver()
{
}

void CMOOSMotorDriver::SetSerial(CMOOSSerialPort* port)
{
    m_pPort = port;
}

// Below are a bunch of functions which to be implemented by 
// child classes.  Default values returned for unimplemented
// functions.
 
bool CMOOSMotorDriver::Initialise()
{
    return true;
}

bool CMOOSMotorDriver::GetPosition(double& pos)
{
    return false;
}

bool CMOOSMotorDriver::SetPosition(double pos)
{
    return false;
}

int CMOOSMotorDriver::ConvertDegreesToCounts(double dfDegrees)
{

    // Query controller so that we know m_nCurrentPosition is current.
    double dfJunk;
    GetPosition(dfJunk);

    int nPositionInCounts = (int)floor((dfDegrees*((float)m_nFeedbackCountsPerRevolution/360.0))+0.5);

    if(m_bAutoWrap)
    {

        int nWraps = m_nCurrentPosition / m_nFeedbackCountsPerRevolution;

        int nDelta = (nPositionInCounts % m_nFeedbackCountsPerRevolution) - 
                     (m_nCurrentPosition % m_nFeedbackCountsPerRevolution);
        
        MOOSTrace("Delta: %d\n", nDelta);
        if(nDelta > m_nFeedbackCountsPerRevolution/2)
            nWraps-=1;
        else if (nDelta < -m_nFeedbackCountsPerRevolution/2)
            nWraps+=1;

        nPositionInCounts += nWraps*m_nFeedbackCountsPerRevolution;
    }

    return nPositionInCounts;
}

bool CMOOSMotorDriver::GetVelocity(double& vel)
{
    return false;
}

bool CMOOSMotorDriver::SetVelocity(double vel)
{
    return false;
}

bool CMOOSMotorDriver::SetPositionModeVelocity(double vel)
{
    return false;
}

bool CMOOSMotorDriver::SetPercentOutput(double dfPercent)
{
    return false;
}

bool CMOOSMotorDriver::SetAcceleration(double acc)
{
    return false;
}

bool CMOOSMotorDriver::SetDeceleration(double dec)
{
    return false;
}

bool CMOOSMotorDriver::GetStatus(int& nCode, std::string& sDescription)
{
    nCode=0;
    sDescription = "Not Implemented.";
    return true;
}

bool CMOOSMotorDriver::SetMode(ModeEnumType mode)
{
    bool bOk = false;
    switch(mode)
    {
        case POSITION: bOk=SetPositionMode();	break;
        case VELOCITY: bOk=SetVelocityMode();	break;
        case PWM:      bOk=SetPwmMode();	break;
    }
    if(bOk)
        m_nMode=mode;

    return bOk;
}

bool CMOOSMotorDriver::SetPositionMode()
{
    return false;
}

bool CMOOSMotorDriver::SetVelocityMode()
{
    return false;
}

bool CMOOSMotorDriver::SetPwmMode()
{
    return false;
}

ModeEnumType CMOOSMotorDriver::GetMode()
{
    return m_nMode;
}
