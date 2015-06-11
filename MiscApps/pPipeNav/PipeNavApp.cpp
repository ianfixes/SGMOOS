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
//PipeNavApp.cpp - implementation of the CPipeNav class
//
//////////////////////////////////////////////////////////////////////

#include "PipeNavApp.h"
#include <math.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace std;


CPipeNavApp::CPipeNavApp() : CMOOSApp()
{

}

CPipeNavApp::~CPipeNavApp()
{

}

bool CPipeNavApp::OnStartUp()
{
    string sTemp;
    if(m_MissionReader.GetConfigurationParam("PIPELOCATION",sTemp))
    {
        string sX = MOOSChomp(sTemp, ",");
        m_dfPipeX = atof(sX.c_str());
        if(MOOSIsNumeric(sTemp))
        {
            m_dfPipeY = atof(sTemp.c_str());
            return true;
        }
    }

    MOOSTrace("Error Reading Pipe Location from config file\n");
    return false;
}

bool CPipeNavApp::OnConnectToServer()
{
    m_Comms.Register("NAV_YAW",0.1);
    m_Comms.Register("PIPE_BEARING",0.1);
    m_Comms.Register("PIPE_RANGE",0.1);
    return true;
}

bool CPipeNavApp::Iterate()
{
    return true;
}

bool CPipeNavApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;

    if(m_Comms.PeekAndCheckMail(NewMail,"NAV_YAW",Msg))
    {
        m_dfYaw = Msg.m_dfVal;
    }

    double dfBearing, dfRange = -1.0;
    if(m_Comms.PeekAndCheckMail(NewMail,"PIPE_RANGE",Msg))
        dfRange = Msg.m_dfVal;
    
    if(m_Comms.PeekAndCheckMail(NewMail,"PIPE_BEARING",Msg))
    {
        if(dfRange < 0.0) return false;

        double dfX, dfY;
        dfX = m_dfPipeX + dfRange*sin(m_dfYaw-Msg.m_dfVal);
        dfY = m_dfPipeY - dfRange*cos(m_dfYaw-Msg.m_dfVal);
        m_Comms.Notify("PIPE_RELATIVE_X", dfX, Msg.m_dfTime);
        m_Comms.Notify("PIPE_RELATIVE_Y", dfY, Msg.m_dfTime);
        
    }
        
    return true;
}
