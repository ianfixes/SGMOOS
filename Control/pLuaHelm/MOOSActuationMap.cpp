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
// C++ Implementation: CMOOSActuationMap
//
// Description: 
//
//
// Author: Justin Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "MOOSActuationMap.h"
#include <math.h>
CMOOSActuationMap::CMOOSActuationMap()
{
}


CMOOSActuationMap::~CMOOSActuationMap()
{
}

long CMOOSActuationMap::GetValidDOFs() const
{
    return m_nValidDOFs;
}

STRING_LIST CMOOSActuationMap::Registrations() const
{
    STRING_LIST ret;
    return ret;
}

STRING_LIST CMOOSActuationMap::GetActuatorList() const
{
    return m_ActuatorList;
}

double CMOOSActuationMap::SignedSqrt(double dfVal) const
{
    if(dfVal == 0.0)
        return 0.0;

    double dfSign = (dfVal < 0.0) ? -1.0 : 1.0;
    return dfSign * sqrt(dfSign * dfVal);

}

double CMOOSActuationMap::Force2PropRPM(double dfForce) const
{
    double dfRPM = SignedSqrt(dfForce) * m_dfPropLinearScalingFactor;
    MOOSAbsLimit(dfRPM, m_dfMaxThrust);
    return dfRPM;
}
