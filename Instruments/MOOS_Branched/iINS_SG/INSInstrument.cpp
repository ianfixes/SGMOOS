///////////////////////////////////////////////////////////////////////////
//
//   MOOS - Mission Oriented Operating Suite 
//  
//   A suit of Applications and Libraries for Mobile Robotics Research 
//   Copyright (C) 2001-2005 Massachusetts Institute of Technology and 
//   Oxford University. 
//	
//   This software was written by Paul Newman and others
//   at MIT 2001-2002 and Oxford University 2003-2005.
//   email: pnewman@robots.ox.ac.uk. 
//	  
//   This file is part of a  MOOS Instrument. 
//		
//   This program is free software; you can redistribute it and/or 
//   modify it under the terms of the GNU General Public License as 
//   published by the Free Software Foundation; either version 2 of the 
//   License, or (at your option) any later version. 
//		  
//   This program is distributed in the hope that it will be useful, 
//   but WITHOUT ANY WARRANTY; without even the implied warranty of 
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
//   General Public License for more details. 
//			
//   You should have received a copy of the GNU General Public License 
//   along with this program; if not, write to the Free Software 
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//   02111-1307, USA. 
//
//////////////////////////    END_GPL    //////////////////////////////////
// INSInstrument.cpp: implementation of the CINSInstrument class.
//
//////////////////////////////////////////////////////////////////////
#include <MOOSLIB/MOOSLib.h>
#include <string>
using namespace std;

//driver files
#include "INSInstrument.h"
#include "XbowDriver.h"
#include "MicroStrainDriver.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CINSInstrument::CINSInstrument()
{
    (void) m_dfMagneticOffset;
}

CINSInstrument::~CINSInstrument()
{
	
}

/**
*Make a driver and default make the Xbow
*/
bool CINSInstrument::MakeDriver()
{

    m_pINSSensor = NULL;
    string sType;

    m_MissionReader.GetConfigurationParam("TYPE",sType);

    if(MOOSStrCmp(sType,"XBOW"))
    {
        m_pINSSensor = new CXbowDriver;
        MOOSTrace("INS using Crossbow Driver\n");
    }
    else if(MOOSStrCmp(sType,"MICROSTRAIN")) 
    {
        m_pINSSensor = new CMicroStrainDriver;
        MOOSTrace("INS using Microstrain Driver\n");
    }
    else
    {
        MOOSTrace("CINSInstrument::MakeDriver(): INS driver: %s NOT RECOGNIZED\n", sType.c_str());
    }
    return m_pINSSensor != NULL;
}

/////////////////////////////////////////////
///this is where it all happens..
bool CINSInstrument::Iterate()
{
    if(GetData())
    {
        PublishData();
    }
    
    return true;
}

////////////////////////////////////////////////////////////
// tell the world
bool CINSInstrument::PublishData()
{
    return PublishFreshMOOSVariables();
    
}


bool CINSInstrument::OnStartUp()
{
    //call base class member first
    CMOOSInstrument::OnStartUp();
    
    //here we make the variables that we are managing
    double dfINSPeriod = 0.2;
  
    
    //INS update @ 2Hz
    AddMOOSVariable("Heading",  "SIM_HEADING",  "INS_HEADING",  dfINSPeriod);
    AddMOOSVariable("Temperature", "",          "INS_TEMPERATURE", dfINSPeriod);

    AddMOOSVariable("Yaw",      "SIM_YAW",      "INS_YAW",      dfINSPeriod);
    AddMOOSVariable("Pitch",    "SIM_PITCH",    "INS_PITCH",    dfINSPeriod);
    AddMOOSVariable("Roll",     "SIM_ROLL",     "INS_ROLL",     dfINSPeriod);

    AddMOOSVariable("RollRate",  "SIM_ROLL_VEL",  "INS_ROLL_VEL",  dfINSPeriod);
    AddMOOSVariable("PitchRate",  "SIM_PITCH_VEL",  "INS_PITCH_VEL",  dfINSPeriod);
    AddMOOSVariable("YawRate",  "SIM_YAW_VEL",  "INS_YAW_VEL",  dfINSPeriod);

    AddMOOSVariable("AccelX",  "SIM_ACCEL_X",  "INS_ACCEL_X",  dfINSPeriod);
    AddMOOSVariable("AccelY",  "SIM_ACCEL_Y",  "INS_ACCEL_Y",  dfINSPeriod);
    AddMOOSVariable("AccelZ",  "SIM_ACCEL_Z",  "INS_ACCEL_Z",  dfINSPeriod);

    //we shall need the diference between true north and magnetic north.
    GetMagneticOffset();
    
    if(IsSimulateMode())
    {
        //not much to do...othe than register for input from
        //simulator ...
        RegisterMOOSVariables();
    }
    else
    {
        //try to open 
        if(!SetupPort())
        {
            return false;
        }
    
		if(MakeDriver())
		{
			m_pINSSensor->SetSerialPort(&m_Port);
			m_pINSSensor->SetMissionFileReader(&m_MissionReader);
			m_pINSSensor->SetOffset(this->m_dfMagneticOffset);
			//try 10 times to initialise sensor
			if(!InitialiseSensorN(10,"INS"))
			{
				return false;
			} 
		}
		else
		{
			MOOSTrace("Failed to make INS Driver\n");
			return false;
		}
    }
    
    
    return true;
}



bool CINSInstrument::OnNewMail(MOOSMSG_LIST &NewMail)
{
    return UpdateMOOSVariables(NewMail);
}




bool CINSInstrument::OnConnectToServer()
{
    if(IsSimulateMode())
    {
        //not much to do...
        return RegisterMOOSVariables();
        
    }
    else
    {
        
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////
// here we initialise the sensor, giving it start up values
bool CINSInstrument::InitialiseSensor()
{    
   return m_pINSSensor->Initialise();
}

bool CINSInstrument::GetData()
{
		   
    if(!IsSimulateMode())
    {
		if(m_pINSSensor->GetData())
		{
						//get the notifications from the INS
			MOOSMSG_LIST Notifications;
			MOOSMSG_LIST::iterator q;
			//get the values from the INS Driver
			m_pINSSensor->GetNotifications(Notifications);

			for(q = Notifications.begin();q!=Notifications.end();q++)
			{
				if(m_Comms.IsConnected())
				{
					m_Comms.Post(*q);
				}
			}
			
			//erase 
			//Notifications.clear();

			return true;
		}
		else
		{
			return false;
		}

	}
	else
	{
		//nothing to do...
		return true;
	}
}


