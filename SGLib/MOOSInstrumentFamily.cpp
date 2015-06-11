/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Rob Damus & others at MIT Sea Grant.
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
// InstrumentFamily.cpp: implementation of the CMOOSInstrumentFamily class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include <MOOSLIB/MOOSLib.h>
#include <MOOSLIB/XPCTcpSocket.h>
#include <MOOSGenLib/MOOSGenLib.h>

#include "MOOSInstrumentFamily.h"
#include "MOOSInstrumentDriver.h"
using namespace std;

#define DRIVER_REGISTRATION 0.0 //notify as often as we AppTick

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSInstrumentFamily::CMOOSInstrumentFamily()
{
    m_pSensor = NULL;
}

CMOOSInstrumentFamily::~CMOOSInstrumentFamily()
{
    delete m_pSensor;
}

bool CMOOSInstrumentFamily::GetData()
{
    if(!IsSimulateMode())
    {
        //get the notifications from the Sensor
        MOOSMSG_LIST Notifications;
        m_pSensor->GetNotifications(Notifications);
	
        MOOSMSG_LIST::iterator q;
        for(q = Notifications.begin(); q != Notifications.end(); q++)
        {
            if(m_Comms.IsConnected())
            {
                m_Comms.Post(*q);
            }
        }
        
        return m_pSensor->GetData();
    }
    else
    {
        //nothing to do...
        return true;
    }
}

bool CMOOSInstrumentFamily::PublishData()
{
    return PublishFreshMOOSVariables(); 
}

bool CMOOSInstrumentFamily::OnStartUp()
{
    if( MakeDriver() )
    {
        if( !IsSimulateMode() )
        {
            //try to open the port, depending on what the driver is 
            if( !SetupConnection() )
            {
                return TraceFailure("CMOOSInstrumentFamily::SetupConnection()");
            }
            
            //try 10 times to initialise sensor
            //majority of sensor init is communications based,
            //thus this will come after setting up the connection
            if( !InitialiseSensorN(10, m_sAppName.c_str()) )
            {
                return TraceFailure("CMOOSInstrumentFamily::InitialiseSensorN()");
            }   
            
        }
        
        //registration happens in OnConnectToServer() during a callback from
        //the DB when the HandShake is succesful between process and DB.
        //however, when we are connected when this method is invoked, which is
        //usually the case, we want to register now...
        if( m_Comms.IsConnected() )
            return RegisterVariables();
    }
    else
    {
        return TraceFailure("CMOOSInstrumentFamily::MakeDriver()");
    }
    
    return true;
}

bool CMOOSInstrumentFamily::MakeDriver()
{
    string sType, sConnection;
    
    //the type of driver 
    m_MissionReader.GetConfigurationParam("TYPE",sType);
    //the type of connection supported by this driver
    m_MissionReader.GetConfigurationParam("CONNECTION",sConnection);
    
    //actually create the driver
    m_pSensor = CreateDriverFor(sType);
    if( m_pSensor == NULL )
    {
        return TraceFailure("CMOOSInstrumentFamily::CreateDriverFor()");
    }
    else
    {
        MOOSTrace("Made %s's Driver of Type: %s\n", m_sAppName.c_str(), sType.c_str());	
    }
    
    //tell the driver what it is
    int nConnection = ParseConnectionType(sConnection);
    m_pSensor->SetConnectionType(nConnection);
    
    //establish the MOOS context
    if( !m_pSensor->SetInstrument( this ) )
    {
        return TraceFailure("CMOOSInstrumentFamily::SetInstrument()");
    }
    
    //establish the processconfig block reader with the Sensor
    if( !m_pSensor->SetMissionFileReader(&m_MissionReader) )
    {
        return TraceFailure("CMOOSInstrumentFamily::SetMissionFileReader()");
    }
    
    return true;
}

bool CMOOSInstrumentFamily::SetupConnection()
{
    int nConnectionType = m_pSensor->GetConnectionType();
    
    switch(nConnectionType){
    case CMOOSInstrumentDriver::ESOCKET:
        if( !SetupSocket() )
        {
            return TraceFailure("CMOOSInstrumentFamily::SetupSocket");
        }
	
        return m_pSensor->SetSocket( m_pSocket );
        
    case CMOOSInstrumentDriver::ESERIAL:
        if( !SetupPort() )
        {
            return TraceFailure("CMOOSInstrumentFamily::SetupPort");
        }
        
        return m_pSensor->SetSerialPort( &m_Port );
        
    case CMOOSInstrumentDriver::EPNP:
        MOOSTrace("Plug-n-Play device detected for %s\n", m_sAppName.c_str());
        return true;
    }
    
    return false;
}

