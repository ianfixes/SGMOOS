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
// ConstantHeading->cpp: implementation of the CHoldingPattern class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "HoldingPattern.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CHoldingPattern::CHoldingPattern() : CFaceXYPoint()
{
    m_sClassName = "HoldingPattern";
}

CHoldingPattern::~CHoldingPattern()
{
}

bool CHoldingPattern::Run(CPathAction &DesiredAction)
{
	CFaceXYPoint::Run(DesiredAction);

	m_RadialDOF.SetCurrent(GetWaypointDistance(),m_YawDOF.GetErrorTime());

        if(GetWaypointDistance() > m_RadialDOF.GetDesired())
	   DesiredAction.SetClosedLoop(SURGE, -1.0*m_RadialDOF.GetError(), m_RadialDOF.GetDesired(), m_nPriority, m_sName.c_str());

        return true;
}

bool CHoldingPattern::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);

    if(!CFaceXYPoint::SetParam(sParam,sVal))
    {
        if(sParam=="RADIUS")
        {
            m_RadialDOF.SetDesired(atof(sVal.c_str()));
        }
        else
        {
            return false;
        }
    }

    return true;

}

