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
// C++ Implementation: MOOSParkerMotorDriver
//
// Description: 
//
//
// Author: Justin G Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "MOOSParkerMotorDriver.h"

using namespace std;

CMOOSParkerMotorDriver::CMOOSParkerMotorDriver()
 : CMOOSMotorDriver()
{   

    m_sDriverName="Parker";
    m_dfPositionControlBandwidth = 5.0;
    m_dfCurrentControlBandwidth = 100.0;
    
    m_dfLoad2MotorInertiaRatio = 2.0;    
    m_nFeedbackCountsPerRevolution = 4000;

}


CMOOSParkerMotorDriver::~CMOOSParkerMotorDriver()
{
}

bool CMOOSParkerMotorDriver::Initialise()
{
    bool bOK = true;
    
        
    bOK = bOK & SetParkerValue("ECHO", false);  // Echoing off
    bOK = bOK & SetParkerValue("COMEXC", true);  // Continuous Command Execution
    bOK = bOK & SetParkerValue("LH", 0);  // Hardware end of travel limit (disable)
    
    bOK = bOK & SetParkerValue("LJRAT", m_dfLoad2MotorInertiaRatio);  
 
    bOK = bOK & SetParkerValue("DPBW", m_dfPositionControlBandwidth);  // Position Control Bandwidth
    bOK = bOK & SetParkerValue("DIBW", m_dfCurrentControlBandwidth);  // Current Control Bandwidth
//    bOK = bOK & SetParkerValue("DRIVE", true);  //Enable Drive
    
    MOOSTrace("Parker Motor Initialization: %s\n ", (bOK) ? "OK" : "Failed");
    return bOK;
}

bool CMOOSParkerMotorDriver::GetPosition(double& pos)
{
    string sVal;
    bool bOK = QueryParkerValue("TPE", sVal);
    pos = atof(sVal.c_str());
    return bOK;
}

bool CMOOSParkerMotorDriver::SetPosition(double pos)
{   
    int nPositionInCounts = (int)floor((pos*((float)m_nFeedbackCountsPerRevolution/360.0))+0.5);
    
    bool bOK = SetParkerValue("MC", 0); //Position Mode
    bOK = bOK && SetParkerValue("D", MOOSFormat("%s%ld", (nPositionInCounts<0) ? "-" : "+", nPositionInCounts));
    bOK = bOK && SendCommand("GO");
    return bOK;
}

bool CMOOSParkerMotorDriver::GetVelocity(double& vel)
{
    string sVal;
    bool bOK = QueryParkerValue("TVELA", sVal);
    vel = atof(sVal.c_str()); // * 60.0;  //RPS to RPM
    return bOK;
}

bool CMOOSParkerMotorDriver::SetVelocity(double vel)
{
/*
With continuous command execution enabled (COMEXC1), if you wish motion to stop before
executing the subsequent move, place a WAIT(AS.1=bØ) statement before the
subsequent GO command. If you wish to ensure the load settles adequately before the nex
move, use the WAIT(AS.24=b1) command instead (this requires you to define end-of-
move settling criteria — see Target Zone Mode on page 84 for details).
*/
    if(m_nMode != VELOCITY)
    {
        MOOSTrace("Error: You've commanded a velocity, but aren't in velocity mode.\n");
	return false;
    }

    double dfNewVel = vel; // / 60.0;  //RPM to RPS
    double dfOldVel;

    if (!GetVelocity(dfOldVel))
        return false;
    
//    dfOldVel /= 60.0;  // convert back to RPS
//    double dfOldVelocityAmplitude = fabs(dfOldVel);
//    bool bOldVelocitySign = (dfOldVelocityAmplitude > 0.0001) ? 
//        (dfOldVel >= 0) : true;

    string sOldVelocitySign;
    if(!QueryParkerValue("D", sOldVelocitySign))
        return false;
    
    bool bOldVelocitySign = (atoi(sOldVelocitySign.c_str()) > 0.0);// ? (dfOldVel >= 0) : true;


    double dfNewVelocityAmplitude = fabs(dfNewVel);
    bool bNewVelocitySign = (dfNewVel >= 0);

    if(IsInMotion() && (bOldVelocitySign != bNewVelocitySign))
    {
        return SendCommand( "S");
    }

    if(!IsInMotion())
    {
      string sNewSign = (bNewVelocitySign) ? "+1" : "-1";
      if(!SetParkerValue( "D", sNewSign))
        return false;
    }

    if(SetParkerValue("V", dfNewVelocityAmplitude)) // Commanded Velocity
        return SendCommand("GO");
    else 
        return false;
     
}

// ***********************************************************
// Set average & maximum (ac/de)celeration values to the same
// value for trapeziodal motion (s-curve does not work with 
// continuous command execution. 
// ***********************************************************

bool CMOOSParkerMotorDriver::SetAcceleration(double acc)
{
    return SetParkerValue("A", acc) && SetParkerValue("AA", acc); 
}

bool CMOOSParkerMotorDriver::SetDeceleration(double dec)
{
    return SetParkerValue("AD", dec) && SetParkerValue("ADA", dec);
}

bool CMOOSParkerMotorDriver::IsInMotion()
{
    string sTAS;
    if(QueryParkerValue("TAS", sTAS))
        return sTAS[0]=='1';
    
    return false;
}


bool CMOOSParkerMotorDriver::IsEnabled()
{
    string sEnableStatus;
    QueryParkerValue("DRIVE", sEnableStatus);
    return (sEnableStatus == "1");
}

bool CMOOSParkerMotorDriver::Enable(bool bSetStatus)
{
    // The drive must be stopped to disable...
    // So, stop the drive, and wait for it to stop
    if(!bSetStatus && IsEnabled() && IsInMotion())
    {
        SendCommand("!K");
        SendCommand("WAIT(AS.1=b0)");
    }
    return SetParkerValue("DRIVE", bSetStatus);
}

