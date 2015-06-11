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

//////////////////////////    END_GPL    //////////////////////////////////
// GoForward.cpp: implementation of the CGoForward class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "GoForward.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CGoForward::CGoForward()
{

    m_bInitialised = false;
    m_bClosedLoop = false;
    m_sClassName = "Go Forward";
}

CGoForward::~CGoForward()
{

}
//returns false if we haven't received data in a while..bad news!
bool CGoForward::RegularMailDelivery(double dfTimeNow)
{
    if(m_bClosedLoop)
        return !m_SurgeDOF.IsStale(dfTimeNow,GetStartTime());
    else
        return true;

}


bool CGoForward::Run(CPathAction &DesiredAction)
{

    if(!m_bInitialised)
    {
        Initialise();
    }
	
    if(m_SurgeDOF.IsValid() || !m_bClosedLoop)
    {

        double dfDesired = (m_bClosedLoop) ? m_SurgeDOF.GetError() : m_SurgeDOF.GetDesired();

        
        // send yaw error back to pid (closed loop)
        DesiredAction.SetOpenLoop(SURGE, dfDesired, m_nPriority, m_sName.c_str());

    }
    return true;
}

bool CGoForward::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);


/*
    if(m_bTopLevel)
        if(!CMOOSBehaviour::SetParam(sParam,sVal))
            return true;
*/

    //this is for us...
    if(MOOSStrCmp(sParam,"SPEED"))
    {

        //now user specifies heading w.r.t to true north indegrees , the compassm or otehr magnetic device
        //has already made the corrections...
        double dfDesired = atof(sVal.c_str());

        m_SurgeDOF.SetDesired(dfDesired);
    }
    else
    {
        return false;
    }

    return true;

}

bool CGoForward::Initialise()
{
    m_bInitialised = true;

    return true;
}

void CGoForward::GetChartNodeContents(STRING_LIST& contents) const
{
    contents.push_back(MOOSFormat("Forward Speed: %7.2f%%", m_SurgeDOF.GetDesired()));
}

