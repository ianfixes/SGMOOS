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
//////////////////////////    END_GPL    //////////////////////////////////
// OdysseyIVThrusters.cpp: implementation of the COdysseyIVThrusters class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <MOOSLIB/MOOSLib.h>
#include "OdysseyIVThrusters.h"

#include <iostream>
#include <math.h>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COdysseyIVThrusters::COdysseyIVThrusters()
{

    m_ThrusterNames.push_back("DESIRED_STARBOARD_VECTORED");
    m_ThrusterNames.push_back("DESIRED_BOW_XBODY");
    m_ThrusterNames.push_back("DESIRED_PORT_VECTORED");
    m_ThrusterNames.push_back("DESIRED_STERN_XBODY");

    m_dfScale = ((4096.0-2.0) / (1600.0 * 2.0));
    m_dfShift = (4096.0/2.0 - 1.0);
}

COdysseyIVThrusters::~COdysseyIVThrusters()
{
    m_ThrusterNames.clear();
}

bool COdysseyIVThrusters::Run(MOOSVARMAP& VarMap)
{
    double dfNow = MOOSTime();
    string sCommand = "V";

    STRING_LIST::iterator i;
    for(i=m_ThrusterNames.begin(); i!=m_ThrusterNames.end(); i++)
    {
        if(VarMap[*i].GetAge(dfNow) > m_dfInputTimeOut)
        {
            return Stop();
        }

        double dfThrustPercent=VarMap[*i].GetDoubleVal();

        //scale RPMs to encoder values
        double dfEncodedThrust = dfThrustPercent * m_dfScale + m_dfShift;
        
        // Round to lower speeds (which is a higher value if in reverse)
        dfEncodedThrust = floor(dfEncodedThrust+0.5) ;

        sCommand += MOOSFormat(" %d", (int)dfEncodedThrust); 
    }

    return DoIO(sCommand);
}


void COdysseyIVThrusters::GetRegistrations(STRING_LIST& reg)
{
    reg.insert(reg.end(), m_ThrusterNames.begin(), m_ThrusterNames.end());
}


bool COdysseyIVThrusters::Enable(bool bEnable)
{
    string sInit = "E ";
    sInit += bEnable ? "1" : "0";

    
    return DoIO(sInit);
}

bool COdysseyIVThrusters::DoIO(std::string sCommand)
{
    // all commands begin w/ '#'... if it isn't there, put it there!
    if(sCommand[0]!='#')
            sCommand.insert(0, "#");
        
        
    string sConfirmCommand = sCommand;
    sCommand.append("\r\n");
    string sReply;
    
    // all replies begin w/ '$'
    sConfirmCommand[0] = '$';
    
    m_pPort->Write((char*)sCommand.c_str(), sCommand.size());
    
    if(!m_pPort->GetTelegram(sReply, 1.0)){
        MOOSTrace("iThrusters\tWarning: Thrusters Not Responding!!!\n");
        return false;
    }

    if(sReply != sConfirmCommand){
        MOOSTrace("iThrusters Warning: Invalid Response: (" + sReply +")\n");
        return false;
    }
    
    return true;
}


bool COdysseyIVThrusters::Stop()
{
    //MOOSTrace("Odyssey IV Thruster input timeout\n");
    return DoIO(MOOSFormat("V %d %d %d %d", (int)m_dfShift,(int)m_dfShift,(int)m_dfShift,(int)m_dfShift));
}

void COdysseyIVThrusters::SetSerial(CMOOSSerialPort* port)
{
    m_pPort = port;
}



