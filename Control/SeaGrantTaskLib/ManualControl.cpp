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
// ManualControl.cpp: implementation of the CManualControl class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "ManualControl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CManualControl::CManualControl()
{
    m_sClassName="Manual Control";
    m_bInitialised = false;
}

CManualControl::~CManualControl()
{

}

// This, we'll handle differently than normal
// If our MANUAL_* variables have timed out... then we'll set them to 0
bool CManualControl::RegularMailDelivery(double dfTimeNow)
{

    DOF_MAP::iterator i;
    for(i=m_ManualDOFs.begin(); i!=m_ManualDOFs.end(); i++)
    {
        if(i->second.IsStale(dfTimeNow,GetStartTime()))
            i->second.SetDesired(0.0);
    }

    return true;
}

bool CManualControl::Run(CPathAction &DesiredAction)
{

    if(!m_bInitialised)
    {
        Initialise();
    }
    
    DOF_MAP::iterator i;
    for(i=m_ManualDOFs.begin(); i!=m_ManualDOFs.end(); i++)
    {

        double dfDesired = i->second.GetDesired();
        DesiredAction.SetOpenLoop((WhichDOF)GetDOFNum(i->first), dfDesired, m_nPriority, m_sName.c_str());
    }
    return true;

}

bool CManualControl::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;
    double dfNow = MOOSTime();
    DOF_MAP::iterator i;
    for(i=m_ManualDOFs.begin(); i!=m_ManualDOFs.end(); i++)
    {
        if(PeekMail(NewMail,(string)"MANUAL_"+i->first,Msg))
        {
            if(!Msg.IsSkewed(dfNow))
            {
                i->second.SetCurrent(Msg.m_dfVal,Msg.m_dfTime);  // so that it is "valid"
                i->second.SetDesired(Msg.m_dfVal);
            }
        }
    }

    //always call base class version
    CSGMOOSBehaviour::OnNewMail(NewMail);

    return true;
}

bool CManualControl::GetRegistrations(STRING_LIST &List)
{

    DOF_MAP::iterator i;

    for(i=m_ManualDOFs.begin(); i!=m_ManualDOFs.end(); i++)
        List.push_front((string)"MANUAL_"+i->first);

    //always call base class version
    CSGMOOSBehaviour::GetRegistrations(List);

    return true;
}


bool CManualControl::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);

    if(!CSGMOOSBehaviour::SetParam(sParam,sVal))
    {

        //this is for us...
        if(MOOSStrCmp(sParam,"DOF"))
        {
            MOOSTrace("Adding maunual control of %s\n", sVal.c_str());

            ControlledDOF NewDOF;
            m_ManualDOFs[sVal]=NewDOF;

        }
        else
        {
            //hmmm - it wasn't for us at all: base class didn't understand either
            MOOSTrace("Param \"%s\" not understood!\n",sParam.c_str());
            return false;
        }
    }

    return true;

}

// temporary... until there's a better way of naming dofs.
int CManualControl::GetDOFNum(const string& sDOFName) const
{
    for(int i=0; i<6; i++)
        if(MOOSStrCmp(sDOFName, DOF_Names[i]))
            return i;

    return -1;
}
