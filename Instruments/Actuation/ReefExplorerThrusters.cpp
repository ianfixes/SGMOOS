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
#include "ReefExplorerThrusters.h"

CReefExporerThrusters::CReefExporerThrusters()
{

    m_sName = "Reef Explorer";
    pair<string, CMOOSJRKerrMotorDriver> NewJRKerr;
    NewJRKerr.second.SetMode(PWM);

    NewJRKerr.first="PORT_THRUST";
    m_Motors.push_back(NewJRKerr);

    NewJRKerr.first="BOW_XBODY";
    m_Motors.push_back(NewJRKerr);

    NewJRKerr.first="WINCH";
    m_Motors.push_back(NewJRKerr);

    NewJRKerr.first="STARBOARD_THRUST";
    m_Motors.push_back(NewJRKerr);

    NewJRKerr.first="STERN_XBODY";
    m_Motors.push_back(NewJRKerr);

}

CReefExporerThrusters::~CReefExporerThrusters()
{
    m_Motors.clear();
}

bool CReefExporerThrusters::Initialise()
{
    JRKERR_LIST::iterator i;
   
    bool bIsInititalized=false;

    for(int i=0;i<10;i++)
        if(m_Motors.begin()->second.GroupReset())
            bIsInititalized = true;

    if(!bIsInititalized)
    {
        MOOSTrace("iActuationSG: Group initialisation failed\n");
        return false;
    }

    // first loop through & set all their addresses
    int n=0;
    for(i=m_Motors.begin(); i!=m_Motors.end(); i++)
   	if(!i->second.SetAddress(++n))
        {
            MOOSTrace("iActuationSG: could not set address (%d)\n", n);
            return false;
        }

	// next, initialize gains, etc
    for(i=m_Motors.begin(); i!=m_Motors.end(); i++)
        if(!i->second.SetGains())
        {
            MOOSTrace("iActuationSG: Failed to set gains\n");
            return false;
        }

//    return Enable(true);
    return true;
}

bool CReefExporerThrusters::Enable(bool bSetStatus)
{
    bool bOK = true;
    JRKERR_LIST::iterator i;
    for(i=m_Motors.begin(); i!=m_Motors.end(); i++)
        bOK &= i->second.Enable(bSetStatus);

    return bOK;
}

void CReefExporerThrusters::GetRegistrations(STRING_LIST& reg)
{
    JRKERR_LIST::iterator i;
    for(i=m_Motors.begin(); i!=m_Motors.end(); i++)
        reg.push_back(MOOSFormat("DESIRED_%s", (i->first).c_str()));

}

void CReefExporerThrusters::GetPublications(STRING_LIST& pub)
{
}

void CReefExporerThrusters::SetSerial(CMOOSSerialPort* port)
{
    JRKERR_LIST::iterator i;
    for(i=m_Motors.begin(); i!=m_Motors.end(); i++)
        (i->second).SetSerial(port);
}

bool CReefExporerThrusters::Run(MOOSVARMAP& VarMap)
{
    double dfNow = MOOSTime();
    JRKERR_LIST::iterator i;
    for(i=m_Motors.begin(); i!=m_Motors.end(); i++)
    {
        string sVar=MOOSFormat("DESIRED_%s", (i->first).c_str());
        if(VarMap[sVar].GetAge(dfNow) > m_dfInputTimeOut)
        {
            //return Stop();
        }
        else
        {
            (i->second).SetPercentOutput(VarMap[sVar].GetDoubleVal());
        }
    }
    return true;
}

bool CReefExporerThrusters::GetStatus(string& sDescription)
{
    sDescription = "Not Implemented (yet)";
    return true;
}

bool CReefExporerThrusters::OnTimeOut()
{    
    bool bOK = true;
    JRKERR_LIST::iterator i;

    for(i=m_Motors.begin(); i!=m_Motors.end(); i++)
    {
        // Set winch to default, 
        bOK &= (i==m_Motors.begin()) ?
            (i->second).SetPercentOutput(m_dfWinchDefault):
            (i->second).SetPercentOutput(0.0);
    }
    return bOK;
}

