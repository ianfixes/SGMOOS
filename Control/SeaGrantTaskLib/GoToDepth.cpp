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
// GoToDepth.cpp: implementation of the CGoToDepth class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "GoToDepth.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGoToDepth::CGoToDepth() :CMaintainDepth()
{
    m_sClassName = "Go To Depth";

    m_dfHeaveForce = 20.0;
    m_dfCapture = 2.0;
}

CGoToDepth::~CGoToDepth()
{

}

bool CGoToDepth::Run(CPathAction &DesiredAction)
{
	
	if(m_DepthDOF.IsValid())
	{
		// POSITIVE depth is NEGATIVE heave
		double dfError = -1.0 * m_DepthDOF.GetError();
		double dfErrorSign = dfError > 0.0 ? 1.0 : -1.0;
	
		DesiredAction.SetOpenLoop(HEAVE, dfErrorSign*m_dfHeaveForce, m_nPriority, m_sName.c_str());
	
		if(fabs(dfError) < m_dfCapture)
			OnComplete();
	}
	
	return true;
}

bool CGoToDepth::SetParam(string sParam, string sVal)
{
	MOOSToUpper(sParam);
	MOOSToUpper(sVal);
	
	if(!CSGMOOSBehaviour::SetParam(sParam,sVal))
	{
		//this is for us...
		if(MOOSStrCmp(sParam,"DEPTH"))
		{
			m_DepthDOF.SetDesired(atof(sVal.c_str()));
		}
		else if(MOOSStrCmp(sParam,"FORCE"))
		{
			m_dfHeaveForce = atof(sVal.c_str());
		}
		else if(MOOSStrCmp(sParam,"CAPTURE"))
		{
			m_dfCapture = atof(sVal.c_str());
		}
		else
			return false;
	}

	return true;

}

