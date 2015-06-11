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
// CameraDriverDummy.h: a fake camera to test the camera app.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERADRIVERDUMMY_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_)
#define AFX_CAMERADRIVERDUMMY_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CameraDriver.h"
#include <stdio.h>


class CCameraDriverDummy : public CCameraDriver  
{
public:
    
    CCameraDriverDummy();
    virtual ~CCameraDriverDummy();

    //MOOS Interface
    bool Initialise();
    bool GetStatus();

protected:


    /**Method to initialize all of the Shutter/Gain/WhiteBalance, etc features
       of the DCAM camera*/
    bool InitFeatureSet();
    int ParseMode(string & sMode);

    //Camera Interface
    ///clean up the driver
    bool CleanupHelper();

    ///implementation of setup function
    bool SetupHelper();

    ///implementation of grab
    bool GrabHelper(std::string sDirectory, std::string sFileprefix);

    ///handle new mail 
    bool HandleMail(MOOSMSG_LIST &NewMail);
    
};

#endif // !defined(AFX_CAMERADRIVERDUMMY_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_)
