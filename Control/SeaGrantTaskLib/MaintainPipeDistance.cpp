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
// MaintainPipeDistance.cpp: implementation of the CMaintainPipeDistance class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "MaintainPipeDistance.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMaintainPipeDistance::CMaintainPipeDistance()
{
    m_sClassName="Maintain Pipe Distance";

}

CMaintainPipeDistance::~CMaintainPipeDistance()
{

}

bool CMaintainPipeDistance::GetRegistrations(STRING_LIST &List)
{
    List.push_back("PIPE_RANGE");
    //always call base class version
    CSGMOOSBehaviour::GetRegistrations(List);

    return true;
}

bool CMaintainPipeDistance::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;

    double dfNow = MOOSTime();
    if(PeekMail(NewMail,"PIPE_RANGE",Msg))
    {
        if(!Msg.IsSkewed(dfNow))
            m_PipeDistanceDOF.SetCurrent(Msg.m_dfVal, dfNow);
    }
    return CSGMOOSBehaviour::OnNewMail(NewMail);
}

bool CMaintainPipeDistance::Run(CPathAction &DesiredAction)
{

    if(m_PipeDistanceDOF.IsValid())
    {

        if(m_PipeDistanceDOF.IsSet())
            DesiredAction.SetClosedLoop(SURGE, -m_PipeDistanceDOF.GetError(), m_PipeDistanceDOF.GetDesired(), m_nPriority, m_sName.c_str());

    }
    return true;
}

bool CMaintainPipeDistance::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);

    if(!CSGMOOSBehaviour::SetParam(sParam,sVal))
    {
    
        //this is for us...
        if(MOOSStrCmp(sParam,"Distance"))
        {
            m_PipeDistanceDOF.SetDesired(atof(sVal.c_str()));
        }
        else
        {
            return false;
        }
    }
    return true;

}

bool CMaintainPipeDistance::RegularMailDelivery(double dfTimeNow)
{
    return true;
    return !m_PipeDistanceDOF.IsStale(dfTimeNow,GetStartTime());
}
