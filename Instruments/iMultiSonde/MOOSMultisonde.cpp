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
// MOOSMultisonde.cpp: implementation of the CMOOSMultisonde class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include "MOOSMultisonde.h"
#include "MOOSMultisondeDriver.h"
#include "MOOSYSIDriver.h" 
#include "MOOSHydrolabDriver.h" 
#include "SeabirdCTDDriver.h"
#include "SeaGrantGlobalHelper.h"

#include <iostream>
#include <math.h>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSMultisonde::CMOOSMultisonde()
{

    m_bSimMode=false;

    //some sensible defaults (missionfile can overwrite this)
    SetAppFreq(5);
    SetCommsFreq(8);

}

CMOOSMultisonde::~CMOOSMultisonde()
{
}

bool CMOOSMultisonde::OnStartUp()
{	
	CMOOSInstrument::OnStartUp();
    if(IsSimulateMode())
    {
        
       
    }
    else
    {
        //try to open 
        if(!SetupPort())
        {
            MOOSTrace("CMOOSMultisonde::OnStartUp - Failed to setup port\n");
            return false;
        }
    }

    MakeDriver();
    if(m_MultisondeDriver == NULL)
       return false;
    
    string sMyNameIs = GetAppName();
    MOOSToUpper(sMyNameIs);

    string sSensorList;
    if(m_MissionReader.GetConfigurationParam("SENSORS",sSensorList))
    {
        STRING_LIST Sensors;
        Split(Sensors, sSensorList, ",");

        STRING_LIST::iterator i;
        for(i=Sensors.begin(); i!=Sensors.end(); i++)
        {
            MOOSToUpper(*i);
            AddMOOSVariable(*i, "", sMyNameIs+(string)"_"+*i, 0.1);
        }
        m_MultisondeDriver->SetSensors(Sensors);
    }
    else
    {
        MOOSTrace("%s ERROR:I don't know what sensors you've got.\n", m_sAppName.c_str());
        return false;
    }

    if(!m_MultisondeDriver->Initialise())
    {
        MOOSTrace("Multisonde Driver failed to initialise.\n");
        return false;
    }

    RegisterMOOSVariables();
    return true;
}

bool CMOOSMultisonde::OnConnectToServer()
{
    RegisterMOOSVariables();
    return true;
    
}

bool CMOOSMultisonde::OnNewMail(MOOSMSG_LIST &NewMail)
{	

    string sMyNameIs = GetAppName();
    MOOSToUpper(sMyNameIs);
    
    return UpdateMOOSVariables(NewMail);
}


bool CMOOSMultisonde::Iterate()
{
    if(m_MultisondeDriver->GetData(m_MOOSVars))
        return PublishFreshMOOSVariables();

    return true;
}


bool CMOOSMultisonde::GetData()
{
    bool bOK = true;
    
    //Read data from the multisonde once per iteration
    return m_MultisondeDriver->GetData(m_MOOSVars);

    return bOK;
}

bool CMOOSMultisonde::MakeDriver()
{

    if(!m_MissionReader.GetConfigurationParam("TYPE",m_sType))
    {
        MOOSTrace("ERROR: driver not specified!  Please specify driver in your mission file\n");
        return false;
    }

    if(m_sType == (string)"YSI")
        m_MultisondeDriver = new CMOOSYSIDriver();
    else if(m_sType == (string)"Hydrolab")
        m_MultisondeDriver = new CMOOSHydrolabDriver();
    else if(m_sType == (string)"Seabird")
        m_MultisondeDriver = new CSeabirdCTDDriver();
    else
    {
        MOOSTrace("ERROR: driver \"" + m_sType + " not implemented, please check for typos or write a driver\n");
        return false;
    }
    
    m_MultisondeDriver->SetSerial(&m_Port);
    return true;
}
