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

#include "PhaseCorrNav.h"

using namespace std;


CPhaseCorrNav::CPhaseCorrNav() : CRecordableVisionProcessor()
{
    m_sName = "PHASE_CORR";

}

CPhaseCorrNav::~CPhaseCorrNav()
{
    fftw_destroy_plan( forward_plan );
    fftw_destroy_plan( reverse_plan );

    fftw_free(m_kSpaceCorr);
    fftw_free(m_Ref);
    cvReleaseImage(&m_PhaseCorrImage);

}

bool CPhaseCorrNav::Initialize(CvCapture* capture, CProcessConfigReader& config)
{
/*
    double dfTemp;
    if(config.GetConfigurationParam("TARGETDIAMETER",dfTemp))
        SetTargetDiameter(dfTemp);
*/
    bool ret = CRecordableVisionProcessor::Initialize(capture,config);

    int size = m_FullFrameSize.width * m_FullFrameSize.height;
    size = GetFFTSize();

    // plan the forward FFT.  We're not going to keep this data though.
    FFT_COMPLEX *tempimg = ( FFT_COMPLEX* )fftw_malloc( sizeof( FFT_COMPLEX ) * size );
    
    forward_plan = fftw_plan_dft_1d( size, tempimg, tempimg, FFTW_FORWARD, FFTW_ESTIMATE);

    fftw_free(tempimg);

    // allocate array & plan reverse transform
    m_kSpaceCorr  = ( FFT_COMPLEX* )fftw_malloc( sizeof( FFT_COMPLEX ) * size );

    reverse_plan = fftw_plan_dft_1d( size, m_kSpaceCorr, m_kSpaceCorr, FFTW_BACKWARD, FFTW_ESTIMATE);

    m_Ref = NULL;
    m_PhaseCorrImage = NULL;
    return ret;

}

bool CPhaseCorrNav::ProcessFrame(IplImage* frame, MOOSMSG_LIST& Msgs, double dfGrabTime)
{

    double dfStartTime = MOOSTime();
    int fft_size = GetFFTSize();

    //Convert to Grayscale
    IplImage *gray_frame = cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 1 );
    cvCvtColor(frame, gray_frame, CV_BGR2GRAY );
    uchar* frame_data = ( uchar* ) gray_frame->imageData;
    FFT_COMPLEX *newFrame  = ( FFT_COMPLEX* )fftw_malloc( sizeof( FFT_COMPLEX ) * fft_size );

    //Copy OpenCV frame into something FFTW can understand.
    for( int i = 0, k = 0 ; i < frame->height ; i++ ) 
    {
        for( int j = 0 ; j < frame->width ; j++, k++ ) 
        {
            newFrame[k][0] = ( double )frame_data[i * gray_frame->widthStep + j];
            newFrame[k][1] = 0.0;
        }
    }


    fftw_execute_dft(forward_plan, newFrame, newFrame);

    if(m_Ref!=NULL)
    {

        // obtain the cross power spectrum
        double tmp;
        for(int i = 0; i < fft_size ; i++ ) 
        {
            m_kSpaceCorr[i][0] = ( newFrame[i][0] * m_Ref[i][0] ) - ( newFrame[i][1] * ( -m_Ref[i][1] ) );
            m_kSpaceCorr[i][1] = ( newFrame[i][0] * ( -m_Ref[i][1] ) ) + ( newFrame[i][1] * m_Ref[i][0] );
        
            tmp = sqrt( pow( m_kSpaceCorr[i][0], 2.0 ) + pow( m_kSpaceCorr[i][1], 2.0 ) );
        
            m_kSpaceCorr[i][0] /= tmp;
            m_kSpaceCorr[i][1] /= tmp;
        }

        fftw_execute(reverse_plan);

        if(m_PhaseCorrImage == NULL)
            m_PhaseCorrImage = cvCreateImage( cvSize( frame->width, frame->height ), IPL_DEPTH_64F, 1 );

        double  *PhaseCorrData = ( double* )m_PhaseCorrImage->imageData;

        //Copy it back, normalize & shift
        for( int i = 0; i < m_PhaseCorrImage->height/2 ; i++ ) 
            for( int j = 0 ; j < m_PhaseCorrImage->width/2 ; j++ ) 
            {
                int loc1 = i*m_PhaseCorrImage->width+j; 
                int loc2=  (i+m_PhaseCorrImage->height/2)*m_PhaseCorrImage->width+j+m_PhaseCorrImage->width/2;
                PhaseCorrData[loc1] = m_kSpaceCorr[loc2][0] / ( double )fft_size;
                PhaseCorrData[loc2] = m_kSpaceCorr[loc1][0] / ( double )fft_size;

                loc1 = i*m_PhaseCorrImage->width+j+m_PhaseCorrImage->width/2; 
                loc2=  (i+m_PhaseCorrImage->height/2)*m_PhaseCorrImage->width+j;
                PhaseCorrData[loc1] = m_kSpaceCorr[loc2][0] / ( double )fft_size;
                PhaseCorrData[loc2] = m_kSpaceCorr[loc1][0] / ( double )fft_size;
                
            }

        // find the maximum value and its location
        CvPoint minloc, maxloc;
        double  minval, maxval;
        cvMinMaxLoc( m_PhaseCorrImage, &minval, &maxval, &minloc, &maxloc, 0 );
        
        //MOOSTrace("Max X: %d Max Y: %d Magnitude %f\n", maxloc.x, maxloc.y, maxval);
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_DX", maxloc.x-frame->width/2, dfGrabTime));
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_DY", maxloc.y-frame->height/2, dfGrabTime));
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_MAG", maxval, dfGrabTime));

        cvConvertScale(m_PhaseCorrImage,gray_frame,255.0/maxval);

        IplImage *temp_frame = cvCloneImage(frame);
        cvCvtColor(gray_frame, temp_frame, CV_GRAY2BGR );
        CRecordableVisionProcessor::ProcessFrame(frame, Msgs, dfGrabTime);
        cvReleaseImage(&temp_frame);

        // out with the old...
        fftw_free(m_Ref);
    }
    // in with the new.
    m_Ref=newFrame;

    cvReleaseImage(&gray_frame);

    Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_PROC_TIME", MOOSTime()-dfStartTime, dfGrabTime));
    return true;
}

int CPhaseCorrNav::GetFFTSize()
{
    return m_FullFrameSize.width * m_FullFrameSize.height;
}

