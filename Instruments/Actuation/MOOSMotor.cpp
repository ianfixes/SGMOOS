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

// MOOSMotor.cpp: implementation of the CMOOSMotor class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include "MOOSMotor.h"
#include "MOOSCopleyDriver.h"
#include "MOOSParkerMotorDriver.h"
#include "MOOSJRKerrMotorDriver.h"
#include <iostream>
#include <math.h>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSMotor::CMOOSMotor()
{

    m_bSimMode=false;

    //some sensible defaults (missionfile can overwrite this)
    SetAppFreq(5);
    SetCommsFreq(8);
    m_bReportVelocity=false;
    m_bReportPosition=false;
    m_bReportCurrent=false;
    m_bReverse = false;
    m_dfPositionOffset=0.0;
    m_dfStatusInterval = 5.0;

    m_bDoProfile=false;
    m_dfProfileSum=0.0;
    m_nNumProfiles=0;
}

CMOOSMotor::~CMOOSMotor()
{
    delete m_MotorDriver;
}

bool CMOOSMotor::OnStartUp()
{
    
    CMOOSInstrument::OnStartUp();
    if(!SetupPort())
        return false;
    
    string sMyNameIs = GetAppName();
    MOOSToUpper(sMyNameIs);
    double dfMotorPeriod = 0.01;
    string TmpStr;

    //Variables which should be relevant for all Motors (not driver specific)
    AddMOOSVariable("StatusMsg","",MOOSFormat("%s_STATUS", sMyNameIs.c_str()), dfMotorPeriod);
    //AddMOOSVariable("StatusCode","",MOOSFormat("%s_STATUS_CODE", sMyNameIs.c_str()), dfMotorPeriod);

    m_MissionReader.GetConfigurationParam("TYPE",TmpStr);
    MakeDriver(TmpStr);
    if(m_MotorDriver == NULL)
        return false;
        
    if(!m_MotorDriver->Initialise())
      return false; 

    // Input variable is independent of mode
    AddMOOSVariable("Desired", MOOSFormat("DESIRED_%s", sMyNameIs.c_str()), "",  dfMotorPeriod);

    ModeEnumType nMode;
    if(m_MissionReader.GetConfigurationParam("MODE", TmpStr))
    {
        if(MOOSStrCmp(TmpStr, "POSITION"))
        {
            m_DesiredAction = &CMOOSMotorDriver::SetPosition;
	    m_bReportPosition=true;
            nMode=POSITION;
        }
	else if(MOOSStrCmp(TmpStr, "VELOCITY"))
        {
            m_DesiredAction = &CMOOSMotorDriver::SetVelocity;
	    m_bReportVelocity=true;
            nMode=VELOCITY;
        }
	else if(MOOSStrCmp(TmpStr, "PWM"))
        {
            m_DesiredAction = &CMOOSMotorDriver::SetPercentOutput;
	    nMode=PWM;
        }
	else
	{
            MOOSTrace("Invalid Mode for Motor: (" + TmpStr + ")\n");
	    return false;
	}

	if(!m_MotorDriver->SetMode(nMode))
	{
            MOOSTrace("Unable to set to driver to " + TmpStr + " mode.\n");
	    return false;
	} 
    }
    else
    {
        MOOSTrace("No Mode indicated in Mission File\n");
	return false;
    }
    
    
    bool bTemp;

    if(m_MissionReader.GetConfigurationParam("ReportPosition", bTemp))
        m_bReportPosition=bTemp;
 
    if(m_MissionReader.GetConfigurationParam("ReportVelocity", bTemp))
        m_bReportVelocity=bTemp;
    
    if(m_MissionReader.GetConfigurationParam("ReportCurrent", bTemp))
        m_bReportCurrent=bTemp;
    
    if(m_bReportPosition)
    	AddMOOSVariable("MotorPosition","",MOOSFormat("%s_POSITION", sMyNameIs.c_str()), dfMotorPeriod);
    
    if(m_bReportVelocity)
    	AddMOOSVariable("MotorVelocity","",MOOSFormat("%s_VELOCITY", sMyNameIs.c_str()), dfMotorPeriod);

    if(m_bReportCurrent)
    	AddMOOSVariable("MotorCurrent","",MOOSFormat("%s_CURRENT", sMyNameIs.c_str()), dfMotorPeriod);

    double dfTemp;
    if(m_MissionReader.GetConfigurationParam("Acceleration", dfTemp))
    {
        m_MotorDriver->SetAcceleration(dfTemp);
    }

    if(m_MissionReader.GetConfigurationParam("PositionOffset", dfTemp))
    {
        m_dfPositionOffset=dfTemp;
    }

    if(m_MissionReader.GetConfigurationParam("Deceleration", dfTemp))
    {
        m_MotorDriver->SetDeceleration(dfTemp);
    }

    if(m_MissionReader.GetConfigurationParam("Reverse", TmpStr))
    {
        if(MOOSStrCmp("TRUE", TmpStr))
            m_bReverse = true;
    }

    if(m_MissionReader.GetConfigurationParam("AutoWrap", bTemp))
    {
        m_MotorDriver->SetAutoWrap(bTemp);
    }

    RegisterMOOSVariables();
    return true;
}

