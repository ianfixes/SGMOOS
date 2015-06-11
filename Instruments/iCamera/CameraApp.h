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
// CameraApp.h: interface for the CCameraApp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERAAPP_H__2D792428_206D_4E8A_98A9_3A7F58D21EF9__INCLUDED_)
#define AFX_CAMERAAPP_H__2D792428_206D_4E8A_98A9_3A7F58D21EF9__INCLUDED_

#include <MOOSLIB/MOOSLib.h>
#include <string>
#include "CameraDriver.h"
#include "CameraDriverEventsink.h"


using namespace std;

class CCameraApp : public CMOOSApp, public CCameraDriverEventsink
{
public:
    CCameraApp();
    virtual ~CCameraApp();
    

    void Cleanup();

    ///get the driver by name
    CCameraDriver* GetDriver(std::string DriverName);


    /**
    *we override the OnNewMail to handle Task's sending of variable
    *with more than one value 
    */

    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool OnCommandMsg(CMOOSMsg CmdMsg);

    bool OnStartUp();
    bool Iterate();

protected:

    CCameraDriver* m_driver;

    int     m_nFrames;
    int     m_nSessions;
    double  m_dfLastGrabTime;
    string  m_sOutputDir;
    string  m_sOutputBaseDir;


    ///minimum period that the camera can handle
    double m_dfMinPeriodS;

    ///period requested by the user
    double m_dfPeriodS;

    ///whether the period has elapsed
    bool CameraIsReady();

    ///get a string representation of date and session number (for output directory)
    std::string TimestampedDirectory();


public:
    //CCameraDriverEventsink interface
    bool ConfigRead(const std::string k, std::string &v) {return m_MissionReader.GetConfigurationParam(k, v);};
    bool CommsRegister(const std::string s) {return m_Comms.Register(s, 0.1);};
    bool CommsNotify(const std::string k, std::string s, double t) {return m_Comms.Notify(k, s, t);};
    bool CommsNotify(const std::string k, double      d, double t) {return m_Comms.Notify(k, d, t);};
    bool CommsPeekMail(MOOSMSG_LIST l, std::string v, CMOOSMsg &m) {return m_Comms.PeekMail(l, v, m);};

};

#endif // !defined(AFX_CAMERAAPP_H__2D792428_206D_4E8A_98A9_3A7F58D21EF9__INCLUDED_)
