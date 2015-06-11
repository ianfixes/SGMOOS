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
// C++ Implementation: MOOSCopleyDriver
//
// Description: 
//
//
// Author: Justin G Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <MOOSLIB/MOOSLib.h>
#include "MOOSCopleyDriver.h"
#include <bitset>

#define DESIRED_STATE 0x24
#define PROFILE_TYPE 0xc8
#define POSITION_COMMAND 0xca
#define VELOCITY_COMMAND 0x2f
#define STATUS 0Xa0
#define FAULT 0xa4
#define MOTOR_POSITION 0x32
#define MOTOR_VELOCITY 0x18
#define MOTOR_CURRENT 0x0c

CMOOSCopleyDriver::CMOOSCopleyDriver()
: CMOOSMotorDriver()
{
    m_sDriverName="Copley";
    m_nProfileType=1;  //Absolute, S-curve
    m_nFault = -1; //initialized to an impossible value, since faults are only reported on changes
    m_nFeedbackCountsPerRevolution = 16384;  //On Resolver

    FillStatusMessages();

}

bool CMOOSCopleyDriver::SetPosition(double pos)
{
    int nPositionInCounts = ConvertDegreesToCounts(pos);
    
    MOOSTrace("Converted %f to %d\n", pos, nPositionInCounts);
    if(SetRAMValue(POSITION_COMMAND, nPositionInCounts)) //set desired position
        return(SetTrajectoryGenerator(1)); //enable trajectory generator, this is our "go" command
     
    return false;
}

bool CMOOSCopleyDriver::GetPosition(double& pos){
    int nReturnedValue;

    if(GetRAMValue(MOTOR_POSITION, nReturnedValue))
    {
        m_nCurrentPosition = nReturnedValue;
        if(m_bAutoWrap)
            nReturnedValue %= m_nFeedbackCountsPerRevolution;
        pos = nReturnedValue / ((float)m_nFeedbackCountsPerRevolution/360.0);

        return true;
    }
    return false;
}

bool CMOOSCopleyDriver::SetVelocity(double vel)
{
    int VelocityInTenthsOfCountsPerSecond = (int)floor((vel*((float)m_nFeedbackCountsPerRevolution/360.0)*10.0) + 0.5); 
    
    if(SetRAMValue(VELOCITY_COMMAND, VelocityInTenthsOfCountsPerSecond)) //set desired velocity   
        return SetRAMValue(DESIRED_STATE, 11); //enable in programmed velocity mode
     
    return false;
}


bool CMOOSCopleyDriver::GetVelocity(double& vel){
    int nReturnedValue;
    if(GetRAMValue(MOTOR_VELOCITY, nReturnedValue))
    {
        vel = nReturnedValue / ((float)m_nFeedbackCountsPerRevolution/360.0);
        return true;
    }
    return false;
}

bool CMOOSCopleyDriver::GetCurrent(double& dfI)
{
    int nReturnedValue;
    if(GetRAMValue(MOTOR_CURRENT, nReturnedValue))
    {
        dfI = (double)nReturnedValue / 100.0;
        return true;
    }
    return false;
}

bool CMOOSCopleyDriver::SendCommand(const string& cmd){
    return DoIO(cmd);
}

bool CMOOSCopleyDriver::IsEnabled()
{
    return true;
}

bool CMOOSCopleyDriver::SetPositionMode()
{
    m_nDesiredState=21; //enable in programmed position mode
    return SetRAMValue(PROFILE_TYPE, m_nProfileType); //profile type
}

bool CMOOSCopleyDriver::SetVelocityMode()
{
    m_nDesiredState=11; //enable in programmed velocity mode
    return true;

}

bool CMOOSCopleyDriver::Enable(bool bSetStatus)
{
    return bSetStatus ?
        SetRAMValue(DESIRED_STATE, m_nDesiredState) : //enable in desired mode
        SetRAMValue(DESIRED_STATE, 0);  //disable
        
}

bool CMOOSCopleyDriver::GetStatus(int& nCode, string& sDescription)
{
    int nStatusRegister;
    if(!GetRAMValue(STATUS, nStatusRegister))
        return false;

    bitset<32> status((long)nStatusRegister);
    sDescription.clear();

    if(status.none())
    {    
        sDescription = "Enabled";
        return true;
    }
    else
    {
        for(int i=0; i<32; i++)
            if(status.test(i))
                sDescription+=m_sStatusMessage[i];

        /*
        // print bitmask to string... useful for debugging
        sDescription = status.to_string();
        sDescription.insert(24," ");
        sDescription.insert(16," ");
        sDescription.insert(8," ");
        */

        return true;
    }
    
}