bool CMOOSMotor::OnConnectToServer()
{

    string sMyNameIs = GetAppName();
    MOOSToUpper(sMyNameIs);
    double dfMotorPeriod=0.01;

    AddMOOSVariable("Command", MOOSFormat("%s_COMMAND", sMyNameIs.c_str()), "", dfMotorPeriod);
    AddMOOSVariable("Enable",  MOOSFormat("%s_ENABLE", sMyNameIs.c_str()), "", dfMotorPeriod);

    RegisterMOOSVariables();
    return true;
}

bool CMOOSMotor::OnNewMail(MOOSMSG_LIST &NewMail)
{	

    CMOOSMsg Msg;
    string sMyNameIs = GetAppName();
    MOOSToUpper(sMyNameIs);
    
    UpdateMOOSVariables(NewMail);

    
    if(m_MOOSVars["Desired"].IsFresh())
    {
        if(m_MotorDriver->IsEnabled())
        {
            double dfNow = MOOSTime();
            double dfVal=m_MOOSVars["Desired"].GetDoubleVal();
            if(m_MotorDriver->GetMode()==POSITION)
                 dfVal += m_dfPositionOffset;
            (m_MotorDriver->*m_DesiredAction) ((m_bReverse)? -1.0*dfVal : dfVal);
            m_MOOSVars["Desired"].SetFresh(false);
            if(m_bDoProfile)
            {
                double dfTime = MOOSTime()-dfNow;
                m_dfProfileSum += dfTime;
                m_nNumProfiles++;
                MOOSTrace("Command took %f seconds... Average: %f\n", dfTime, m_dfProfileSum/(double)m_nNumProfiles);
            }
      }
      else
	  MOOSTrace("%s not Enabled\n", m_sAppName.c_str());
    }
    else 
        MOOSTrace("DESIRED Action not fresh\n");

    if(m_Comms.PeekMail(NewMail, MOOSFormat("%s_COMMAND", sMyNameIs.c_str()), Msg))
    {
            if( !Msg.IsSkewed( MOOSTime() ) )
                m_MotorDriver->SendCommand(Msg.m_sVal);
    }

    if(m_Comms.PeekMail(NewMail,MOOSFormat("%s_ENABLE", sMyNameIs.c_str()), Msg))
    {
            if(!Msg.IsSkewed( MOOSTime() ) )
            {
                bool m_bEnableVal = MOOSStrCmp(Msg.GetString(), "True");
                m_MotorDriver->Enable(m_bEnableVal);
            }
    }

    return true;

}

bool CMOOSMotor::Iterate()
{
    bool bOK = GetData() && GetStatus();
    
    PublishFreshMOOSVariables();

    return bOK;
}

bool CMOOSMotor::GetData()
{
    double dfPos, dfVel, dfI;
    bool bOK = true;
    double dfTimeNow = MOOSTime();
    
    if(m_bReportPosition)
    {
        if(m_MotorDriver->GetPosition(dfPos))
        {
            if(m_bReverse) 
                dfPos *= -1.0;
            dfPos -= m_dfPositionOffset;
		//
		// Is this sadistic or what?
		dfPos = MOOSRad2Deg(MOOS_ANGLE_WRAP(MOOSDeg2Rad(dfPos)));

        	SetMOOSVar("MotorPosition", dfPos, dfTimeNow);
        }
        else 
            bOK = false;
    }

    if(m_bReportVelocity)
    {
        if(m_MotorDriver->GetVelocity(dfVel))
            SetMOOSVar("MotorVelocity", (m_bReverse) ? -1.0*dfVel : dfVel, dfTimeNow);
        else
            bOK = false;
    }

    if(m_bReportCurrent)
    {
        if(m_MotorDriver->GetCurrent((dfI)))
            SetMOOSVar("MotorCurrent", dfI, dfTimeNow);
         else
            bOK = false;
   }
    return bOK;
}

bool CMOOSMotor::GetStatus()
{
    int nStatusCode;

    string sStatusDescription;
    if(m_MotorDriver->GetStatus(nStatusCode, sStatusDescription))
    {
        double dfTimeNow = MOOSTime();
        if((m_sPreviousStatus != sStatusDescription) || (dfTimeNow-m_dfStatusPosted>=m_dfStatusInterval))
	{
            m_sPreviousStatus = sStatusDescription;
            m_dfStatusPosted = dfTimeNow;
            SetMOOSVar("StatusMsg", sStatusDescription, dfTimeNow);
        }
        return true;
    }
    return false;
}

bool CMOOSMotor::MakeDriver(string sDriverName)
{
    if(MOOSStrCmp(sDriverName, "COPLEY"))
        m_MotorDriver = new CMOOSCopleyDriver();
    else if(MOOSStrCmp(sDriverName, "PARKER"))
        m_MotorDriver = new CMOOSParkerMotorDriver();
    else if(MOOSStrCmp(sDriverName, "JRKERR"))
        m_MotorDriver = new CMOOSJRKerrMotorDriver();
    else if(MOOSStrCmp(sDriverName, "Generic"))
    {
        MOOSTrace("ERROR: driver not specified!  Please specify driver in your mission file\n");
        return false;
    }
    else
    {
        MOOSTrace("ERROR: driver \"" + sDriverName + " not implemented, please check for typos or write a driver\n");
        return false;
    }
    
    m_MotorDriver->SetSerial(&m_Port);
    return true;
}

