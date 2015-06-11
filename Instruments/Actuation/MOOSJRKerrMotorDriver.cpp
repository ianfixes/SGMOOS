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

// MOOSJRKerrDriver.cpp: implementation of the CMOOSJRKerrMotorDriver class.
//
// Driver for three JR Kerr / Kerr Automation Engineering
// PIC-SERVO-3PH motor controllers in a tailcone for an Odyssey II class vehicle
//
// This code does some slightly non-portable things like assume ints
// are always 32 bits and chars always 8 bits, but we'll live with it
// for now.
//
// 2/2002 - jmorash@alum.mit.edu
//
//////////////////////////////////////////////////////////////////////
#include <MOOSLIB/MOOSLib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include "MOOSJRKerrMotorDriver.h"

#define JRKERR_UNKNOWN -1;
#define JRKERR_ADDR_INITIAL    0x00
#define JRKERR_ADDR_GROUP      0xFF

//Command bytes are: upper nibble = number of following Data bytes, lower nibble = command number
#define JRKERR_RESET_POSITION       0x00
#define JRKERR_SET_ADDRESS          0x21
#define JRKERR_DEFINE_STATUS        0x12
#define JRKERR_READ_STATUS          0x13
#define JRKERR_LOAD_TRAJECTORY_POS  0xD4
#define JRKERR_LOAD_TRAJECTORY_PWM  0x24
#define JRKERR_START_MOTION         0x05
#define JRKERR_SET_GAINS            0xD6
#define JRKERR_STOP_MOTOR           0x17
#define JRKERR_IO_CONTROL           0x18
#define JRKERR_SET_HOME_MODE        0x19
#define JRKERR_SET_BAUD_RATE        0x1A
#define JRKERR_CLEAR_BITS           0x0B
#define JRKERR_SAVE_AS_HOME         0x0C
#define JRKERR_NOP                  0x0E
#define JRKERR_HARD_RESET           0x0F


#define JRKERR_ENCODER_TICKS_PER_DEGREE 2.67

#define JRKERR_MAX_PWM 255
#define JRKERR_MIN_PWM 0

#define HARDWARE_FAILURE_TO 4.0
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSJRKerrMotorDriver::CMOOSJRKerrMotorDriver()
{
    m_bMoveDone = true;
    m_sLogFileName="JRKerrLog.txt";
    m_sDriverName = "JRKerr";

    m_nAddress=1; //unless otherwise set

    // default values borrowed from iActuation
    m_nFeedbackCountsPerRevolution=960;
    m_dfPositionModeVelocity=100000.0/m_nFeedbackCountsPerRevolution;
    m_dfAcceleration=100/m_nFeedbackCountsPerRevolution;
}

// initialize static variable
bool CMOOSJRKerrMotorDriver::m_bGroupResetDone=false;

CMOOSJRKerrMotorDriver::~CMOOSJRKerrMotorDriver()
{
    
}

void CMOOSJRKerrMotorDriver::TraceKerrMessage(char * pMsg,int nLen)
{

    
    //print out command sent (hex values)
    if(m_pPort->IsVerbose())
    {

        MOOSTrace("Sent %02X: %02X", (unsigned char)pMsg[1], (unsigned char)pMsg[2]);
        //print Data bytes, but not checksum
        for (int i=3; i < (nLen-1); i++)
        {
            MOOSTrace(" %02X", (unsigned char)pMsg[i]);
        }
        MOOSTrace("\n");
    }
}

bool CMOOSJRKerrMotorDriver::SendCmdAndReadReply(int nAddress, int nCmd, char *Data, int nTries)
{
    bool bSuccess;
    int nCount=0;
    do
    {
        SendCmd(nAddress, nCmd, Data);
        nCount++;
    }
    while(!(bSuccess=ReadReply(nAddress, nCmd, Data)) && (nCount < nTries));
    
    
    if(nCount > 1)
    {
        string sSuccess = bSuccess ? "Succeeded" : "Failed";
        MOOSTrace("JRKerr had to repeat command %d times before it %s\n", nCount, sSuccess.c_str());
    }
   
    return bSuccess;

}


void CMOOSJRKerrMotorDriver::SendCmd(int nAddress, int nCmd, char *Data)
{
    //This function assembles, checksums and sends commands, then
    //deals with the returned status byte(s)
    
    int nDataLen=0;
    
    
    
    //Commands always start with 0xAA, followed by the module address,
    //then the command byte, then the associated Data bytes, then the
    //checksum.  Data length is implicit in the upper nibble of the
    //command byte. Checksum does not include the preamble (0xAA).
    
    nDataLen = (((unsigned char)nCmd & 0xF0) >> 4) & 0x0F;
    
    char Tx[100];
    int nNdx = 0;
    Tx[nNdx++] = (char)0xAA;
    Tx[nNdx++] = nAddress;
    Tx[nNdx++] = nCmd;
    int i=0;
    for (i=0;i<nDataLen;i++)
    {
        //send Data bytes
        Tx[nNdx++] = Data[i];
    }
    
    //now figure checksum. it's an 8bit wraparound 2's complement sum
    
    char cCheckSum = 0;
    for(i = 1;i<nNdx;i++)
    {
        cCheckSum+=Tx[i];
    }
    
    //append
    Tx[nNdx++] = cCheckSum;
    
    //write
    m_pPort->Write(Tx,nNdx);
    
    if(m_pPort->IsVerbose())
        TraceKerrMessage(Tx,nNdx);
    
}

