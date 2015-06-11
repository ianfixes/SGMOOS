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
// C++ Interface: MOOSMotorDriver
//
// Description: 
//
//
// Author: Justin G Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MOOSMOTORDRIVER_H
#define MOOSMOTORDRIVER_H


#include <MOOSGenLib/MOOSGenLib.h>
#include <math.h>
using namespace std;

enum ModeEnumType {POSITION, VELOCITY, PWM};

/**
Class to define the iterface of between MOOS instrument
	@author Justin G Eskesen <jge@mit.edu>
*/
class CMOOSMotorDriver
{
public:
    CMOOSMotorDriver();
    ~CMOOSMotorDriver();

    std::string GetDriverName();
    //Initialization methods
    //Values are read (from configuration) & stored by the Motor Instument class
    //I'm just the messanger
    void SetSerial(CMOOSSerialPort* port);
//    virtual bool SetHomeToCurrent()=0;

//    virtual bool SetVelocityLimit(double vel)=0;
    virtual bool SetAcceleration(double acc);
    virtual bool SetDeceleration(double dec);
    virtual bool SetPositionModeVelocity(double vel);

    virtual bool Initialise();
    
    //Motor Control Methods 
    //position control... 
    virtual bool GetPosition(double& pos);
    //virtual bool GetPositionError(double& err)=0;
    virtual bool SetPosition(double pos);

    virtual bool GetCurrent(double& dfI) {return false;}

    //velocity control....
    virtual bool GetVelocity(double &vel);
    virtual bool SetVelocity(double vel);

    virtual bool SetPercentOutput(double dfPercent);

    virtual bool GetStatus(int& nCode, std::string& sDescription);

    virtual bool SendCommand(const std::string& cmd)=0;
    virtual bool IsEnabled()=0;
    virtual bool Enable(bool bSetStatus)=0;
    bool SetMode(ModeEnumType mode); 
    ModeEnumType  GetMode();
    void SetAutoWrap(bool bOnOff) { m_bAutoWrap=bOnOff; }
    virtual bool SetAddress(int nAddress) { m_nAddress = nAddress; return true;}

protected:
    virtual bool SetPwmMode();
    virtual bool SetPositionMode();
    virtual bool SetVelocityMode(); 
    int ConvertDegreesToCounts(double dfDegrees);


    int m_nCurrentPosition;  // Current position (in Ticks)

    ModeEnumType m_nMode;

    std::string m_sDriverName;
    int m_nFeedbackCountsPerRevolution;
    CMOOSSerialPort* m_pPort;    
    
    double m_dfVelocityLimit; //Rotations per second (for position control)
    double m_dfAcceleration; //Rotations per second squared
    double m_dfDeceleration;  //Rotations per second squared

    int m_nAddress;  //For multidrop networks
    bool m_bAutoWrap;
};

#endif