bool CMOOSCopleyDriver::GetRAMValue(int nAddress, int& nValue)
{
    string sCommand = MOOSFormat("g r0x%x", nAddress);
    return DoIO(sCommand, nValue);
}


bool CMOOSCopleyDriver::SetRAMValue(int nAddress, int nValue)
{
    string sCommand = MOOSFormat("s r0x%x %d", nAddress, nValue);
    return DoIO(sCommand);
}

bool CMOOSCopleyDriver::SetTrajectoryGenerator(int nValue)
{
    string sCommand = MOOSFormat("t %d", nValue);
    return DoIO(sCommand);
}

bool CMOOSCopleyDriver::DoIO(const string& sCommand, int& nValue)
{
    string sCommandPlusCR = sCommand + "\r\n";
    
    if(m_pPort->IsVerbose())
        MOOSTrace("Sending to Copley: " + sCommandPlusCR);
        
    if(!m_pPort->Write((char*)sCommandPlusCR.c_str(), sCommandPlusCR.length()))
        MOOSTrace("WARNING: Unsuccessful serial port write\n");
        
    if(sCommand[0] == 'r')
        return true;  // there is no response on a reset command.

    string sReply;
    if(!m_pPort->GetTelegram(sReply, 1.0))
    {
        MOOSTrace("WARNING: No Response from Copley Motor Controller\n");
        return false;
    }
    
//   this->AddNotification("COPLEY_RESPONSE", sReply.c_str());
    
    // Successful set, copy, trajectory generator
    if(sReply == "ok")
        return true;

    string sResponseIndicator = MOOSChomp(sReply, " ");
    int nReturnedValue = atoi(sReply.c_str());

    // Handle succesesful requests for values
    if(sResponseIndicator == "v")
    {
        nValue = nReturnedValue;
        return true;
    }
    
    // Anything else is an error.. deal with it!
    if(sResponseIndicator == "e")
    {
        string sOutput = "Copley Error: " + sCommand + " -> " + sReply;
//        this->AddNotification("COPLEY_RESPONSE", sOutput.c_str());
//        MOOSTrace("Copley Error: " + GetErrorString(nReturnedValue) + "\n");
//        throw CMOOSException(sOutput);
    }
    else 
        MOOSTrace("Unkown response from Copley\n");
    
    return false;
}

//most I/O doesn't require a returned value, just a bool...
bool CMOOSCopleyDriver::DoIO(const string& sCommand)
{
    int nFakeReturnedValue = 0;
    return DoIO(sCommand, nFakeReturnedValue);
}

void CMOOSCopleyDriver::FillStatusMessages()
{
    m_sStatusMessage.push_back("Short Circuit ");
    m_sStatusMessage.push_back("Amp Over Temperature. ");
    m_sStatusMessage.push_back("Over Voltage. ");
    m_sStatusMessage.push_back("Under voltage. ");
    m_sStatusMessage.push_back("Motor Over Temperature. ");
    m_sStatusMessage.push_back("Feedback Error. ");
    m_sStatusMessage.push_back("Motor Phasing Error. ");
    m_sStatusMessage.push_back("Current Limited. ");
    m_sStatusMessage.push_back("Voltage Limited. ");
    m_sStatusMessage.push_back("Positive Limit Switch. ");
    m_sStatusMessage.push_back("Negative Limit Switch. ");
    m_sStatusMessage.push_back("Amp Disabled by Hardware. ");
    m_sStatusMessage.push_back("Amp Disabled by Software. ");
    m_sStatusMessage.push_back(""); //("Attempting to Stop Motor. ");
    m_sStatusMessage.push_back(""); //("Motor Brake Active. ");
    m_sStatusMessage.push_back("PWM Outputs Disabled. ");
    m_sStatusMessage.push_back("Positive Software Limit. ");
    m_sStatusMessage.push_back("Negative Software Limit. ");
    m_sStatusMessage.push_back("Following Error. ");
    m_sStatusMessage.push_back("Following Warning. ");
    m_sStatusMessage.push_back("Amplifier has been reset. ");
    m_sStatusMessage.push_back("Encoder position wrapped (rotary) or hit limit (linear). ");
    m_sStatusMessage.push_back("Amplifier Fault. ");
    m_sStatusMessage.push_back("Velocity Limited. ");
    m_sStatusMessage.push_back("Acceleration Limited. ");
    m_sStatusMessage.push_back("Pos Outside of Tracking Window. ");
    m_sStatusMessage.push_back("Home Switch Active. ");
    m_sStatusMessage.push_back("In Motion. ");
    m_sStatusMessage.push_back("Velocity Outside of Tracking Window. ");
    m_sStatusMessage.push_back("Phase not Initialized. ");
    m_sStatusMessage.push_back("Unused. ");
    m_sStatusMessage.push_back("Unused. ");

}
