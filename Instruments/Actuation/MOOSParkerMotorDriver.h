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
// C++ Interface: MOOSParkerMotorDriver
//
// Description: 
//
//
// Author: Justin G Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MOOSPARKERMOTORDRIVER_H
#define MOOSPARKERMOTORDRIVER_H

#include "MOOSMotorDriver.h"

/**
Implementation of a MOOSMotorDriver for a Parker Motor.

	@author Justin G Eskesen <jge@mit.edu>
*/
class CMOOSParkerMotorDriver : public CMOOSMotorDriver
{
public:
    CMOOSParkerMotorDriver();

    ~CMOOSParkerMotorDriver();
    
    bool Initialise();
    
    //position control...
    bool GetPosition(double& pos);
    bool SetPosition(double pos);

    //velocity control....
    bool GetVelocity(double &vel);
    bool SetVelocity(double vel);
    
    bool SetAcceleration(double acc);
    bool SetDeceleration(double dec);
    
    bool GetStatus(int& nCode, std::string& sDescription);
    
    bool SendCommand(const std::string& cmd);
    bool IsEnabled();
    bool Enable(bool bSetStatus);
    bool IsInMotion();

protected:
    bool GetTAS();
    bool SetVelocityMode();
    bool SetParkerValue(const char* sVarName, bool bVal);
    bool SetParkerValue(const char* sVarName, int nVal);
    bool SetParkerValue(const char* sVarName, double dfVal);
    bool SetParkerValue(const char* sVarName, const std::string& sVal);

    bool QueryParkerValue(const char* sVarName, std::string& sVal);
    bool GetParkerTelegram(std::string& sReply);    
    void ClearSerialBuffer(); 
    double m_dfPositionControlBandwidth;
    double m_dfCurrentControlBandwidth;
    
    double m_dfLoad2MotorInertiaRatio;

    bool m_TransferAxisStatus[32];
};

#endif