bool CMOOSParkerMotorDriver::SetVelocityMode()
{
    SetParkerValue("MC", 1); // Velocity Mode
    return true;
}

bool CMOOSParkerMotorDriver::GetStatus(int& nCode, std::string& sDescription)
{
    string sTAS, sDrive;
    nCode = 0;

    if(!QueryParkerValue("TAS", sTAS) || !QueryParkerValue("DRIVE",sDrive))
    {
       sDescription = "Comms Error";
       return false;
    }

    if(sDrive[0] == '1')
    {
        sDescription = "Enabled";
    }
    else
    {
        if(sTAS[17] == '1')
        {
            sDescription = "FAULT";
        }
        else
        {
            sDescription = "Disabled";
        }
    }
    return true;
}

bool CMOOSParkerMotorDriver::GetTAS()
{
    string sTAS;
    if(QueryParkerValue("TAS", sTAS))
    {
        if(sTAS.length() == 32 + 7) //32 bits + 7 separators
        {
            MOOSTrace("Got TAS: " + sTAS + "\n");
            for(int i=0; i<32; i++)
                m_TransferAxisStatus[i] = (sTAS[i + i/4] == '1');
            
            return true;
        }
    }
    return false;
}

bool CMOOSParkerMotorDriver::SetParkerValue(const char* sVarName, bool bVal)
{
    string sVal = (bVal) ? "1" : "0";
    return  SetParkerValue(sVarName, sVal);
}

bool CMOOSParkerMotorDriver::SetParkerValue(const char* sVarName, double dfVal)
{

    string sVal = MOOSFormat("%.4f", dfVal);

    string sStringToSend = sVarName + sVal;
    if(SendCommand(sStringToSend))
    {
        // confirm it was set properly
        string sNewVal;
        if(QueryParkerValue( sVarName, sNewVal))
        {
            double dfNewVal = atof(sNewVal.c_str());
            //if(dfVal != dfNewVal)
            if(false)
            {
                MOOSTrace("Comms good, but value(%s) didn't get set (%f != %f)\n", sVarName, dfVal, dfNewVal);
                return false;
            }
            else return true;
        }
    }
    return false;

}


bool CMOOSParkerMotorDriver::SetParkerValue(const char* sVarName, int nVal)
{
    string sVal = MOOSFormat("%d", nVal);
    return  SetParkerValue(sVarName, sVal);
}

bool CMOOSParkerMotorDriver::SetParkerValue(const char* sVarName, const std::string& sVal)
{

    string sStringToSend = sVarName + sVal;
    if(SendCommand(sStringToSend))
    {
        // confirm it was set properly
        string sNewVal;
        if(QueryParkerValue( sVarName, sNewVal))
            if(sVal != sNewVal)
            {
                MOOSTrace("Comms good, but value (%s) didn't get set (%s != %s)\n", sVarName, sVal.c_str(), sNewVal.c_str());
                return false;
            }
    }
    else return false;

    
    return true;
}


bool CMOOSParkerMotorDriver::SendCommand(const std::string& sCmd)
{
    string sReply;
    bool bOK = false;
    MOOSTrace("Commanding: " + sCmd + "\n");
    string sCmdCR = sCmd + "\r";

    m_pPort->Write((char*)sCmdCR.c_str(), sCmdCR.size());
    
    if (GetParkerTelegram(sReply))
    {  
        if(sReply == sCmd)
        {
	    bOK = true;
        }
        
        while(GetParkerTelegram(sReply))
        {
	    MOOSTrace("ERROR: got reply %s\n", sReply.c_str());
	    bOK = false;
        }
    }
    
    return true;

}

bool CMOOSParkerMotorDriver::QueryParkerValue(const char* cVarName, std::string &sVal)
{
    
    string sReply;

    string sVarName = cVarName;  // let's deal w/ a string internally
    
    //... and make it upper case.
    MOOSToUpper(sVarName);
    
    string sVarNamePlusCR = sVarName + "\r";
    
    m_pPort->Write((char*)sVarNamePlusCR.c_str(), sVarNamePlusCR.length());
    
    string sTheEcho;
    
    
    if(!(GetParkerTelegram(sTheEcho) && GetParkerTelegram(sReply)))
    {
        MOOSTrace("ERROR: No Response from Parker\n");
	return false;
    }
    
    // for some reason, I've seen both of these reply formats
    if(sTheEcho != "> " + sVarName && sTheEcho !=  sVarName)
    {
       MOOSTrace("FAILURE: Parker responded: " + sTheEcho + ", supposed to echo " + sVarName + "\n");
       return false;
    }
    
    if(sReply[0] == '*'){  //first character is an asterisk.
       if(sReply.compare(1,sVarName.length(),sVarName)==0)
       {
            sReply.erase(0, sVarName.length()+1);
            sVal = sReply;  //Now, that's the value
            return true;
       }
    }


    MOOSTrace("FAILURE: Parker responded: " + sTheEcho + "\n");
    return false;
}

// Get Telegram Wrapper: to get rid of indeterminite # of blank lines. 
bool CMOOSParkerMotorDriver::GetParkerTelegram(std::string& sReply)
{
    while(m_pPort->GetTelegram(sReply, 0.1))
    {
	// get rid of prompt, if exists
        if(sReply.compare(0,2, "> ")==0)
            sReply.erase(0, 2);

        if(!sReply.empty())
        {
	    return true;
        }
    }
    return false;
}

void CMOOSParkerMotorDriver::ClearSerialBuffer()
{
    string sReply;
//    while(m_pPort->Flush() != 0){}
    while(m_pPort->GetTelegram(sReply, 0.1)){}
}

