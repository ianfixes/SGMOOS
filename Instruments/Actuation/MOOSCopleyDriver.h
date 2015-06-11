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

*************************************************************************///
// C++ Interface: MOOSCopleyDriver
//
// Description: 
//
//
// Author: Justin G Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MOOSCOPLEYDRIVER_H
#define MOOSCOPLEYDRIVER_H

#include "MOOSMotorDriver.h"
/**
MOOS driver for a Copley Motor Controller.  Originally, for use in 
a Rotating Thruster Unit on Odyssey IV.

	@author Justin G Eskesen <jge@mit.edu>
*/

using namespace std;

/**
 * FIXME: needs class description
 */
class CMOOSCopleyDriver : public CMOOSMotorDriver
{
public:
    CMOOSCopleyDriver();
    ~CMOOSCopleyDriver();
    
//    bool SetHomeToCurrentPosition();
    
//    bool SetVelocityLimit(double vel);
//    bool SetAcceleration(double acc);
//    bool SetDeceleration(double dec);
    
    //Motor Control Methods 
    //position control... 
    bool GetPosition(double& pos);
//    bool GetPositionError(double& err);
    bool SetPosition(double pos);

    //velocity control....
    bool GetVelocity(double &vel);
    bool SetVelocity(double vel);

    bool GetCurrent(double& dfI);

    bool SendCommand(const string& cmd);
    bool IsEnabled();
    bool Enable(bool bSetStatus);
    bool GetStatus(int& nCode, string& sDescription);

protected:

    bool SetPositionMode();
    bool SetVelocityMode();
    bool GetRAMValue(int nAddress, int& nValue);
    bool SetRAMValue(int nAddress, int nValue);
    bool SetTrajectoryGenerator(int nValue);
    bool DoIO(const string& sCommand, int& nValue);
    bool DoIO(const string& sCommand);
    void FillStatusMessages();
    int m_nFault;
    int m_nLastErrorCode;
    string GetErrorString(int nErrorCode);
    
    double m_dfMaxAcceleration; //counts per second squared
    double m_dfMaxDeceleration; //counts per second squared
    double m_dfMaxVelocity; //counts per second
    
//    Profile type.
//          0 = Absolute move, trapezoidal profile.
//          1 = Absolute move, S-curve profile.
//          256 = Relative move, trapezoidal profile.
//          257 = Relative move, S-curve profile.
//          2 = Velocity move.
    int m_nProfileType; 
    int m_nDesiredState; // 22 = Programmed Position
    
    //***********************************************************************************************
    // Variable IDs: integers which identify variables in the ASCII command structure
    // NOTE: they appear in documentation as hexidecimals, so we'll maintain that in the code
    //***********************************************************************************************


    vector<string> m_sStatusMessage;
};

#endif
