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
// MOOSInstrumentDriver.cpp: implementation of the CMOOSInstrumentDriver class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include <MOOSLIB/MOOSLib.h>
#include <MOOSGenLib/MOOSGenLib.h>
#include "MOOSInstrumentDriver.h"
using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
string CMOOSInstrumentDriver::sMEDIAN = "AVERAGE";
string CMOOSInstrumentDriver::sAVERAGE = "MEDIAN";

CMOOSInstrumentDriver::CMOOSInstrumentDriver()
{
    m_bInitialised	= false;
    m_bVerbose		= false;
    
    //filter information
    m_bFilter		= false;
    m_nFilterWindow	= 5;
    m_sFilterType	= sAVERAGE;
    m_pSensorFilter = NULL;
    //default is serial port connection
    SetConnectionType(CMOOSInstrumentDriver::ESERIAL);
}

CMOOSInstrumentDriver::~CMOOSInstrumentDriver()
{
    delete m_pSensorFilter;
}


bool CMOOSInstrumentDriver::SetSocket(XPCTcpSocket *pSocket)
{
    m_pSocket = pSocket;
    return m_pSocket != NULL;   
}

bool CMOOSInstrumentDriver::SetSerialPort(CMOOSSerialPort *pPort)
{
    m_pPort = pPort;
    //if port is verbose then so are we!
    m_bVerbose = m_pPort->IsVerbose();
    
    return m_pPort != NULL;   
}

bool CMOOSInstrumentDriver::SetMissionFileReader(CProcessConfigReader* pMissionFileReader)
{
    m_pMissionFileReader = pMissionFileReader;
    
    return m_pMissionFileReader != NULL;
}

bool CMOOSInstrumentDriver::SetInstrument(CMOOSInstrumentFamily* pInstrument)
{
    m_pInstrumentFamily = pInstrument;
    
    return m_pInstrumentFamily != NULL;
}

void CMOOSInstrumentDriver::SetSensorType(std::string sType)
{
    m_sType = sType;
}

void CMOOSInstrumentDriver::SetConnectionType(int nType)
{
    m_nConnectionType = nType;
}

string CMOOSInstrumentDriver::GetSensorType()
{
    return m_sType;
}

int CMOOSInstrumentDriver::GetConnectionType()
{
    return m_nConnectionType;
}

bool CMOOSInstrumentDriver::GetNotifications(MOOSMSG_LIST &List)
{
    List.splice(List.begin(), m_Notifications);
    
    return true;
}

bool CMOOSInstrumentDriver::SendMessage(CMOOSMsg Msg)
{
    string sCmd = Msg.m_sVal;
    
    if( MOOSStrCmp(sCmd, "START") )
    {
        int nTries = 0;
        int nMaxTries = 5;
        while( !IsInitialised() )
        {
            MOOSTrace("Attempting to Initialise() CMOOSInstrumentDriver %d times\n", nTries);
            Initialise();
            if( nTries++ > nMaxTries )
                return false;
        }
	
        Start();
    }
    else if( MOOSStrCmp(sCmd, "STOP") )
    {
        Stop();
    }
    else if( MOOSStrCmp(sCmd, "RESET") )
    {
        Reset();
    }
    else if( MOOSStrCmp(sCmd, "INIT") )
    {
        Initialise();
    }
    else
    {
        OnNewCommand(Msg);
    }
    
    return true;
}

bool CMOOSInstrumentDriver::Stop()
{
    MOOSTrace("Warning, base class CMOOSInstrumentDriver::Stop() called\n");
    return true;
}

bool CMOOSInstrumentDriver::Start()
{
    MOOSTrace("Warning, base class CMOOSInstrumentDriver::Start() called\n");
    return true;
}

bool CMOOSInstrumentDriver::Reset()
{
    MOOSTrace("Warning, base class CMOOSInstrumentDriver::Reset() called\n");
    //ready for re-init
    m_bInitialised = false;
    return true;
}

