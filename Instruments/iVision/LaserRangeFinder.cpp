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

#include "LaserRangeFinder.h"


CLaserRangeFinder::CLaserRangeFinder() : CRecordableVisionProcessor()
{
    m_sName="LASER";
}

CLaserRangeFinder::~CLaserRangeFinder()
{
}

bool CLaserRangeFinder::Initialize(CvCapture* capture, CProcessConfigReader& config)
{
    return CRecordableVisionProcessor::Initialize(capture, config);
}

bool CLaserRangeFinder::ProcessFrame(IplImage* frame, MOOSMSG_LIST& Msgs, double dfGrabTime)
{
    //int nChannel = 0;
    // IplImage* SingleChannel = cvCreateImage(cvGetSize(frame), frame->depth, 1);
    // cv::extractImageCOI((cv::Mat*)frame,SingleChannel,nChannel);
    //cv::Mat SplitChannels = cv::cvarrToMat(frame);
    //int fromto[] = {0,0};
    //cv::mixChannels(frame,1,SingleChannel,1,fromto,1);
    
    IplImage* SafeCopy = cvCloneImage(frame);
    
    for(int y=0;y<SafeCopy->height;y++)
    {
        uchar* ptr=(uchar*)(SafeCopy->imageData+y*SafeCopy->widthStep);
        for(int x=0;x<SafeCopy->width; x++)
        {
            ptr[3*x+0] = 0;
            ptr[3*x+1] = 0;
        }
    }
    return CRecordableVisionProcessor::ProcessFrame(SafeCopy, Msgs, dfGrabTime);
    
    cvReleaseImage(&SafeCopy);

}

