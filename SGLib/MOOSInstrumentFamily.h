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
// InstrumentFamily.h: interface for the CInstrumentFamily class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INSTRUMENTFAMILY_H__1185DC4F_9338_4063_B335_27EE1D353FDC__INCLUDED_)
#define AFX_INSTRUMENTFAMILY_H__1185DC4F_9338_4063_B335_27EE1D353FDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <MOOSLIB/MOOSLib.h>
#include "SensorDataFilter.h"
#define DEFAULT_SOCKET_PORT 8000
#define DEFAULT_IP_ADDRESS "localhost" 
#define COMMAND_SUFFIX "_COMMAND"

class CMOOSInstrumentDriver;

/**This abstract base class will be extended to create the notion of a family
of instruments that all perform a particular function.  For instance, you might
have a single physical GPS Instrument on your robot, and want to have the ability
to switch this unit at your leisure - i.e. experiment with different manufacturers's
models of GPS.
Thus, you extend this class and also create a CMOOSInstrumentDriver that represents
the implementation of the GPS manufacturer you have chosen.  Thus, flesh out the
CreateDriverFor(string) pure virtual to return this GPSTypeDriver class and you
now have a GPSInstrument that uses the GPSTypeDriver for communications with the 
GPS, and the majority of the work simply involves correctly using the CMOOSInstrumentDriver
methods to notify the MOOS about the GPS data*/
class CMOOSInstrumentFamily : public CMOOSInstrument
{
public:	
	
	CMOOSInstrumentFamily();
	virtual ~CMOOSInstrumentFamily();
	
	/**
	@return The m_sAppName_COMMAND formatted name that the Driver responds to
	*in OnNewMail()
	*/
	std::string GetDriverCommand();

	/**Overriding base class implementation to provide framework for 
	MOOS variable registration*/
	bool OnConnectToServer();
	/**@return the port this Instrument connects to its Sensor on*/
	long GetSocketPort();
	/**sets the port this Instrument connects to its Sensor on*/
	void SetSocketPort(long lPort);
	
	/**@return the IP address of the sensor*/
	std::string GetIPAddress();
	/**Sets the IP address of the sensor*/
	void SetIPAddress(std::string sIP);
protected:
	/*The m_sAppName_COMMAND formatted name that the Driver responds to
	*in OnNewMail()*/
	std::string m_sDriverCommand;
	/**Implementations are responsible for providing a factory-like
	way of generating the appropriate Driver based on the input string.
	e.g. sType = "XBOW" - then a new XBowDriver is returned.
	*/
	virtual CMOOSInstrumentDriver* CreateDriverFor(std::string sType) = 0;
	/**Responsible for enforcing the m_sAppName_COMMAND standard
	of passing messages to the Driver - ALWAYS call from base class
	if you override this method*/
	virtual bool OnNewMail(MOOSMSG_LIST &NewMail);
	/**Overriding base class implementation to provide framework for 
	MOOS variable initialization- ALWAYS call from base class
	if you override this method*/
	virtual bool OnStartUp();
	
	/**Allows actions to take that ensure sensor is started correctly*/
	bool InitialiseSensor();
	/**Method for updating local MOOSVariables and handling Driver Notifications*/
	bool PublishData();
	/**Returns the <code>CMOOSInstrumentDriver::EConnectionType</code> based on 
	input string: "SERIAL" or "SOCKET"*/
	int ParseConnectionType(std::string &sConnection);
	/**Sets up the communications channel to the Sensor based on SensorConnectionType.
	Creates either a <code>CMOOSSerialPort</code> or <code>XPcTcpSocket</code>*/
	bool SetupConnection();
	/**Responsible for reading .moos file for Sensor specific configuration,
	then calls <code>CreateDriverFor(string)</code> to make the Driver*/
	bool MakeDriver();
	/**Overriding base class implementation to provide framework for GetData()*/
	bool Iterate();
	/**Entry point to polling Sensor or receiving data automatically*/
	bool GetData();
	
	/**the port for Socket comms to the Sensor*/
	long m_lSocketPort;
	/**The Socket attached to the m_lSocketPort*/
	XPCTcpSocket* m_pSocket;
	/**the IP address of the Sensor if it is a 
	<code>CMOOSInstrumentDriver::ESOCKET</code> type*/
	std::string m_sIPAddress;
	/**analagous to SetupPort - only for a Socket connection to the sensor*/
	bool SetupSocket();
	/**convenience method for tracing a method failure*/
	bool TraceFailure(const char* sMethod);
	/**handles registering MOOSVariables and the variables for the Driver 
	(i.e. Sensor*/
	bool RegisterVariables();
	/**Iterates over the notifications a Sensor is interested in hearing about*/
	bool RegisterDriverVariables();
	/**The concrete implementation of a Sensor.  This object is attached to the physical
	device and is responsible for communicating with the device, extracting data
	from the device, and managing the lifecycle of the device. */
	CMOOSInstrumentDriver* m_pSensor;
	friend class CMOOSInstrumentDriver;
};

#endif // !defined(AFX_INSTRUMENTFAMILY_H__1185DC4F_9338_4063_B335_27EE1D353FDC__INCLUDED_)
