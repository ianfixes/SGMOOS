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
// MaintainPipeRelHeading.cpp: implementation of the CMaintainPipeRelHeading class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "MaintainPipeRelHeading.h"
#include "SeaGrantGlobalHelper.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CMaintainPipeRelHeading::CMaintainPipeRelHeading()
{
    CMaintainHeading();
    CMaintainPipeDistance();

    m_sClassName = "Pipe Relative Heading";
}

CMaintainPipeRelHeading::~CMaintainPipeRelHeading()
{

}

//returns false if we haven't received data in a while..bad news!
bool CMaintainPipeRelHeading::RegularMailDelivery(double dfTimeNow)
{
    return CMaintainHeading::RegularMailDelivery(dfTimeNow) &&
           CMaintainPipeDistance::RegularMailDelivery(dfTimeNow);

}

bool CMaintainPipeRelHeading::OnNewMail(MOOSMSG_LIST &NewMail)
{

    return CMaintainHeading::OnNewMail(NewMail) &&
           CMaintainPipeDistance::OnNewMail(NewMail);
    
}

bool CMaintainPipeRelHeading::Run(CPathAction &DesiredAction)
{
    if(m_YawDOF.IsValid())
    {
        double dfSwayError = m_PipeDistanceDOF.GetCurrent() * sin(m_YawDOF.GetError());
    
        DesiredAction.SetClosedLoop(SWAY, dfSwayError, 0.0, m_nPriority, m_sName.c_str());
    
        PublishDesiredHeading();
        return true;
    }
    return false;
}

bool CMaintainPipeRelHeading::GetRegistrations(STRING_LIST &List)
{

    //always call base class version
    CMaintainHeading::GetRegistrations(List);
    CMaintainPipeDistance::GetRegistrations(List);

    return true;
}

bool CMaintainPipeRelHeading::SetParam(string sParam, string sVal)
{
    return CMaintainHeading::SetParam(sParam,sVal);
}