bool CMOOSJRKerrMotorDriver::ReadReply(int nAddress, int nCmd, char *Data)
{
    //now deal with the returned Data.
    //first handle the initial status byte, which is always there.
    double dfPortTimeOut = 1.5;

    //doesn't return anything.. no need to check.

    if(nCmd == JRKERR_HARD_RESET) 
        return true;
    
    char status[5]="";

    int nBytesInStatus = (nCmd==JRKERR_READ_STATUS) ? 4 : 2;
    //nBytesInStatus++; // Null Character at end

    int nRead = m_pPort->ReadNWithTimeOut(status,nBytesInStatus,dfPortTimeOut);
    
    if (nRead < nBytesInStatus) 
    {
        if(nRead<=0)
            MOOSTrace("No response from JRKerr controller.\n");
        else 
            MOOSTrace("Incomplete read from JRKerr Serial Port\n");
        return false;
    }

    char nStatByte = status[0];
    
    char checksum = nStatByte;
    
    //then, if we're reading extra info,
    
    if (nCmd==JRKERR_READ_STATUS) 
    {
        if (Data[0] & 0x20) 
        {
            if (m_pPort->IsVerbose())
                MOOSTrace("%02X: found device type %2d, version %2d\n", nAddress, status[1], status[2]);
            
            checksum += status[1] + status[2];
        }
    }
    
    if (checksum != status[nBytesInStatus-1])
    {
        MOOSTrace("JRKerr: recieved checksum is incorrect\n");
        return false;
    }
    
    
    m_bMoveDone = (nStatByte & 0x01) ? true : false;
    bool bCheckSumFail = (nStatByte & 0x02) ? true : false;
    bool bPowerStatus = (nStatByte & 0x08) ? true : false;
    
    if (m_pPort->IsVerbose()) 
    {
        MOOSTrace("[%02X] Status Returned: Move Done: %s CheckSum: %s Power: %s\n",
		nAddress,
                (m_bMoveDone) ? "Yes " : "No  ", 
                (bCheckSumFail) ? "Fail " : "Pass",
                (bPowerStatus) ? "On" : "Off"); 
    }
    if(bCheckSumFail) return false;
 
    return true;
}

// Initialise JRKerr for single-motor usage. 
// Multi-drop networks need to be done manually
bool CMOOSJRKerrMotorDriver::Initialise()
{

    if(m_pPort==NULL)
        return false;

    if(m_pPort->IsVerbose())
    {
        MOOSTrace("iActuation Init() : Sending JRKerr init commands\n");
    }

    if(GroupReset())
        if(SetAddress(m_nAddress))
            if(SetGains())
		return true;

    return false;
    
}

bool CMOOSJRKerrMotorDriver::SetGains()
{
    
    ///////////////////////////////////////////////////////////////////
    //              now send the commands
    ///////////////////////////////////////////////////////////////////
    bool bInitOK = true;

    
    //Set PID gains.   defaults: Kp=5000, Kd=1000, Ki=0, IntLimit=0,
    //OutputLimit=255, CurrentLimit=0, ErrorLimit=10000, ServoRate=1
    //are fine values for all three actuators.
    //Multi-byte values are sent LSB first.
    
    //Do these for every contoller : ReadStatus, Set PID Gains,

    unsigned short Gains[8] = {5000, 1000, 0, 0, 255, 0, 10000, 1};

//    bInitOK = bInitOK && SendCmdAndReadReply(nActuator+1, JRKERR_READ_STATUS, "\x20");
        
    char Buffer[16];
    string sTmp;
    for(int i=0; i<8; i++)
    {
        Buffer[i*2]   = Gains[i]%256; //LSB
        Buffer[i*2+1] = Gains[i]/256; //MSB
    }

        
    bInitOK = bInitOK && SendCmdAndReadReply(m_nAddress, JRKERR_SET_GAINS, Buffer);

//    bInitOK = bInitOK && SendCmdAndReadReply(m_nAddress, JRKERR_STOP_MOTOR, "\x01");

    if (!bInitOK) 
    {
        MOOSTrace("JRKerr (%d) init failed!\n", m_nAddress);
        return false;
    }
    else
    {
        MOOSTrace("JRKerr (%d) init OK\n", m_nAddress);
        return true;
    }
}
bool CMOOSJRKerrMotorDriver::SetAddress(int nAddress)
{
    m_nAddress = nAddress;

    //set address
    char Data[14]="";
    Data[0]=m_nAddress; 
    Data[1]=0xFF;
    return SendCmdAndReadReply(JRKERR_ADDR_INITIAL, JRKERR_SET_ADDRESS, Data);

}

