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
// CameraDriverDC1394.h: supporting DC1394 cameras.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERADRIVERDC1394_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_)
#define AFX_CAMERADRIVERDC1394_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CameraDriver.h"
#include <stdio.h>
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#include <stdlib.h>
#include <sys/time.h>
#include "conversions.h"


//#define CARD_1394    0
//#define VIDEO_DEVICE "/dev/video1394-0"

enum
{
    ENCODING_RGB = 0,
    ENCODING_BAYER,
    ENCODING_RGB48,
    ENCODING_UYV,
    ENCODING_UYVY,
    ENCODING_UYYVYY,
    ENCODING_Y,
    ENCODING_Y16
};


class CCameraDriverDC1394 : public CCameraDriver  
{
public:
    
    CCameraDriverDC1394();
    virtual ~CCameraDriverDC1394();

    //MOOS Interface
    bool Initialise();
    bool GetStatus();

protected:


    raw1394handle_t       m_raw1394Handle;
    dc1394_cameracapture  m_dc1394Camera;
    nodeid_t              m_cameraNode;
    bool                  m_bRaw1394HandleCreated;
    bool                  m_bDc1394CameraCreated;
    unsigned char*        m_rgbBuffer;
    int                   m_iFrameWidth;
    int                   m_iFrameHeight;

    int                   m_iMode;
    int                   m_iFormat;
    int                   m_iFramerate;
    int                   m_iTriggermode;
    int                   m_iDataspeed;
    int                   m_iEncoding;
    int                   m_iBytesPerPixel;
    int                   m_i1394Card;
    int                   m_iCameraNode;
    std::string           m_sVideoDevice;


    /**Method to initialize all of the Shutter/Gain/WhiteBalance, etc features
       of the DCAM camera*/
    bool InitFeatureSet();
    int ParseMode(string & sMode);

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

    ///print out as much data as possible on the camera
    void dumpCameraInfo();

    ///print out an error message (soft!) if the camera isn't ready to do something
    void CheckReadiness();

    ///check if a command was successful, print an error if not
    void Check1394Success(int r, const char* feature);

    ///convert a config parameter from a string to an integer based on a callback
    int getCfgEnumValue(const char* s, int (*work)(const char*), bool reqd);

    ///implementation of grab
    bool GrabHelper(std::string sDirectory, std::string sFileprefix);

    ///grab an image into memory
    bool GrabImg();

    ///save an in-memory image to disk
    bool Save(std::string sFilename);


    ///check if the camera function works, create a MOOS register with the name of the feature
    void CreateRegisterNamed(const char* feature, 
                             int (*work)(raw1394handle_t, nodeid_t, unsigned int*));


    ///this function makes my life easier by doing all the work associated with a get uint
    unsigned int GeneralUintGet(const char* feature, 
                                int (*work)(raw1394handle_t, nodeid_t, unsigned int*));

    
    ///this function makes my life easier by doing all the work associated with a set uint
    //... pre-check, error check, and retval
    void GeneralUintSet(const char* feature, 
                        int (*work)(raw1394handle_t, nodeid_t, unsigned int),
                        unsigned int v);
    

    ///handle an incoming command from MOOS with a set function
    void GeneralHandleUInt(const char* suffix, 
                           MOOSMSG_LIST &NewMail, 
                           int (*work)(raw1394handle_t, nodeid_t, unsigned int));


    ///read bayer pattern from camera
    int GetBayerPattern();
    
    static int str2mode(const char* s);
    static int str2format(const char* s);
    static int str2framerate(const char* s);
    static int str2triggermode(const char* s);
    static int str2dataspeed(const char* s);
    static int str2encoding(const char* s);

    //camera functions

    //this is the only weird one
    void GetWhteBalance(unsigned int* blueval, unsigned int* redval);
    void SetWhiteBalance(unsigned int blueval, unsigned int redval);

    unsigned int GetBrightness();
    void         SetBrightness(unsigned int v);
    unsigned int GetExposure();
    void         SetExposure(unsigned int v);
    unsigned int GetSharpness();
    void         SetSharpness(unsigned int v);
    unsigned int GetHue();
    void         SetHue(unsigned int v);
    unsigned int GetSaturation();
    void         SetSaturation(unsigned int v);
    unsigned int GetGamma();
    void         SetGamma(unsigned int v);
    unsigned int GetShutter();
    void         SetShutter(unsigned int v);
    unsigned int GetGain();
    void         SetGain(unsigned int v);
    unsigned int GetIris();
    void         SetIris(unsigned int v);
    unsigned int GetFocus();
    void         SetFocus(unsigned int v);
    unsigned int GetTemperature();
    void         SetTemperature(unsigned int v);
    unsigned int GetWhiteShading();
    void         SetWhiteShading(unsigned int v);
    unsigned int GetTriggerDelay();
    void         SetTriggerDelay(unsigned int v);
    unsigned int GetFrameRate();
    void         SetFrameRate(unsigned int v);
    unsigned int GetTriggerMode();
    void         SetTriggerMode(unsigned int v);
    unsigned int GetZoom();
    void         SetZoom(unsigned int v);
    unsigned int GetPan();
    void         SetPan(unsigned int v);
    unsigned int GetTilt();
    void         SetTilt(unsigned int v);
    unsigned int GetOpticalFilter();
    void         SetOpticalFilter(unsigned int v);
    unsigned int GetCaptureSize();
    void         SetCaptureSize(unsigned int v);
    unsigned int GetCaptureQuality();
    void         SetCaptureQuality(unsigned int v);

    
};

#endif // !defined(AFX_CAMERADRIVERDC1394_H__054C33D8_C29D_47F2_AE31_6AE99A70E233__INCLUDED_)
