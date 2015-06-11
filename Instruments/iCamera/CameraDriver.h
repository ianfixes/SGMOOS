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
// CameraDriver.h: interface for the CCameraDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERADRIVER_H__25F56D3C_9DD0_4567_88F1_F7F8A0FBCDCF__INCLUDED_)
#define AFX_CAMERADRIVER_H__25F56D3C_9DD0_4567_88F1_F7F8A0FBCDCF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MOOSLIB/MOOSLib.h>
#include <string>
#include "CameraDriverEventsink.h"

using namespace std;


/**
 * Fixme: documentation goes here
 */
class CCameraDriver 
{
public:
    CCameraDriver();
    virtual ~CCameraDriver();
   
//    virtual bool OnNewCommand(CMOOSMsg CmdMsg);

    //interface to the camera
    ///grab an image with the supplied filename
    bool Grab(string m_sDirectory, string m_sFileprefix, int iFrameNumber);

    //double GetLastTime() {return m_dfLastGrabTime;}

    ///set up driver with useful objects
    bool Setup(CCameraDriverEventsink* mySupport, std::string myApp);
    
    ///cleanup function
    bool Cleanup();

    ///handle a mail message
    virtual bool HandleMail(MOOSMSG_LIST &NewMail) = 0;


protected:
    CCameraDriverEventsink*  m_support;
    std::string              m_sAppName;
    int                      m_iFrameNumber;
    
    ///implementation specific: setup the camera
    virtual bool SetupHelper() = 0;

    ///implementation specific: grab an image with the supplied filename
    virtual bool GrabHelper(string m_sDirectory, string m_sFileprefix) = 0;

    ///implementation-specific cleanup
    virtual bool CleanupHelper() = 0;

    ///get application name (supplied by Setup())
    std::string GetAppName();

    ///put an error message in this app's default MOOS register
    void ReportError(std::string s);

    ///broadcast the name of the most recent image that was saved
    void ReportSavedImage(std::string filename);

        
    
};

#endif // !defined(AFX_CAMERADRIVER_H__25F56D3C_9DD0_4567_88F1_F7F8A0FBCDCF__INCLUDED_)

