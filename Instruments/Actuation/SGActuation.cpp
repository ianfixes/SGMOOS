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
// SGActuation.cpp: implementation of the CSGActuation class.
//
//////////////////////////////////////////////////////////////////////

#include <MOOSLIB/MOOSLib.h>
#include "SGActuation.h"
#include "ReefExplorerThrusters.h"
#include "OdysseyIVThrusters.h"

#include <iostream>
#include <math.h>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSGActuation::CSGActuation()
{

    m_bSimMode=false;

    //some sensible defaults (missionfile can overwrite this)
    SetAppFreq(5);
    SetCommsFreq(8);

    m_dfLastTimeSet = -1000.000; // Don't run until commanded to....
    m_dfInputTimeOut = 5.0;
    
}

CSGActuation::~CSGActuation()
{
    delete m_Actuators;
}

bool CSGActuation::Iterate()
{
    bool bOk = m_Actuators->Run(m_MOOSVars);
//    bOk &= m_Actuators->GetStatus();

    PublishFreshMOOSVariables();
    return bOk;
}

bool CSGActuation::OnConnectToServer()
{
    string sMyNameIs = m_sAppName;
    string sVarname;
    MOOSToUpper(sMyNameIs);

    // Subscribe to enable command
    sVarname = sMyNameIs + (string)"_ENABLE";
    AddMOOSVariable("Enable", sVarname, (string)"JUNK_" + sVarname, 0.5);

    // Publish status
    sVarname = sMyNameIs + (string)"_STATUS";
    AddMOOSVariable("Status", (string)"JUNK_" + sVarname, sVarname, 0.5);

    // Subscribe to initialize command
    sVarname = sMyNameIs + (string)"_INIT";
    AddMOOSVariable("Init", sVarname, (string)"JUNK_" + sVarname, 0.5);

    RegisterMOOSVariables();
    return true;
}

bool CSGActuation::OnNewMail(MOOSMSG_LIST &NewMail)
{
    UpdateMOOSVariables(NewMail);

    CMOOSVariable & Enable = m_MOOSVars["Enable"];
    if(Enable.IsFresh())
    {
        bool bEnable = MOOSStrCmp(Enable.GetStringVal(), "TRUE");
        m_Actuators->Enable(bEnable);
    }

    CMOOSVariable & Init = m_MOOSVars["Init"];
    if(Init.IsFresh())
    {
        m_Actuators->Initialise();
    }
    return true;

}

bool CSGActuation::OnStartUp()
{
	
    //call base class version first...
    CMOOSInstrument::OnStartUp();

    if(!SetupPort()) return false;
    
    string sTimeOut;
    m_MissionReader.GetConfigurationParam("TIMEOUT",m_dfInputTimeOut);

/*
    if(!IsSimulateMode())
    {
        //try to open 
        if(SetupPort())
        {
            if(Initialize())
                return true;
        }
        return false;
    }
*/

    if(!MakeActuationSet())
    {
        MOOSTrace("Could not set up vehicle-specific actuation\n");
        return false;
    }

    STRING_LIST Subscribe;
    m_Actuators->GetRegistrations(Subscribe);

    //GET DESIRED
    for(STRING_LIST::iterator s = Subscribe.begin();  s != Subscribe.end(); s++)
        AddMOOSVariable(*s, *s, (string)"JUNK_" + *s, 0.5);

    m_Actuators->Enable(false);

    RegisterMOOSVariables();

    return true;

}


bool CSGActuation::MakeActuationSet()
{
    
    string sWhich;
    //if(!m_MissionReader.GetConfigurationParam("VehicleType", sWhich))
    if(!m_MissionReader.GetValue("VehicleType", sWhich))
    {
        MOOSTrace("Actuation:  vehicle type must be specified.\nAborting\n");
        return false;
    }

    if(MOOSStrCmp(sWhich, "ReefExplorer"))
        m_Actuators = new CReefExporerThrusters();
    else if(MOOSStrCmp(sWhich, "OdysseyIV"))
        m_Actuators = new COdysseyIVThrusters();
/*
    else if(MOOSStrCmp(sWhich, "Odyssey2Tailcone"))
        m_Actuators = new COdyssey2Tailcone();
*/
    else
    {
        MOOSTrace("Actuation: Invalid Vehicle Type (%s)\n", sWhich.c_str());
        return false;
    }

    m_Actuators->SetSerial(&m_Port);

    return m_Actuators->Initialise();

}



