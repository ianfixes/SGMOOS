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
// C++ Implementation: MOOSYSIDriver
//
// Description: Implementation of CMOOSYSIDriver class.
//
//
// Author: Martin McBrien <mmcbrien@mit.edu>,(C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include <iostream>
#include <string>

#include <MOOSLIB/MOOSLib.h>
#include <MOOSGenLib/MOOSGenLib.h>
#include "MOOSYSIDriver.h"
#include "SeaGrantGlobalHelper.h"

using namespace std;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSYSIDriver::CMOOSYSIDriver()
{
    m_bVerbose=false;  //set to false to stop all those darn messages
}

CMOOSYSIDriver::~CMOOSYSIDriver()
{

}
 
/////////////////////////////////////////////
///this is where it all happens..

bool CMOOSYSIDriver::Initialise()
{
    return StartReadings();    
}

bool CMOOSYSIDriver::GetData(MOOSVARMAP& Vars)
{
    string sData;
    double dfWhen;

    // Read YSI data
    if(m_pPort->GetLatest(sData,dfWhen))
    {
        list<double> dfData;
        if(Split(dfData, sData, " "))
            return ProcessData(Vars, dfData, dfWhen);
    }

    return false;
}


bool CMOOSYSIDriver::StartReadings()
{
    
    string sLatest;
    double dfWhen;
    for(int nTry=0; nTry<5; nTry++)
    {
        m_pPort->Write("run\r\n", 5);
	MOOSPause(3);
    	if(m_pPort->GetLatest(sLatest, dfWhen))
	    return true;
    }

    return false;
}
