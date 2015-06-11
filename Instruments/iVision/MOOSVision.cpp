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
//
// MOOSVision.cpp: implementation of the CMOOSVision class.
//
//////////////////////////////////////////////////////////////////////

#include "MOOSVision.h"
#include "HoughPipeFollow2.h"
#include "PhaseCorrNav.h"
#include "PhaseCorrRotScale.h"
#include "OpticalFlow.h"
#include "LaserRangeFinder.h"
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSVision::CMOOSVision()
{
    m_bCommandMessageFiltering=true;
    m_nVideoInputOffset = 0;
    m_bIsPS3Eye = false;
    m_nCameraIndex = CV_CAP_ANY;
}

CMOOSVision::~CMOOSVision()
{
    MOOSTrace("Releasing Camera\n");
    cvReleaseCapture( &m_Capture );
    ClearProcessors();
}

bool CMOOSVision::OnStartUp()
{
    //CMOOSApp::OnStartUp();

    m_MissionReader.GetConfigurationParam("CAMERAINDEX",m_nCameraIndex);
    m_MissionReader.GetConfigurationParam("PS3EYE",m_bIsPS3Eye);

    string sTemp;
    if(m_MissionReader.GetConfigurationParam("VIDEOINPUTFILE",sTemp))
    {
        m_Capture = cvCaptureFromFile( sTemp.c_str() );
        if (m_Capture)
        {
            MOOSTrace("using VIDEOINPUTFILE of %s\n", sTemp.c_str());
        }
        else
        {
            MOOSTrace("OpenCV rejected VIDEOINPUTFILE of %s\n", sTemp.c_str());
        }
    }
    else
    {
        MOOSTrace("Capturing from camera\n");
        m_Capture = cvCaptureFromCAM( m_nCameraIndex );
    }
    if(m_Capture == NULL)
        return false;



    return true;
}


bool CMOOSVision::OnCommandMsg(CMOOSMsg Msg)
{

    if(!Msg.IsSkewed(MOOSTime()))
    {
        string sArgs = Msg.m_sVal;
        string sCmd = MOOSChomp(sArgs, ":");
        if(MOOSStrCmp(sCmd, "START"))
        {
            MOOSTrace("Starting...\n");

            //It's here instead of above because we want it reread changes every command.
            if(!m_MissionReader.GetConfigurationParam("VIDEOINPUTOFFSET",m_nVideoInputOffset))
                m_nVideoInputOffset = 0;

            //since we're intitalizing, set video input to beginning.
            //has no effect if using camera as input
            cvSetCaptureProperty(m_Capture, CV_CAP_PROP_POS_FRAMES, (double)m_nVideoInputOffset);

            MakeProcessors(sArgs);
            MOOSTrace("%d vision processors initialized\n", m_Viz.size());

            if(m_Viz.size()<1)
            {
                MOOSTrace("No processors initialized.  Not starting\n");
                return false;
            }
        }
        else if (MOOSStrCmp(sCmd, "STOP"))
        {
            MOOSTrace("Stopping...\n");
            ClearProcessors();
            return true;
        }
    }
    else
        MOOSTrace("Skewed Command\n");

    return true;
    
}


bool CMOOSVision::Iterate()
{

    if(m_Viz.size()>0)
    {
        //Grab a frame
        double dfPreGrabTime = MOOSTime();

#ifdef CHEVRON_HACK
        IplImage* narrowframe = cvQueryFrame( m_Capture );

        //hack to double width of DVD input;
        CvSize size= cvGetSize(narrowframe);
        size.width *=2;
        IplImage* frame = cvCreateImage( size, narrowframe->depth, 3 );
        cvResize(narrowframe, frame);
#else

        // Workaround for PS3 Eye.
        if(m_bIsPS3Eye)
            cvGrabFrame(m_Capture);
        
        IplImage* frame = cvQueryFrame( m_Capture );
#endif

        if(frame!=NULL) 
        {
            MOOSMSG_LIST Messages;
            //double dfPostGrabTime = MOOSTime();
            Messages.push_back(CMOOSMsg(MOOS_NOTIFY,GetAppName()+"_INPUT_IO_TIME", MOOSTime()-dfPreGrabTime, dfPreGrabTime));

            for(PROC_LIST::iterator i=m_Viz.begin(); i!=m_Viz.end(); i++)
                (*i)->ProcessFrame(frame, Messages, dfPreGrabTime);

            //MOOSTrace("Query took %f seconds\n", MOOSTime()-dfPreGrabTime);
            MOOSMSG_LIST::iterator m;
            for(m=Messages.begin();m!=Messages.end();m++)
                m_Comms.Post(*m);

        }
        else
            ClearProcessors();
#ifdef CHEVRON_HACK
        cvReleaseImage(&frame);
#endif
    }
    return true;
}

void CMOOSVision::MakeProcessors(string sArgs)
{

    while(!sArgs.empty())
    {
        string sTemp = MOOSChomp(sArgs, ",");
    
        CRecordableVisionProcessor* newViz;
    
        if(MOOSStrCmp(sTemp,"VIDEO"))
            newViz = new CRecordableVisionProcessor();
        else if(MOOSStrCmp(sTemp,"PIPE"))
            newViz = new CHoughPipeFollow();
        else if(MOOSStrCmp(sTemp,"PIPE2"))
            newViz = new CHoughPipeFollow2();
        else if(MOOSStrCmp(sTemp,"PHASECORR"))
            newViz = new CPhaseCorrNav();        
        else if(MOOSStrCmp(sTemp,"ROTSCALE"))
            newViz = new CPhaseCorrRotScale();
        else if(MOOSStrCmp(sTemp,"OPTICALFLOW"))
            newViz = new COpticalFlow();
        else if(MOOSStrCmp(sTemp,"Laser"))
            newViz = new CLaserRangeFinder();
        else
        {
            MOOSTrace("Unknown Processor Type: %s\n", sTemp.c_str());
            continue;
        }
    
        if(newViz->Initialize(m_Capture, m_MissionReader))
            m_Viz.push_back(newViz);
        else
        {
            MOOSTrace("Failed To Initialize: %s\n", sTemp.c_str());
            delete newViz;
        }
    }
}

void CMOOSVision::ClearProcessors()
{
    for(PROC_LIST::iterator i=m_Viz.begin(); i!=m_Viz.end(); i++)
        delete(*i);
    m_Viz.clear();

    MOOSTrace("Clearing Processors\n");
}

