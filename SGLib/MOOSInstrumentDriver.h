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
// MOOSInstrumentDriver.h: interface for the CMOOSInstrumentDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOOSINSTRUMENTDRIVER_H__1E24B006_99D4_4373_A0F1_4A48984C41E4__INCLUDED_)
#define AFX_MOOSINSTRUMENTDRIVER_H__1E24B006_99D4_4373_A0F1_4A48984C41E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MOOSGenLib/MOOSGenLib.h>//CMOOSSerialPort, XPCTcpSocket
//#include "MOOSCommClient.h" //contains CMOOSMsg, MOOSMSG_LIST
#include "MOOSInstrumentFamily.h"
#include "AverageFilter.h"
#include "MedianFilter.h"

#include <string>

typedef std::list<std::string> STRING_LIST;

#define SOCKET_RECEIVE_TIMEOUT 1.0 //seconds

/**
 * FIXME: needs class description
 */
class CMOOSInstrumentDriver  
{
public:
	void SetInitialised(bool bInit);
	static std::string sMEDIAN;
	static std::string sAVERAGE;
	/**@return the <code>CSensorDataFilter</code> this Driver uses*/
	CSensorDataFilter* GetDataFilter();
	/**@return the <code>XPCTcpSocket</code> this Driver watches over*/
	XPCTcpSocket* GetSocket();
	/**@return the <code>CMOOSSerialPort</code> this Driver watches over*/
	CMOOSSerialPort* GetSerialPort();
	
	CMOOSInstrumentDriver();
	virtual ~CMOOSInstrumentDriver();

	/**A sensor's communication channel is either over a serial port interface or
	via a TCP/IP socket*/
	typedef enum 
	{
		ESOCKET = 0,
		ESERIAL,
		EPNP //plug-n-play
	} 
	EConnectionType;

	/**Test method for determining verbosity of serial port*/
	bool IsVerbose();
	/**Test method for determining iniliaztion state*/
	bool IsInitialised();
	/**Interface to passing a CMOOSMsg to the sensor.  this is necessary to communicate
	with the sensor directly via SENSOR_COMMAND*/
	bool SendMessage(CMOOSMsg Msg);
	/**Method called by CMOOSInstrument to access the publications this sensor
	wants to tell the world about*/
	bool GetNotifications(MOOSMSG_LIST &List);
	/**Method called by CMOOSInstrument to access the MOOSVariables this sensor
	is interested in hearing about*/
	bool GetRegistrations(STRING_LIST &RegList);
	/**Interface to receiving status from the sensor via SENSOR_STATUS.  */
	virtual bool GetStatus();
	/**Implementation of the <code>CMOOSInstrument</code> interface*/
	virtual bool Initialise();
	/**Implementation of the <code>CMOOSInstrument</code> interface*/
	virtual bool GetData();
	

	/*@return the sensor's type*/
	std::string GetSensorType();
	/**@return the enum of the connection type
	*@see EConnectionType
	*/
	int GetConnectionType();
	/**Sets the type of sensor*/
	void SetSensorType(std::string sType);
	/**Sets the connection type supported by sensor*/
	void SetConnectionType(int nType);
	
	/**
	*@return a NULL test against the <code>m_pSocket</code>
	*Sets the <code>XPCTcpSocket</code> this driver will use to communicate with
	*the sensor
	*/
	bool SetSocket(XPCTcpSocket* pSocket);
	/**
	*@return a NULL test against the <code>m_pPort</code>
	*Sets the <code>CMOOSSerialPort</code> this driver will use to communicate with
	*the sensor*/
	bool SetSerialPort(CMOOSSerialPort* pPort);
	/**The ProcessConfigReader allows this Sensor to read in sensor-specific
	configuration from the .moos file*/
	bool SetMissionFileReader(CProcessConfigReader* pMissionFileReader);
	/**This allows for setting a reference is back to the context of the MOOS 
	community in which this Driver participates.  Thus allowing for access to 
	frameworks like the MOOSVariable*/
	bool SetInstrument(CMOOSInstrumentFamily* pInstrument);

protected:
	void MakeFilter();
	/**Can be either AVERAGE or MEDIAN*/
	std::string m_sFilterType;
	/**This version supports an equal width window, i.e. 
	CSensorDataFilter::m_nWindowSize == CSensorDataFilter::m_nDataSize*/
	int m_nFilterWindow;
	/**Whether or not to use the CSensorDataFilter*/
	bool m_bFilter;
	/**Factory method for creating a <code>CSensorDataFilter</code> type:
	either an <code>CAverageFilter</code> or a <code>CMedianFilter</code> presently supported*/
	CSensorDataFilter* CreateFilter();
	/**Filter for incoming data*/
	CSensorDataFilter* m_pSensorFilter;
	/**Convenience method for adding a variable name to the list of variables
	this sensor has subscribed to*/
	bool AddRegistration(std::string &sVarName);
	/**Convenience method for alerting the MOOS community about this variable's change
	in <code>double</code> value*/
	bool AddNotification(const char * sKey,double dfVal);
	/**Convenience method for alerting the MOOS community about this variable's change
	in <code>std::string</code> value*/
	bool AddNotification(const char * sKey,const char* sVal);
	/**Method for RESETing a sensor.*/
	virtual bool Reset();
	/**Method for STARTing a sensor.*/
	virtual bool Start();
	/**Method for STOPing a sensor.*/
	virtual bool Stop();	
	/**extends the SendMessage() interface to provide for more functionality than
	just STOP, CLOSE, and START.*/
	virtual bool OnNewCommand(CMOOSMsg Msg);
	/**list of variable values from sensor*/
	MOOSMSG_LIST m_Notifications;
	/**list of variables that driver needs*/
	STRING_LIST m_Registrations;
	/**The type of this driver, i.e. the manufacturer's physical implementation of the sensor.  
	The manufacturer's name is useful in definining	the type, which is read from
	the configuration block, e.g. <code>Type = XBOW</code> or <code>Type = MICROSTRAIN</code>*/
	std::string m_sType;
	/**Drivers sometimes have to communicate with their sensor via Sockets, thus
	this std::std::string keeps track of the IP to connect to*/
	std::string m_sIPAddress;
	/**We support <code>ESOCKET</code> or <code>ESERIAL</code> connections to sensors*/
	int m_nConnectionType;
	/**State of initialization*/
	bool m_bInitialised;
	/**Verbosity is usually a function of the sensor's serial port, but can be 
	controlled by the Instrument that owns this sensor*/
	bool m_bVerbose;
	/**Pointer to a socket that connects driver to sensor*/
	XPCTcpSocket* m_pSocket;
	/**Reference to the <code>CMOOSSerialPort</code> that driver uses to talk to 
	sensor*/
	CMOOSSerialPort* m_pPort;
	/**Reference to the <code>CProcessConfigReader</code> that driver uses to load
	specific data from the processConfig block about the sensor*/
	CProcessConfigReader* m_pMissionFileReader;
	/**Reference to the <code>CMOOSInstrumentFamily</code> that driver uses to 
	access the state of MOOSVariables it has registered for*/
	CMOOSInstrumentFamily* m_pInstrumentFamily;
        CMOOSVariable * GetMOOSVar(std::string sName);


};

#endif // !defined(AFX_MOOSINSTRUMENTDRIVER_H__1E24B006_99D4_4373_A0F1_4A48984C41E4__INCLUDED_)