bool CMOOSJRKerrMotorDriver::GroupReset()
{
        //First we send 16 null bytes, wait 'at least 1 ms' and flush the rx buffer
        //on the device
        for (int i=0; i<16; i++) 
        {
            m_pPort->Write("\x00", 1);
        }
        MOOSPause(1);
        
        m_pPort->Flush();

        // next, reset to powerup state... 
        // but only do this for 1 driver, if there's more than 1
        // since it's a group command
        if(!SendCmdAndReadReply(JRKERR_ADDR_GROUP, JRKERR_HARD_RESET, (char*)"\x00"))
        {
            MOOSTrace("JRKerrMotorDriver: Unable to reset group state\n");
            return false;
        }

        return true;
}

bool CMOOSJRKerrMotorDriver::SetPosition(double pos) 
{
    if(m_nMode!=POSITION)
        return false;

    char nCmd[13]="";
    
    //wait until the previous move is complete.
    double dfTimeWaited = 0;
    while(!m_bMoveDone) 
    {
        //this will set the m_bMoveDone flag when it's done moving - NOP just returns a status byte
        SendCmdAndReadReply(m_nAddress, JRKERR_NOP, (char*)"\x00");
        MOOSPause(10);
        dfTimeWaited+=0.01;

        if(  dfTimeWaited>HARDWARE_FAILURE_TO)
        {
            MOOSTrace("JRKerr HW not responding! FAILED MOVE ASSUMED\n");
            return false;
        }
    }
    
    int position = int(pos * (double)m_nFeedbackCountsPerRevolution/360.0);
    int velocity = int(m_dfPositionModeVelocity * m_nFeedbackCountsPerRevolution);
    int acceleration = int(m_dfAcceleration * m_nFeedbackCountsPerRevolution);
    

    //command byte, then 4bytes pos, 4b vel, 4b accel
    //command byte = 10010111: start now, pos mode, load p,v,a
    nCmd[0]=0x97;
    //now for some crazy fun byte-noodling. Send LSB first.
    nCmd[1]=(position & 0x000000FF);
    nCmd[2]=(position & 0x0000FF00)>>8;
    nCmd[3]=(position & 0x00FF0000)>>16;
    nCmd[4]=(position & 0xFF000000)>>24;
    nCmd[5]=(velocity & 0x000000FF);
    nCmd[6]=(velocity & 0x0000FF00)>>8;
    nCmd[7]=(velocity & 0x00FF0000)>>16;
    nCmd[8]=(velocity & 0xFF000000)>>24;
    nCmd[9]=(acceleration & 0x000000FF);
    nCmd[10]=(acceleration & 0x0000FF00)>>8;
    nCmd[11]=(acceleration & 0x00FF0000)>>16;
    nCmd[12]=(acceleration & 0xFF000000)>>24;

    return SendCmdAndReadReply(m_nAddress, JRKERR_LOAD_TRAJECTORY_POS, nCmd);
}

bool CMOOSJRKerrMotorDriver::SetPercentOutput(double dfPercent)
{

    if(m_nMode!=PWM)
         return false;

    int nThrust;
    int nDirection=1; //0=fwd, 1=rev

    MOOSAbsLimit(dfPercent, 100.0);

    nThrust = (int)((dfPercent/100.0) * JRKERR_MAX_PWM + JRKERR_MIN_PWM);
    
    if(nThrust<0) {
        nDirection=0;
        nThrust=-nThrust;
    }
    
    //control byte, then PWM byte
    //control byte = 0b1?001000 = start move now, PWM mode, ? = direction
    char nCmd[2]="";

    nCmd[0] = 0x88 | ((unsigned char)nDirection << 6);
    nCmd[1] = (unsigned char)nThrust;
    
    bool bOk = SendCmdAndReadReply(m_nAddress, JRKERR_LOAD_TRAJECTORY_PWM, nCmd);
//    bOk = bOk && SendCmdAndReadReply(m_nAddress, JRKERR_START_MOTION,"");
    return bOk;
    
}

bool CMOOSJRKerrMotorDriver::SetAcceleration(double acc)
{
    m_dfAcceleration=acc;
    return true;
}

bool CMOOSJRKerrMotorDriver::SetPositionModeVelocity(double vel)
{
    m_dfPositionModeVelocity=vel;
    return true;
}

// We don't need to set anything, just use PWM commands.
bool CMOOSJRKerrMotorDriver::SetPwmMode()
{
    return true;
}

bool CMOOSJRKerrMotorDriver::SendCommand(const std::string& cmd)
{
    return m_pPort->Write((char *)cmd.c_str(), cmd.length());
}

bool CMOOSJRKerrMotorDriver::IsEnabled()
{
    return true;
}

bool CMOOSJRKerrMotorDriver::Enable(bool bSetStatus)
{    
    SetPercentOutput(0);
    return SendCmdAndReadReply(m_nAddress, JRKERR_STOP_MOTOR, (char*)((bSetStatus) ? "\x01" : "\x00"));
}
