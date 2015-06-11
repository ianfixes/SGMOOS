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

#include "PhaseCorrRotScale.h"

using namespace std;


CPhaseCorrRotScale::CPhaseCorrRotScale() : CPhaseCorrNav()
{
    m_sName = "ROTATION_SCALE";

    m_nRotationalRes = 1024;
    m_nScaleRes = 512;
}

CPhaseCorrRotScale::~CPhaseCorrRotScale()
{

    cvReleaseImage(&m_PolarImage);

}

bool CPhaseCorrRotScale::Initialize(CvCapture* capture, CProcessConfigReader& config)
{

    m_PolarImage = cvCreateImage( cvSize(m_nScaleRes,m_nRotationalRes), IPL_DEPTH_8U, 3);

    return CPhaseCorrNav::Initialize(capture,config);

}

bool CPhaseCorrRotScale::ProcessFrame(IplImage* frame, MOOSMSG_LIST& Msgs, double dfGrabTime)
{
    double min_r = (frame->width>frame->height) ? frame->height/2 : frame->width/2;

    double scale = m_nScaleRes/log(min_r);

    cvLogPolar( frame, m_PolarImage, cvPoint2D32f(frame->width/2,frame->height/2), scale, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );

    return CPhaseCorrNav::ProcessFrame(m_PolarImage, Msgs, dfGrabTime);

}

int CPhaseCorrRotScale::GetFFTSize()
{
    return m_nRotationalRes * m_nScaleRes;
}

