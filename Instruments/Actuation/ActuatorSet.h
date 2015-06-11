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
// C++ Interface: ActuatorSet
//
// Description: 
//
//
// Author: Justin G Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ActuatorSet_H
#define ActuatorSet_H

/**
Class to define the iterface of between MOOS instrument
	@author Justin G Eskesen <jge@mit.edu>
*/

#include <MOOSGenLib/MOOSGenLib.h>
#include <MOOSLIB/MOOSLib.h>
#include "MOOSMotorDriver.h"
using namespace std;

/**
 * FIXME: needs class description
 */
class CActuatorSet
{
public:
    CActuatorSet();
    virtual ~CActuatorSet();

    //Register/Subscribe nothing by default
    virtual void GetRegistrations(STRING_LIST& reg){}
    virtual void GetPublications(STRING_LIST& pub){}

    virtual void SetSerial(CMOOSSerialPort* port)=0;
    virtual bool Initialise(){return true;}


    virtual bool Run(MOOSVARMAP& VarMap) = 0;

    virtual bool GetStatus(string& sDescription);

    virtual bool Enable(bool bSetStatus)=0;

protected:

    string m_sName;
//    MOTOR_MAP m_Motors;  // Most, but not all, derived classes will use this

    double m_dfInputTimeOut;
};

#endif

