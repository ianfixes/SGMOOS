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

// MOOSJRKerrDriver.h: interface for the CMOOSJRKerrDriver class.
//
//////////////////////////////////////////////////////////////////////
#ifndef JRKerrMotorDriver_H
#define JRKerrMotorDriver_H

#include "MOOSMotorDriver.h"

/**
 * FIXME: needs class description
 */
class CMOOSJRKerrMotorDriver : public CMOOSMotorDriver  
{
public:
    CMOOSJRKerrMotorDriver();
    ~CMOOSJRKerrMotorDriver();

    bool Initialise();
    bool SetPercentOutput(double dfPercent);
    bool SetPosition(double pos);
    bool SetAcceleration(double acc);
    bool SetPositionModeVelocity(double dec);
    bool SendCommand(const std::string& cmd);
    bool IsEnabled();
    bool Enable(bool bSetStatus);
    bool SetAddress(int nAddress);
    bool GroupReset();
    bool SetGains();
protected:
    bool m_bMoveDone;
    string m_sLogFileName;
    bool SetPwmMode();    
    double m_dfPositionModeVelocity;
    static bool m_bGroupResetDone;

    bool SendCmdAndReadReply(int nAddress, int nCmd, char *Data, int nTries=5);
    void SendCmd(int nAddress, int nCmd, char *Data);
    bool ReadReply(int nAddress, int nCmd, char *Data);
    bool DoPositionMove(int position, int velocity, int acceleration, int address);
    void TraceKerrMessage(char * pMsg,int nLen);
//    bool LogPosition();
//    bool SyncLog();
//    double GetRPM();

};

#endif
