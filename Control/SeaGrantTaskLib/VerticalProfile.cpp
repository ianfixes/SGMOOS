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
// VerticalProfile.cpp: implementation of the CVerticalProfile class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "VerticalProfile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVerticalProfile::CVerticalProfile()
{
    m_sClassName = "Vertical Profile";
    m_bInitialised = false;
    m_dfWinchSpeed = 75.0;
    m_dfSampleStartTime = 10.0;
    m_dfSampleTime = 1.0;  // Should be set by user.
    m_dfDesiredWinch = 0.0;
    m_dfFinishTime = 0.0;
    m_bProfileDone = false;
    m_bPostProfileDone = false;
}

CVerticalProfile::~CVerticalProfile()
{

}

//returns false if we haven't received data in a while..bad news!
bool CVerticalProfile::RegularMailDelivery(double dfTimeNow)
{
    return !m_DepthDOF.IsStale(dfTimeNow,GetStartTime());
}


bool CVerticalProfile::Run(CPathAction &DesiredAction)
{

    if(!m_bInitialised)
    {
        Initialise();
    }

    if(!m_bProfileDone)
    {
        if(m_bSampling)
        {
            // if we're done, go to the next sample point.
            if(MOOSTime()-m_dfSampleStartTime >= m_dfSampleTime)
            {
                m_CurrentProfileStep++;
    
                StartTransiting();
            }
        }
        else  // Transiting
        {
            if((m_dfDesiredWinch > 0.0  && m_DepthDOF.GetError() > 0.0) ||
               (m_dfDesiredWinch < 0.0  && m_DepthDOF.GetError() < 0.0))
            {
                // Continue transiting
            }
            else
            {
                // Done w/ transit
                if(m_CurrentProfileStep == m_ProfileDepths.end())
                    OnProfileEnd();
                else
                {
                    StartSampling();
                }
            }
        }
    }
    else //post profile stuff here.
    {
        if(m_bPostProfileDone)
            OnComplete();
        else
            if(MOOSTime()-m_dfSampleStartTime < m_dfFinishTime)
            {
                m_dfDesiredWinch = -m_dfWinchSpeed;
            }
            else
            {
                m_dfDesiredWinch = 0.0;
                m_bPostProfileDone = true;
            }
    }

    return true;
}


bool CVerticalProfile::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;
    double dfNow = MOOSTime();
    if(PeekMail(NewMail,"YSI_DEPTH",Msg))
    {
        if(!Msg.IsSkewed(dfNow))
        {
	    m_DepthDOF.SetCurrent(Msg.m_dfVal,Msg.m_dfTime);
        }
    }

    if(PeekMail(NewMail,"NAV_ALTITUDE",Msg))
    {
        if(!Msg.IsSkewed(dfNow))
        {
	    m_AltitudeDOF.SetCurrent(Msg.m_dfVal,Msg.m_dfTime);
        }
    }


    //always call base class version
    CSGMOOSBehaviour::OnNewMail(NewMail);

    return true;
}

bool CVerticalProfile::GetNotifications(MOOSMSG_LIST & List)
{
    if(m_bActive)
    {
        CMOOSMsg Msg(MOOS_NOTIFY, "DESIRED_WINCH", m_dfDesiredWinch);
        List.push_back(Msg);
    }

    if(m_bActive && !m_bProfileDone)
    {
        CMOOSMsg Msg(MOOS_NOTIFY, "DESIRED_DEPTH", m_DepthDOF.GetDesired());
        List.push_back(Msg);
     
    }

    return CSGMOOSBehaviour::GetNotifications(List);
}

bool CVerticalProfile::GetRegistrations(STRING_LIST &List)
{

    List.push_front("NAV_ALTITUDE");
    List.push_front("YSI_DEPTH");

    //always call base class version
    CSGMOOSBehaviour::GetRegistrations(List);

    return true;
}


bool CVerticalProfile::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);


    if(!CSGMOOSBehaviour::SetParam(sParam,sVal))
    {
        //this is for us...
        if(MOOSStrCmp(sParam,"PROFILE"))
        {
	    if(ParseProfileString(sVal))
            {
                m_CurrentProfileStep = m_ProfileDepths.begin();
                return true;
            }
        }

        if(MOOSStrCmp(sParam,"SAMPLETIME"))
        {
            m_dfSampleTime = atof(sVal.c_str());
            return true;
        }

        if(MOOSStrCmp(sParam,"WINCHSPEED"))
        {
            m_dfWinchSpeed = atof(sVal.c_str());
            return true;
        }

        if(MOOSStrCmp(sParam,"POSTPROFILE"))
        {
            m_dfFinishTime = atof(sVal.c_str());
            return true;
        }

        else
        {
            // hmmm - it wasn't for us at all, base class didn't understand either
            MOOSTrace("Param \"%s\" not understood!\n",sParam.c_str());
            return false;
        }
    }

    return true;

}

void CVerticalProfile::StartSampling()
{
    m_dfDesiredWinch = 0.0;
    m_dfSampleStartTime = MOOSTime();
    m_bSampling = true;
}

void CVerticalProfile::StartTransiting()
{
    // If we're done, go home, otherwise transit to next spot.
    if(m_CurrentProfileStep == m_ProfileDepths.end())
        m_DepthDOF.SetDesired(0.2);
    else
        m_DepthDOF.SetDesired(*m_CurrentProfileStep);

    double dfSign = (m_DepthDOF.GetError()>0) ? 1.0 : -1.0;
    m_dfDesiredWinch = dfSign * m_dfWinchSpeed;

    m_bSampling = false;
}

bool CVerticalProfile::ParseProfileString(string sDepths)
{

    string sTok;
    while(!sDepths.empty())
    {
        sTok = MOOSChomp(sDepths, ",");
        MOOSTrimWhiteSpace(sTok);
        if(MOOSIsNumeric(sTok))
            m_ProfileDepths.push_back(atof(sTok.c_str()));
        else
            return false;
    }

    MOOSTrace("Vertical Profile read %d depths\n", m_ProfileDepths.size());

    return m_ProfileDepths.size() != 0;
}

bool CVerticalProfile::OnProfileEnd()
{

    m_dfSampleStartTime = MOOSTime();
    m_dfDesiredWinch = -m_dfWinchSpeed;
    m_bProfileDone = true;
    return true;
}

bool CVerticalProfile::Initialise()
{
    //set a pitch driven depth controller

    StartTransiting();
    m_bInitialised = true;

    return false;
}

