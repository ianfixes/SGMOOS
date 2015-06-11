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
// C++ Interface: CMOOSActuationMap
//
// Description: 
//
//
// Author: Justin Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ACTUATIONMAP_H
#define ACTUATIONMAP_H

#include <string>
#include <map>
#include <list>

#include <MOOSLIB/MOOSLib.h>
#include "PathAction.h"

using namespace std;
typedef list<string> STRING_LIST; 

/**
This is the most basic of abstract base classes meant to simply define an interface for vehicle-specific child classes to utilize.

	@author Justin Eskesen <jge@mit.edu>
*/

class CMOOSActuationMap
{
public:
    CMOOSActuationMap();
    ~CMOOSActuationMap();
    virtual bool Engage(const CPathAction& DesiredAction, MOOSVARMAP& VarMap)=0;
    virtual bool Subscribe(MOOSVARMAP& Vars) const;

    // Return a bitmask corresponding to 6 possible DOFs
    long GetValidDOFs() const;
    STRING_LIST GetActuatorList() const;
    virtual bool ReadConfig(CProcessConfigReader& Config) {return true;}

protected:

    double SignedSqrt(double dfVal) const;
    double Force2PropRPM(double dfForce) const;
    long m_nValidDOFs;

    STRING_LIST m_ActuatorList;

    // Maximum thrust command (units unspecified)
    double m_dfMaxThrust;

    double m_dfPropLinearScalingFactor;

};

#endif
