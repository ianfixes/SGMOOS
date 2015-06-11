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
// Description: Defines class CMOOSMultisonde.
//
//
// Author: Justin G Eskesen <jge@mit.edu>, Martin McBrien <mmcbrien@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//


#ifndef MOOSMULTISONDEDRIVER_H
#define MOOSMULTISONDEDRIVER_H

#include <MOOSLIB/MOOSLib.h>
#include <MOOSGenLib/MOOSGenLib.h>
using namespace std;


/**
 * FIXME: needs class description
 */
class CMOOSMultisondeDriver 
{
public:
    CMOOSMultisondeDriver();
    ~CMOOSMultisondeDriver();

    //Initialization methods
    //Values are read (from configuration) & stored by the Multisonde class
    //I'm just the messenger
    void SetSerial(CMOOSSerialPort* port);
    //virtual bool GetPublications(list<string>& VarNames);

    //Get Multisonde Readings 
    virtual bool Initialise()=0;
    virtual bool GetData(MOOSVARMAP& Vars)=0;
    virtual bool StartReadings(){return true;}
    void SetSensors(STRING_LIST Sensors){m_sSensorNames=Sensors;}
protected:
    virtual bool ProcessData(MOOSVARMAP& Vars, list<double>& dfVals, double dfWhen);
//    virtual bool GetNamesString(string& sVarNameString) = 0;
    CMOOSSerialPort* m_pPort;
    STRING_LIST m_sSensorNames;
//    string m_sPrefix;

    virtual bool GetNotifications(MOOSMSG_LIST & List) {return true;}

};

#endif

