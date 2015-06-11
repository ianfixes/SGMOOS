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

    SGMOOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SGMOOS.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/
// CameraApp.cpp: implementation of the CCameraApp class.
//
//////////////////////////////////////////////////////////////////////

#include "CameraApp.h"
#include "CameraDriverDC1394.h"
#include "CameraDriverDummy.h"
#include "CameraDriverGphoto2.h"

#include <time.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace std;


/**
 * fixme
 */
CCameraApp::CCameraApp() : CMOOSApp()
{

    //Use CMOOSApp's command message filtering feature
    EnableCommandMessageFiltering(true);

    //sort messages by time
    SortMailByTime(true);

    m_driver = NULL;

    m_nSessions = 0;
    m_nFrames = 0;

    m_dfPeriodS = -1;
    m_dfLastGrabTime = -1; 
}

CCameraApp::~CCameraApp()
{
    Cleanup();
    if (m_driver) delete m_driver;

    MOOSTrace("CCameraApp destructing cleanly\n");
}

void CCameraApp::Cleanup()
{
    if (m_driver && !m_driver->Cleanup())
    {
        MOOSTrace("Camera driver didn't exit cleanly!\n");
    }

}

/**
 * Camera Initialization
 * 
 * Make sure all config params are available, print an error if not
 * load all member variables 
 * init the camera driver
 */
bool CCameraApp::OnStartUp()
{
    std::string sTempString, v, sDriver;
    
    std::string sUsage = 
        "\nMOOS MISSION FILE CONFIGURATION ERROR!\n"
        "The iCamera application expects the following configuration parameters:\n"
        "\tType          = <DC1394|gphoto|dummy>\n"
        "\tPeriod_Min_s  = <Min time between camera captures, in seconds>\n"
        "\tOutput_dir    = <directory to save files in, no trailing slash>\n"
        "\nThe camera application cannot continue.\n"
        "\n";

    //check type
    if (!m_MissionReader.GetConfigurationParam("Type", sDriver))
    {
        MOOSTrace(sUsage);
        return false;
    }

    //check output directory
    if (!m_MissionReader.GetConfigurationParam("OUTPUT_DIR", m_sOutputBaseDir))
    {
        MOOSTrace(sUsage);
        return false;
    }

    //check period
    if (!m_MissionReader.GetConfigurationParam("Period_Min_s", sTempString))
    {
        MOOSTrace(sUsage);
        return false;
    }
    else
    {
        //fixme, this should go in the driver maybe
        m_dfMinPeriodS = atof(sTempString.c_str());
    }



    //get the driver... note single equals
    if (!(m_driver = GetDriver(sDriver)))
    {
        MOOSTrace("ERROR: Unknown Driver Type (%s)", sDriver.c_str());
        return false;
    }

    //disclose camera type
    v = GetAppName() + "_TYPE";
    MOOSToUpper(v);    
    m_Comms.Notify(v, sDriver);

    //put up help message
    v = GetAppName() + "_CMD";
    MOOSToUpper(v);
    m_Comms.Notify(v, "Try 'GRABEVERY=<num seconds>' or 'STOPGRABBING'");
    

    return m_driver->Setup(this, GetAppName());

}


/**
 * return a driver class based on the name
 */
CCameraDriver* CCameraApp::GetDriver(std::string DriverName)
{
    if (MOOSStrCmp(DriverName, "Dummy"))
        return new CCameraDriverDummy();

    if (MOOSStrCmp(DriverName, "DC1394"))
        return new CCameraDriverDC1394();

    if (MOOSStrCmp(DriverName, "Gphoto2"))
        return new CCameraDriverGphoto2();
        
    //   if (MOOSStrCmp(DriverName "gphoto"))
    //   return new CCameraDriverGphoto();

    return NULL;
}


bool CCameraApp::OnCommandMsg(CMOOSMsg CmdMsg)
{
    if(CmdMsg.IsSkewed(MOOSTime()))
    {
        MOOSTrace("Got Skewed Command");
        return false;
    }

    string sCommand = CmdMsg.m_sVal;
    string sPrefix = MOOSChomp(sCommand, "=");
    string sFilename;
    string sDestinationDir;
    char   pDirPath[1024];

    double tmp;
    
    //grab every n seconds
    // soft limit on the min period of the camera

    if (MOOSStrCmp(sPrefix,"GRAB"))
    {   

        time_t rawtime;
        struct tm* timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        //if a filename was supplied, use it otherwise use a generated one
        if (!sCommand.empty())
        {
            sFilename = sCommand;
        }
        else
        {
            sFilename = 
                MOOSFormat("%04d-%02d-%02d_%02d%02d%02d",
                           (1900 + timeinfo->tm_year),
                           (1 + timeinfo->tm_mon),
                           timeinfo->tm_mday,
                           timeinfo->tm_hour,
                           timeinfo->tm_min,
                           timeinfo->tm_sec);
        }
    
        //just need to make sure the necessary time has elapsed
        if (CameraIsReady())
        {
            m_dfLastGrabTime = MOOSTime();
            //frame number of -1 to indicate not to save it
            m_driver->Grab(m_sOutputDir, sFilename, -1);
        }
    }
    else if (MOOSStrCmp(sPrefix,"GRABEVERY"))
    {
        //update session number and directory
        m_nSessions++;
        sDestinationDir = TimestampedDirectory();


        sprintf(pDirPath, "%s/%s", 
                m_sOutputBaseDir.c_str(),
                sDestinationDir.c_str());

        m_sOutputDir = pDirPath;

        mkdir(m_sOutputDir.c_str(), 0777);

        tmp = atof(sCommand.c_str());
        m_dfPeriodS = tmp < m_dfMinPeriodS ? m_dfMinPeriodS : tmp;
        return true;
    }

    else if (MOOSStrCmp(sPrefix, "STOPGRABBING"))
    {
        m_dfPeriodS = -1;
        return true;
    }

    MOOSTrace("Unknown command: ");
    MOOSTrace(CmdMsg.m_sVal);

    return false;
}

/**
 * get the name for a directory
 */
std::string CCameraApp::TimestampedDirectory()
{
    time_t rawtime;
    struct tm* timeinfo;
    
    char ret[255];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(ret, "iCamera_%04d-%02d-%02d_%02d%02d_session%04d",
            (1900 + timeinfo->tm_year),
            (1 + timeinfo->tm_mon),
            timeinfo->tm_mday,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            m_nSessions);

    return ret;
        
}




bool CCameraApp::Iterate()
{
    char fileprefix[255];

    sprintf(fileprefix, "%06d", m_nFrames);

    //if we have a min capture period set, then we're on
    if (m_dfPeriodS > -1)
    {
        //just need to make sure the necessary time has elapsed
        if (CameraIsReady())
        {
            m_dfLastGrabTime = MOOSTime();
            m_driver->Grab(m_sOutputDir, fileprefix, m_nFrames);
            m_nFrames++;
        }
    }

    return true;
}


bool CCameraApp::OnNewMail(MOOSMSG_LIST &NewMail)
{

    return m_driver->HandleMail(NewMail);
}


bool CCameraApp::CameraIsReady()
{
    return ((MOOSTime() - m_dfLastGrabTime) > m_dfPeriodS);
}

