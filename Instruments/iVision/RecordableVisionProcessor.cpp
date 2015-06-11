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

#include "RecordableVisionProcessor.h"
#include <time.h>

using namespace std;

CRecordableVisionProcessor::CRecordableVisionProcessor()
{
    m_sName = "VIDEO";
    m_sOutputPath = ".";
    m_bRecordVideo = false;
    m_VideoWriter = NULL;
    m_nCodecCode = CV_FOURCC('I','4','2','0');
    m_dfAngularRes = 1.0;  // report 'in pixels' as default;
    m_nDownsampleFactor = 1; //default: no downsampling
}

CRecordableVisionProcessor::~CRecordableVisionProcessor()
{
    cvReleaseVideoWriter(&m_VideoWriter);
}

bool CRecordableVisionProcessor::Initialize(CvCapture* capture, CProcessConfigReader& config)
{

    config.GetConfigurationParam(m_sName+"_VideoOut",m_bRecordVideo);

    config.GetConfigurationParam("ANGULARRES",m_dfAngularRes);

    MOOSTrace("Camera angular resolution (radians/pixel): %f\n", m_dfAngularRes);

    double width  = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
    double height = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
    if (width==0.0 || height==0.0)
    {
        IplImage* frame = cvQueryFrame( capture );
        width = frame->width;
        height = frame->height;
    }

    m_FullFrameSize = cvSize(width,height);

    if(m_bRecordVideo)
    {
        //If we're using a video as our capture device, output its framerate,
        //otherwise, use apptick.
        double dfFrameRate  = cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
        if(dfFrameRate <= 0.0)
        {
            if(!config.GetConfigurationParam("APPTICK",dfFrameRate))
                dfFrameRate=5.0;  //'cause its gotta be somethin'
        }
        

        string sNewFourCC;
        if(config.GetConfigurationParam("FOURCC_CODEC",sNewFourCC))
        {
            if(sNewFourCC.length() == 4)
                m_nCodecCode = CV_FOURCC(sNewFourCC[0],sNewFourCC[1],sNewFourCC[2],sNewFourCC[3]);
            else
                MOOSTrace("Invalid FourCC Codec Code (%s). Must be 4 characters.  Ignoring\n", sNewFourCC.c_str());
        }


        string sPath=".";
        config.GetConfigurationParam("OUTPUT_PATH",sPath);
    
        struct tm *Now;
        time_t aclock = (time_t) MOOSTime();
        Now = localtime( &aclock );
    
        string sVideoOutName = MOOSFormat("%s/%04d%02d%02d_%02d%02d%02d_%s.avi",
            sPath.c_str(),
            Now->tm_year+1900, Now->tm_mon+1, Now->tm_mday,
            Now->tm_hour, Now->tm_min, Now->tm_sec,
            m_sName.c_str());

#ifdef CHEVRON_HACK
        m_VideoWriter = cvCreateVideoWriter(sVideoOutName.c_str(), m_nCodecCode, dfFrameRate, cvSize(width*2,height),true);
#else
        MOOSTrace("Output file: %s\n", sVideoOutName.c_str());
        MOOSTrace("Output fps:  %f\n", dfFrameRate);
        m_VideoWriter = cvCreateVideoWriter(sVideoOutName.c_str(), m_nCodecCode, dfFrameRate, m_FullFrameSize,true);
#endif
        m_nFrameNumber = 0;
        return m_VideoWriter != NULL;
    }
    return true;
}

bool CRecordableVisionProcessor::ProcessFrame(IplImage* frame, MOOSMSG_LIST& Msgs, double dfGrabTime)
{

    double dfStartTime = MOOSTime();
    if (m_bRecordVideo && m_VideoWriter != NULL)
    {

        //flip x & y axes if using I420 (for some reason)
        if(m_nCodecCode == CV_FOURCC('I','4','2','0'))
            cvFlip(frame,NULL,0);

        if(cvWriteFrame(m_VideoWriter, frame))
        {
            Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_FRAME", ++m_nFrameNumber, dfGrabTime));
            Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_OUTPUT_IO_TIME", MOOSTime()-dfStartTime, dfGrabTime));
            return true;
        }
        else
        {
            MOOSTrace("cvWriteFrame failed\n");
            return false;
        }
    }
    else
       return true;
}
