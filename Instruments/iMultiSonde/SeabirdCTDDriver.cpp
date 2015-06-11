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
#include <iostream>
#include <string.h>

#include <MOOSLIB/MOOSLib.h>
#include <MOOSGenLib/MOOSGenLib.h>
#include "SeabirdCTDDriver.h"
#include "SeaGrantGlobalHelper.h"

using namespace std;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSeabirdCTDDriver::CSeabirdCTDDriver()
{
	m_sSensorNames.push_back("Depth");
	m_sSensorNames.push_back("Cond");
	m_sSensorNames.push_back("Press");
	m_sSensorNames.push_back("Sal");
	m_sSensorNames.push_back("SoundSpeed");
}

CSeabirdCTDDriver::~CSeabirdCTDDriver()
{

}
 
/////////////////////////////////////////////
///this is where it all happens..

bool CSeabirdCTDDriver::Initialise()
{
	if(!m_pPort->IsStreaming())
	{

	}
	else
	{
	}
	return true;
}

bool CSeabirdCTDDriver::GetData(MOOSVARMAP& Vars)
{
    string sData;
    double dfWhen;

	char sPollCmd[10] = "TS\r";
	m_pPort->Write(sPollCmd, strlen(sPollCmd));

	if(m_pPort->IsStreaming())
	{			
		if(!m_pPort->GetLatest(sData,dfWhen))
			return false;
	}
	else
	{
		if(!m_pPort->GetTelegram(sData, 3.0, &dfWhen))
		{
			return false;
		}
		//ToDo: Strip off previous Prompt?

	}
	return ParseSeabirdString(sData, Vars, dfWhen);
}


bool CSeabirdCTDDriver::ParseSeabirdString(string sReply, MOOSVARMAP& Vars, double dfWhen)
{
    list<double> dfData;
    if(Split(dfData, sReply, ","))
        return ProcessData(Vars, dfData, dfWhen);
    else
        return false;
}