bool CMOOSInstrumentFamily::RegisterDriverVariables()
{
    bool bSuccess = true;
    //on behalf of the driver:
    //retrieve from Sensor
    STRING_LIST DriverRegs;
    if( m_pSensor == NULL )
	return false;
    else
	m_pSensor->GetRegistrations(DriverRegs);
    
    STRING_LIST::iterator p;
    for(p = DriverRegs.begin(); p != DriverRegs.end(); p++)
    {
	bSuccess = true;
	string sVarName = *p;
	//watch over these variables in the Instrument
	bSuccess = AddMOOSVariable(sVarName,sVarName,"",DRIVER_REGISTRATION);
	
	if( m_Port.IsVerbose() )
	{
	    MOOSTrace("On behalf of %s's driver, registering: %s\n", m_sAppName.c_str(), sVarName.c_str());
	    if( bSuccess )
		MOOSTrace("%s added to %s's variable map\n", sVarName.c_str(), m_sAppName.c_str());
	    else
		MOOSTrace("%s already exists in %s's variable map - ignoring\n", sVarName.c_str(), m_sAppName.c_str());
	}
    }
    
    //mind the _COMMAND variable for the Sensor
    m_sDriverCommand = m_sAppName;
    MOOSToUpper(m_sDriverCommand);
    
    m_sDriverCommand += COMMAND_SUFFIX;
    
    bSuccess = AddMOOSVariable(m_sDriverCommand,m_sDriverCommand,"",DRIVER_REGISTRATION);
    if( m_Port.IsVerbose() )
    {
	if( bSuccess )
	    MOOSTrace("%s's driver will use: %s as its COMMAND interface\n", m_sAppName.c_str(), m_sDriverCommand.c_str());
	else
	    MOOSTrace("%s already exists in %s's variable map - ignoring\n", m_sDriverCommand.c_str(), m_sAppName.c_str());
    }
    
    return true;
}

bool CMOOSInstrumentFamily::RegisterVariables()
{
    //now register 
    MOOSTrace("%s Registering...\n", m_sAppName.c_str());
    if( !RegisterDriverVariables() )
        MOOSTrace("Problem registering %s's driver variables\n", m_sAppName.c_str());
    
    return RegisterMOOSVariables();
}

bool CMOOSInstrumentFamily::TraceFailure(const char* sMethod)
{
    MOOSTrace("%s FAILED for: %s\n", sMethod, m_sAppName.c_str());
    return false;
}

bool CMOOSInstrumentFamily::Iterate()
{
    if(GetData())
    {
        PublishData();
    }
    
    return true;
}

bool CMOOSInstrumentFamily::SetupSocket()
{
    STRING_LIST sParams;
    if(!m_MissionReader.GetConfiguration(m_sAppName, sParams))
    {
        return TraceFailure("SetupSocket():GetConfiguration()");
    }
    
    MOOSTrace("XPCSocket:Configure() : ");
    
    STRING_LIST::iterator p;
    for(p=sParams.begin();p!=sParams.end();p++)
    {
        string sLine = *p;
        string sTok = MOOSChomp(sLine,"=");
        string sVal = sLine;
        
        if(MOOSStrCmp(sTok,"PORT"))
        {
            long lPort = atoi(sVal.c_str());
            
            if(lPort <= 0)
            {
                lPort = DEFAULT_SOCKET_PORT;
            }
            
            SetSocketPort(lPort);
        }
        else if(MOOSStrCmp(sTok,"IPADDRESS"))
        {
            if( !sVal.empty() )
                SetIPAddress(sVal);
            else
                SetIPAddress(DEFAULT_IP_ADDRESS);
        }
    }
    
    
    m_pSocket = new XPCTcpSocket(GetSocketPort());
    
    MOOSTrace("Opening Sensor connection to %s:%d\n",GetIPAddress().c_str(), GetSocketPort());
    try
    {
        m_pSocket->vConnect(GetIPAddress().c_str());
        MOOSTrace("Sensor Socket connect Success %s:%d\n", GetIPAddress().c_str(), GetSocketPort());
    }
    catch(XPCException e)
    {
        (void) e;
        delete m_pSocket;
        
        return TraceFailure("CMOOSInstrumentFamily::SetupSocket()");
    }
    
    return true;
}

void CMOOSInstrumentFamily::SetIPAddress(std::string sIP)
{
    m_sIPAddress = sIP;
}

string CMOOSInstrumentFamily::GetIPAddress()
{
    return m_sIPAddress;
}

void CMOOSInstrumentFamily::SetSocketPort(long lPort)
{
    m_lSocketPort = lPort;
}

long CMOOSInstrumentFamily::GetSocketPort()
{
    return m_lSocketPort;
}


CMOOSInstrumentDriver* CMOOSInstrumentFamily::CreateDriverFor(std::string sType)
{
    return NULL;
}

int CMOOSInstrumentFamily::ParseConnectionType(std::string &sConnection)
{
    if( sConnection == "SERIAL" )
        return CMOOSInstrumentDriver::ESERIAL;
    else if ( sConnection == "SOCKET" )
        return CMOOSInstrumentDriver::ESOCKET;
    else if ( sConnection == "PNP" )
        return CMOOSInstrumentDriver::EPNP;
    else
        return CMOOSInstrumentDriver::ESERIAL;
}

bool CMOOSInstrumentFamily::OnConnectToServer()
{
    //the Sensor is made, thus registration is in order
    //this occurs when a connection to the DB is not initially available
    //but becomes so at a later point in time.  therefore, OnStartUp() has already
    //been called and the Sensor should be available via MakeDriver()
    if( m_pSensor != NULL )
        return RegisterVariables();
    
    return true;
}

bool CMOOSInstrumentFamily::InitialiseSensor()
{    
    if( m_pSensor->Initialise() )
        m_pSensor->SetInitialised( true );
    
    return m_pSensor->IsInitialised();
}


bool CMOOSInstrumentFamily::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;
    
    //only forward messages to the driver that conform to the
    //m_sAppName_COMMAND format
    if(m_Comms.PeekMail(NewMail, GetDriverCommand().c_str() , Msg))
    {
        if( !Msg.IsSkewed( MOOSTime() ) )
            m_pSensor->SendMessage(Msg);
    }
    
    //handle the updating of all messages
    return UpdateMOOSVariables(NewMail);
}

std::string CMOOSInstrumentFamily::GetDriverCommand()
{
    return m_sDriverCommand;
}

