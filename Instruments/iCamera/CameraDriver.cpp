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
// CameraDriver.cpp: implementation of the CCameraDriver class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include "CameraDriver.h"
#include <MOOSLIB/MOOSLib.h>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCameraDriver::CCameraDriver()
{
    
}

CCameraDriver::~CCameraDriver()
{

}

bool CCameraDriver::Setup(CCameraDriverEventsink *mySupport, std::string myApp)
{  
    m_support = mySupport;
    m_sAppName = myApp;

    return SetupHelper();
}

bool CCameraDriver::Grab(std::string sDirectory, std::string sFileprefix, int iFrameNumber)
{
    m_iFrameNumber = iFrameNumber;
    return GrabHelper(sDirectory, sFileprefix);
}

std::string CCameraDriver::GetAppName()
{
    return m_sAppName;
}

void CCameraDriver::ReportError(std::string s)
{
    std::string v(GetAppName());
    v += "_ERROR";

    MOOSToUpper(v);

    m_support->CommsNotify(v, s);
}

void CCameraDriver::ReportSavedImage(std::string sSavedFilename)
{
    std::string v(GetAppName());
    std::string tmp;
    double t = MOOSTime();

    MOOSToUpper(v);

    tmp = v + "_SAVEDFILE";
    m_support->CommsNotify(tmp, sSavedFilename, t);

    //record frame number if greater than -1 (which means we did a single grab)
    if (-1 < m_iFrameNumber)
    {
        tmp = v + "_FRAMENUMBER";
        m_support->CommsNotify(tmp, m_iFrameNumber, t);
    }
}

bool CCameraDriver::Cleanup()
{
    return this->CleanupHelper();
}

 /**
  * handle mail event, essentially the leftovers from the camera app
  * that calls the driver
  *
  * FIXME, this goes in the specific implementation
  */
 //bool CCameraDriver::HandleMail(CMOOSMsg CmdMsg)
 //{
 //
 //    MOOSTrace("CCameraDriver::OnNewCommand(): <%s>\n", CmdMsg.m_sVal.c_str());
 //    //Command Syntax:
 //    //<COMMAND>[=<Val>]
 //
 //}

