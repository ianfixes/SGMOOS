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
// BaslerCameraDriver.h: interface for the CBaslerCameraDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASLERCAMERADRIVER_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_)
#define AFX_BASLERCAMERADRIVER_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CameraDriver.h"
#include "stdafx.h"
#include <BvcDib.h>
#include <BvcColorConverter.h>

class CBaslerCameraDriver : public CCameraDriver  
{
public:
    unsigned long GetShutter();
    unsigned long GetBrightness();
    unsigned long GetWhiteBalanceRed();
    unsigned long GetWhiteBalanceBlue();
    unsigned long GetGain();
    
    void SetGain(unsigned long ulGain);
    void SetShutter(unsigned long ulShutter);
    void SetWhiteBalanceRed(unsigned long ulWhiteBalanceRed);
    void SetWhiteBalanceBlue(unsigned long ulWhiteBalanceBlue);
    void SetBrightness(unsigned long ulBrightness);
    
    CBaslerCameraDriver();
    virtual ~CBaslerCameraDriver();
    
    //MOOS Interface
    bool Initialise();
    bool GetStatus();
protected:
    /**Method to initialize all of the Shutter/Gain/WhiteBalance, etc features
       of the DCAM camera*/
    bool InitFeatureSet();
    CSize m_ImageSize;
    int m_nRGBMode;
    int ParseMode(string & sMode);
    /**the Device Independent Bitmap pointer to Basler's encapsulating CDib class*/
    CDibPtr m_DibPtr;
    //camera object
    CBcam m_Bcam;
    CString m_DeviceName;
    //Camera Interface
    bool Grab();
    bool Save();
    bool SetMode(int nMode);
    
    DCSVideoFormat m_VideoFormat;
    DCSVideoMode m_VideoMode;
    DCSVideoFrameRate m_FrameRate;
    DCSColorCode m_ColorCode;
    
    /**The A102fc Basler camera has a Monochrome or Color mode*/
    typedef enum 
    {
        EMONO = 0,
        ECOLOR,
        EYUV,
        ESPECIAL //Format 7
    } 
    ERGBMode;
    
};

#endif // !defined(AFX_BASLERCAMERADRIVER_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_)
