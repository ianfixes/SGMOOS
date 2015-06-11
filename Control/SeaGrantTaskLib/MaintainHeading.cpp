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
// MaintainHeadingTask.cpp: implementation of the CMaintainHeading class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;
#include <MOOSGenLib/MOOSGenLib.h>

#include "MaintainHeading.h"
#include "SeaGrantGlobalHelper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CMaintainHeading::CMaintainHeading()
{

    m_bInitialised = false;

    m_sClassName = "Maintain Heading";
    m_YawDOF.SetDesired(0);
    m_YawDOF.SetTolerance(0.00);
    m_bAllowHeadingChanges=false;
}

CMaintainHeading::~CMaintainHeading()
{

}

//returns false if we haven't received data in a while..bad news!
bool CMaintainHeading::RegularMailDelivery(double dfTimeNow)
{
    return !m_YawDOF.IsStale(dfTimeNow,GetStartTime());
}


bool CMaintainHeading::Run(CPathAction &DesiredAction)
{

    if(!m_bInitialised)
    {
        Initialise();
    }
	
    if(m_YawDOF.IsValid())
    {

        double dfError = m_YawDOF.GetError();
	
	if(dfError<-PI) dfError += 2*PI;
	else if(dfError>PI) dfError -= 2*PI;

        // send yaw error back to pid (closed loop)
        DesiredAction.SetClosedLoop(YAW, dfError, m_YawDOF.GetDesired(), m_nPriority, m_sName.c_str());

        PublishDesiredHeading();

    }
    return true;
}

bool CMaintainHeading::OnNewMail(MOOSMSG_LIST &NewMail)
{
    double dfNow=MOOSTime();
    CMOOSMsg Msg;
    if(PeekMail(NewMail,"NAV_YAW",Msg))
    { 
        if(!Msg.IsSkewed(dfNow))
        {
            //yaw is in radians
            m_YawDOF.SetCurrent(Msg.m_dfVal, Msg.m_dfTime);
        }
        else
        {
            MOOSTrace("MaintainHeading: Skewed NAV_YAW\n");
        }
    }

    if(PeekMail(NewMail,"MANUAL_HEADING",Msg))
    {
        if(!Msg.IsSkewed(dfNow))
        {
            SetDesiredHeading(Msg.m_dfVal);
        }
    }

    if(PeekMail(NewMail,"MANUAL_HEADING_STEP",Msg))
    {
        if(!Msg.IsSkewed(dfNow))
        {
            double dfCurrentHeading = -1.0 * MOOSRad2Deg(m_YawDOF.GetDesired());
            SetDesiredHeading(dfCurrentHeading + Msg.m_dfVal);
        }
    }
    //always call base class version
    CSGMOOSBehaviour::OnNewMail(NewMail);

    return true;
}

bool CMaintainHeading::GetRegistrations(STRING_LIST &List)
{

    List.push_front("NAV_YAW");

    if(m_bAllowHeadingChanges)
    {
        List.push_front("MANUAL_HEADING");
        List.push_front("MANUAL_HEADING_STEP");
    }
    //always call base class version
    CSGMOOSBehaviour::GetRegistrations(List);

    return true;
}


bool CMaintainHeading::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);
    
    if(CSGMOOSBehaviour::SetParam(sParam,sVal)) return true;

    
    //if we get here, this is for us...

    if(MOOSStrCmp(sParam,"ALLOWHEADINGCHANGES"))
    {
        ParseBoolean(m_bAllowHeadingChanges, sVal);
        return true;
    }
    else if(MOOSStrCmp(sParam,"HEADING"))
    {
        
        //now user specifies heading w.r.t to true north in degrees,
        // the compass or other magnetic device has already made the corrections...
        double dfDesired = atof(sVal.c_str());
        
        double dfMagneticOffset = 0.0;
        
        if(m_pMissionFileReader)
        {
            if(!m_pMissionFileReader->GetValue("MAGNETICOFFSET",dfMagneticOffset))
            {
                MOOSTrace("WARNING: No magnetic offset specified "
                          "in Mission file (Field name = \"MagneticOffset\")\n");
            }
        }
        
        SetDesiredHeading(dfDesired);

        return true;
    }
    
    return false;

}

bool CMaintainHeading::Initialise()
{
    m_bInitialised = true;

    return true;
}

void CMaintainHeading::SetDesiredHeading(double dfHeading)
{
    m_YawDOF.SetDesired(MOOS_ANGLE_WRAP(-1.0*MOOSDeg2Rad(dfHeading)));
}

void CMaintainHeading::PublishDesiredHeading()
{

    //Tell the world what you want.
    CMOOSMsg YawGoal(MOOS_NOTIFY,"DESIRED_HEADING", -1.0 * MOOSRad2Deg(m_YawDOF.GetDesired()));
    m_Notifications.push_front(YawGoal);
}

void CMaintainHeading::GetChartNodeContents(STRING_LIST& contents) const
{
    CSGMOOSBehaviour::GetChartNodeContents(contents);
    contents.push_back(MOOSFormat("Heading: %7.2f", -1.0*MOOSRad2Deg(m_YawDOF.GetDesired())));
}

