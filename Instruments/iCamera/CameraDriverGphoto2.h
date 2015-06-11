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
// CameraDriverGphoto2.h: supporting Gphoto2 cameras.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERADRIVERGPHOTO2_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_)
#define AFX_CAMERADRIVERGPHOTO2_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MOOSLIB/MOOSLib.h>
#include "CameraDriver.h"
#include <stdio.h>
#include "CLIBuster.h"


class CCameraDriverGphoto2 : public CCameraDriver  
{
public:
    
    CCameraDriverGphoto2();
    virtual ~CCameraDriverGphoto2();

    //MOOS Interface
    bool Initialise();
    bool GetStatus();

protected:
    CLIBuster* m_cb;
    std::string m_sGphotoBinary;
    bool m_bChangedDirectory;

    //Camera Interface
    ///clean up the driver
    bool CleanupHelper();

    ///implementation of setup function
    bool SetupHelper();

    ///handle new mail
    bool HandleMail(MOOSMSG_LIST &NewMail);

    ///get all config params
    bool GetConfigs();

    ///return true if all necessary config params were supplied
    bool GotAllConfigs();

    ///create registes that correspond to SUPPORTED camera features
    void CreateCameraRegisters();

    ///implementation of grab
    bool GrabHelper(std::string sDirectory, std::string sFileprefix);
    
};

#endif // !defined(AFX_CAMERADRIVERGPHOTO2_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_)
