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
// C++ Interface: MOOSMultisondeDriver
//
// Description: Implements class CMOOSMultisonde.
//
//
// Author: Justin G Eskesen <jge@mit.edu>, Martin McBrien <mmcbrien@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "MOOSMultisondeDriver.h"

using namespace std;

CMOOSMultisondeDriver::CMOOSMultisondeDriver()
{
	std::string m_sDriverName = "MultisondeBaseClass_DoNotUse";
}

CMOOSMultisondeDriver::~CMOOSMultisondeDriver()
{

}

void CMOOSMultisondeDriver::SetSerial(CMOOSSerialPort* port)
{
    m_pPort = port;
}

bool CMOOSMultisondeDriver::ProcessData(MOOSVARMAP& Vars, list<double>& dfVals, double dfWhen)
{
    if(m_sSensorNames.size() != dfVals.size())
    {
        MOOSTrace("Config error: readings & labels are different lengths\n");
        return false;
    }

    STRING_LIST::iterator i;
    list<double>::iterator j = dfVals.begin();
    for(i=m_sSensorNames.begin(); i!=m_sSensorNames.end(); i++, j++)
    {
        if(Vars.find(*i) != Vars.end())
        {
            if(Vars[*i].GetTime() < dfWhen)
                Vars[*i].Set(*j, dfWhen);
        }
//        j++;
    }

    return true;
}

/*
// simple, but overridable.
bool CMOOSMultisondeDriver::GetPublications(std::list<std::string>& VarNames)
{
    return true;
}
*/