bool CMOOSInstrumentDriver::GetStatus()
{
    MOOSTrace("Warning, base class CMOOSInstrumentDriver::GetStatus() called\n");
    return true;
}

bool CMOOSInstrumentDriver::Initialise()
{
    //should we filter?
    m_pMissionFileReader->GetConfigurationParam("USEFILTER", m_bFilter);
    if( m_bFilter )
    {
        MOOSTrace("%s using Filter?: = %d\n", m_pInstrumentFamily->GetAppName().c_str(), m_bFilter);
        
        m_pMissionFileReader->GetConfigurationParam("FILTERTYPE", m_sFilterType);
        MOOSTrace("%s FilterType = %s\n", m_pInstrumentFamily->GetAppName().c_str(), m_sFilterType.c_str());
        
        //filter window size?
        double dfWindowSize = -1;
        m_pMissionFileReader->GetConfigurationParam("FILTERWINDOW", dfWindowSize);
        if( dfWindowSize > 0 )
            m_nFilterWindow = (int)dfWindowSize;
        MOOSTrace("%s Filtering Window size = %d\n", m_pInstrumentFamily->GetAppName().c_str(),m_nFilterWindow);
        
        //create the filter
	
        MakeFilter();
    }
    
    return true;
}

bool CMOOSInstrumentDriver::GetData()
{
    MOOSTrace("Warning, base class CMOOSInstrumentDriver::GetData() called\n");
    return true;
}

bool CMOOSInstrumentDriver::GetRegistrations(STRING_LIST &RegList)
{
    
    RegList.insert(RegList.begin(),m_Registrations.begin(),m_Registrations.end());
    
    return true;		
}

bool CMOOSInstrumentDriver::OnNewCommand(CMOOSMsg Msg)
{
    MOOSTrace("Warning, base class CMOOSInstrumentDriver::OnNewCommand() called\n");
    return true;
}

bool CMOOSInstrumentDriver::IsInitialised()
{
    return this->m_bInitialised;
}

bool CMOOSInstrumentDriver::IsVerbose()
{
    return this->m_bVerbose;
}

bool CMOOSInstrumentDriver::AddNotification(const char *sKey, const char *sVal)
{
    CMOOSMsg Msg(MOOS_NOTIFY,sKey,sVal,MOOSTime());
    m_Notifications.push_back(Msg);
    return true;
}

bool CMOOSInstrumentDriver::AddNotification(const char *sKey, double dfVal)
{
    CMOOSMsg Msg(MOOS_NOTIFY,sKey,dfVal,MOOSTime());
    m_Notifications.push_back(Msg);
    return true;
}

bool CMOOSInstrumentDriver::AddRegistration(std::string &sVarName)
{
    m_Registrations.push_back( sVarName );
    return true;
}

CMOOSSerialPort * CMOOSInstrumentDriver::GetSerialPort()
{
    return this->m_pPort;
}

XPCTcpSocket* CMOOSInstrumentDriver::GetSocket()
{
    return this->m_pSocket;
}

/**Because a CSensorDataFilter is available to all drivers, the factory
   method that instantiates them is at the top of the class hierarchy*/
CSensorDataFilter* CMOOSInstrumentDriver::CreateFilter()
{
    if( m_sFilterType == sAVERAGE )
        return new CAverageFilter(m_nFilterWindow, m_nFilterWindow);
    else if ( m_sFilterType == sMEDIAN )
        return new CMedianFilter(m_nFilterWindow, m_nFilterWindow);
    else
        return NULL;
}

CSensorDataFilter* CMOOSInstrumentDriver::GetDataFilter()
{
    return this->m_pSensorFilter;
}

void CMOOSInstrumentDriver::MakeFilter()
{
    m_pSensorFilter = CreateFilter();
}

void CMOOSInstrumentDriver::SetInitialised(bool bInit)
{
    m_bInitialised = bInit;
}
CMOOSVariable * CMOOSInstrumentDriver::GetMOOSVar(std::string sName)
{
    return m_pInstrumentFamily->GetMOOSVar(sName);
}
