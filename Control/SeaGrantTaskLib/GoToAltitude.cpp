///////////////////////////////////////////////////////////////////////////
//
//   MOOS - Mission Oriented Operating Suite 
//  
//   A suit of Applications and Libraries for Mobile Robotics Research 
//   Copyright (C) 2001-2005 Massachusetts Institute of Technology and 
//   Oxford University. 
//	
//   This software was written by Paul Newman and others
//   at MIT 2001-2002 and Oxford University 2003-2005.
//   email: pnewman@robots.ox.ac.uk. 
//	  
//   This file is part of a  MOOS Basic (Common) Application. 
//		
//   This program is free software; you can redistribute it and/or 
//   modify it under the terms of the GNU General Public License as 
//   published by the Free Software Foundation; either version 2 of the 
//   License, or (at your option) any later version. 
//		  
//   This program is distributed in the hope that it will be useful, 
//   but WITHOUT ANY WARRANTY; without even the implied warranty of 
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
//   General Public License for more details. 
//			
//   You should have received a copy of the GNU General Public License 
//   along with this program; if not, write to the Free Software 
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//   02111-1307, USA. 
//
//////////////////////////    END_GPL    //////////////////////////////////
// GoToAltitude.cpp: implementation of the CGoToAltitude class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "GoToAltitude.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////

CGoToAltitude::CGoToAltitude() :CMaintainAltitude()
{
    m_sClassName = "Go To Altitude";
    m_dfHeaveForce = 20.0;
    m_dfCapture = 2.0;
}

CGoToAltitude::~CGoToAltitude()
{

}

bool CGoToAltitude::Run(CPathAction &DesiredAction)
{
	
	if(m_AltitudeDOF.IsValid())
	{
		// POSITIVE depth is NEGATIVE heave
		double dfError = m_AltitudeDOF.GetError();
		double dfErrorSign = dfError > 0.0 ? 1.0 : -1.0;
	
		DesiredAction.SetOpenLoop(HEAVE, dfErrorSign*m_dfHeaveForce, m_nPriority, m_sName.c_str());
	
		if(fabs(dfError) < m_dfCapture)
			OnComplete();
	}
	
	return true;
}

bool CGoToAltitude::SetParam(string sParam, string sVal)
{
	MOOSToUpper(sParam);
	MOOSToUpper(sVal);
	
	if(!CSGMOOSBehaviour::SetParam(sParam,sVal))
	{
		//this is for us...
		if(MOOSStrCmp(sParam,"ALTITUDE"))
		{
			m_AltitudeDOF.SetDesired(atof(sVal.c_str()));
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

