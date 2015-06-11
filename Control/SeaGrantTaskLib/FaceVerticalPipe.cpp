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
// FaceVerticalPipeTask.cpp: implementation of the CFaceVerticalPipe class.
//
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <iostream>
using namespace std;
#include <MOOSGenLib/MOOSGenLib.h>

#include "FaceVerticalPipe.h"
#include "SeaGrantGlobalHelper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CFaceVerticalPipe::CFaceVerticalPipe():CSGMOOSBehaviour()
{

    m_bInitialised = false;
    m_bNewBearing = false;
    m_sClassName = "Face Vertical Pipe";
    m_dfStartTime = 5.0;

}

CFaceVerticalPipe::~CFaceVerticalPipe()
{

}

//returns false if we haven't received data in a while..bad news!
bool CFaceVerticalPipe::RegularMailDelivery(double dfTimeNow)
{
    return true;//dfTimeNow-m_dfTimeLastSet < m_dfStarveTime;
}


bool CFaceVerticalPipe::Run(CPathAction &DesiredAction)
{

    // send yaw error back to pid (closed loop)
    if(ShouldReport())
        DesiredAction.SetClosedLoop(YAW, -m_dfBearing, 0.0, m_nPriority, m_sName.c_str());

    return true;
}

bool CFaceVerticalPipe::OnNewMail(MOOSMSG_LIST &NewMail)
{
    double dfNow=MOOSTime();
    CMOOSMsg Msg;

    if(PeekMail(NewMail,"PIPE_BEARING",Msg))
    {
        if(!Msg.IsSkewed(dfNow))
        {
            m_dfTimeLastSet = dfNow;
            m_dfBearing = Msg.m_dfVal;
        }
        else
        {
            MOOSTrace("FaceVerticalPipe: Skewed PIPE_BEARING\n");
        }
    }

    //always call base class version
    CSGMOOSBehaviour::OnNewMail(NewMail);

    return true;
}

bool CFaceVerticalPipe::GetRegistrations(STRING_LIST &List)
{

    List.push_front("PIPE_BEARING");

    //always call base class version
    CSGMOOSBehaviour::GetRegistrations(List);

    return true;
}

bool CFaceVerticalPipe::ShouldReport()
{
    return MOOSTime()-m_dfTimeLastSet < 1.0;
}

