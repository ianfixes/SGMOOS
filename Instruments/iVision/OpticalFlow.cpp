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

#include "OpticalFlow.h"

using namespace std;
struct _Filt
{
    _Filt( double i ) : n(i) {}
    bool operator () ( const CTrackedFeature & c )
    {
        return (c.GetMagnitude() > n);
    }
    double n;
};

COpticalFlow::COpticalFlow() : CRecordableVisionProcessor()
{
    m_sName = "OPTICAL_FLOW";
    m_bFirstRun = true;
}

COpticalFlow::~COpticalFlow()
{
    cvReleaseImage(&m_SafeCopy);
    cvReleaseImage(&m_Gray);
    delete m_LK_cur;
    delete m_LK_ref;
}

bool COpticalFlow::Initialize(CvCapture* capture, CProcessConfigReader& config)
{

    bool bReturn = CRecordableVisionProcessor::Initialize(capture,config);
    
    int nFeatures = 400;
    config.GetConfigurationParam("OpticalFlowFeatures",nFeatures);
    
    m_LK_ref = new LucasKanade(m_FullFrameSize, nFeatures);
    m_LK_cur = new LucasKanade(m_FullFrameSize, nFeatures);
    m_Gray = cvCreateImage( m_FullFrameSize, IPL_DEPTH_8U, 1);
    m_SafeCopy = NULL;
    return bReturn;
}

bool COpticalFlow::ProcessFrame(IplImage* frame, MOOSMSG_LIST& Msgs, double dfGrabTime)
{
    double dfStartTime = MOOSTime();
    cvCvtColor(frame, m_Gray, CV_BGR2GRAY );

    m_LK_ref->SetImage(m_Gray);

    if(m_bFirstRun)
    {
        m_bFirstRun = false;
    }
    else
    {
        TRACKED_FEATURE_LIST TFL = m_LK_cur->Calculate(m_LK_ref);
        /*
        //Find the maximum error & post it.
        double dfMaxErr = 0.0;
        TRACKED_FEATURE_LIST::iterator i;
        for(i=TFL.begin(); i!=TFL.end(); i++)
            if(i->m_fError > dfMaxErr)
                dfMaxErr = i->m_fError;
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY, m_sName+"_MAX_ERROR",dfMaxErr, dfGrabTime));
        */
        FilterResults(TFL);
        
        //Post the # of tracked features.
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY, m_sName+"_N_FEATURES_TRACKED",TFL.size(), dfGrabTime));
        
        InterpretResults(TFL,Msgs,dfGrabTime);
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY, m_sName+"_PROC_TIME", MOOSTime()-dfStartTime, dfGrabTime));

        if(m_bRecordVideo)
        {
            if(m_SafeCopy == NULL) m_SafeCopy=cvCloneImage(frame);
            else cvCopy(frame, m_SafeCopy);
            
            TRACKED_FEATURE_LIST::iterator i;
            for(i=TFL.begin(); i!=TFL.end(); i++)
                i->DrawArrow(m_SafeCopy);
            
            CRecordableVisionProcessor::ProcessFrame(m_SafeCopy,Msgs,dfGrabTime);
            
        }
    }
    swap(m_LK_cur,m_LK_ref);
    
    return true;
}

void COpticalFlow::FilterResults(TRACKED_FEATURE_LIST& tfl)
{
    m_dfMean = 0.0;
    TRACKED_FEATURE_LIST::iterator i;
    
    //Calculate Mean
    for(i=tfl.begin(); i!=tfl.end(); i++)
        m_dfMean += i->GetMagnitude();
    m_dfMean /= (double)tfl.size();
    
    double m_dfStdDev = 0.0;
    //Calculate Std Deviation
    for(i=tfl.begin(); i!=tfl.end(); i++)
        m_dfStdDev += pow(i->GetMagnitude()-m_dfMean, 2.0);
    m_dfStdDev = sqrt(m_dfStdDev/(double)tfl.size());

    double dfThresh=m_dfMean+2.0*m_dfStdDev;
    tfl.remove_if (_Filt(dfThresh));

}

bool COpticalFlow::_FilterPredicate(const CTrackedFeature& tf)
{
    return tf.GetMagnitude() > m_dfMean + 2*m_dfStdDev;
}
void COpticalFlow::InterpretResults(TRACKED_FEATURE_LIST& tfl,MOOSMSG_LIST& Msgs, double dfGrabTime)
{
    TRACKED_FEATURE_LIST::iterator i;

    CvPoint2D32f origin;
    origin.x=(float)m_FullFrameSize.width/2.0;
    origin.y=(float)m_FullFrameSize.height/2.0;
    
    double dfDx,dfDy,dfDz,dfDScale,dfDTheta=0.0;
    for(i=tfl.begin(); i!=tfl.end(); i++)
    {
        // X/Y translational component
        dfDx += i->m_dfDX;
        dfDy += i->m_dfDY;
        
        CTrackedFeature toPrev(origin,i->m_Prev);
        CTrackedFeature toCurr(origin,i->m_Curr);
        
        // Rotational component (about center)
        dfDTheta += MOOS_ANGLE_WRAP(toCurr.m_dfAngle - toPrev.m_dfAngle);
        
        // Radial component
        //dfDr += toCurr.GetMagnitude() - toPrev.GetMagnitude();
        dfDz += tan(m_dfAngularRes*toPrev.GetMagnitude()) /
                    tan(m_dfAngularRes*toCurr.GetMagnitude()) - 1;
                    
        dfDScale += (toCurr.GetMagnitude()-toPrev.GetMagnitude()) / toPrev.GetMagnitude();
    }
    
    Msgs.push_back(CMOOSMsg(MOOS_NOTIFY, m_sName+"_DX",dfDx/(double)tfl.size(), dfGrabTime));
    Msgs.push_back(CMOOSMsg(MOOS_NOTIFY, m_sName+"_DY",dfDy/(double)tfl.size(), dfGrabTime));
    Msgs.push_back(CMOOSMsg(MOOS_NOTIFY, m_sName+"_DZ",dfDz/(double)tfl.size(), dfGrabTime));
    Msgs.push_back(CMOOSMsg(MOOS_NOTIFY, m_sName+"_DSCALE",dfDScale/(double)tfl.size(), dfGrabTime));
    Msgs.push_back(CMOOSMsg(MOOS_NOTIFY, m_sName+"_DTHETA",MOOSRad2Deg(dfDTheta/(double)tfl.size()), dfGrabTime));

}

COpticalFlow::LucasKanade::LucasKanade()
{
}

COpticalFlow::LucasKanade::LucasKanade(const CvSize& ImageSize, int nFeatures)
{
    //self.LucasKanade();
    Init(ImageSize,nFeatures);
}

COpticalFlow::LucasKanade::~LucasKanade()
{
    cvReleaseImage(&m_EigImage);
    cvReleaseImage(&m_TempImage);
    cvReleaseImage(&m_Pyramid);
    cvReleaseImage(&m_GrayImage);

    delete [] m_FeatureArr;
    delete [] m_cFoundFeature;
    delete [] m_fFeatureError;
}

void COpticalFlow::LucasKanade::Init(const CvSize& ImageSize, int nFeatures)
{
  
    m_TermCriteria = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 );

    m_GrayImage = cvCreateImage( ImageSize, IPL_DEPTH_8U, 1 );
    m_EigImage  = cvCreateImage( ImageSize, IPL_DEPTH_32F, 1 );
    m_TempImage = cvCreateImage( ImageSize, IPL_DEPTH_32F, 1 );

    CvSize pyrsize = cvSize(ImageSize.width+8, ImageSize.height/3);
    m_Pyramid = cvCreateImage( pyrsize, IPL_DEPTH_32F, 1 );

    m_nFeatures = nFeatures;
    m_FeatureArr = new CvPoint2D32f[nFeatures];
    m_cFoundFeature = new char[nFeatures];
    m_fFeatureError = new float[nFeatures];

}

void COpticalFlow::LucasKanade::SetImage(IplImage* frame)
{
    cvCopy(frame,m_GrayImage);
    cvGoodFeaturesToTrack(m_GrayImage, m_EigImage, m_TempImage, m_FeatureArr, &m_nFeatures, .01, .01, NULL);

    m_bPyramidValid=false;
}

TRACKED_FEATURE_LIST COpticalFlow::LucasKanade::Calculate(LucasKanade* ref)
{
    int nFlags = ref->m_bPyramidValid ? CV_LKFLOW_PYR_A_READY : 0;

    cvCalcOpticalFlowPyrLK(ref->m_GrayImage, this->m_GrayImage,
                           ref->m_Pyramid, this->m_Pyramid, 
                           ref->m_FeatureArr, this->m_FeatureArr,
                           this->m_nFeatures, cvSize(3,3), 5, 
                           m_cFoundFeature, m_fFeatureError, m_TermCriteria, nFlags);
    m_bPyramidValid = true;
    
    TRACKED_FEATURE_LIST tfl;
    for(int i=0; i<m_nFeatures; i++)
    {
        if(m_cFoundFeature[i])
        {
            tfl.push_back(CTrackedFeature(ref->m_FeatureArr[i], m_FeatureArr[i],m_fFeatureError[i]));
        }
    }
    return tfl;
}

CTrackedFeature::CTrackedFeature(CvPoint2D32f prev, CvPoint2D32f curr, float fErr) : m_Curr(curr), m_Prev(prev), m_fError(fErr)
{
    m_dfDX = m_Curr.x - m_Prev.x;
    m_dfDY = m_Curr.y - m_Prev.y;
    m_dfAngle = atan2( m_dfDY, m_dfDX );
    m_dfMagnitude = sqrt(m_dfDX*m_dfDX + m_dfDY*m_dfDY);
    m_bTag=false;
}

void CTrackedFeature::DrawArrow(IplImage* frame, CvScalar line_color, double dfHeadSize, double dfArrowScale)
{
    int line_thickness = 1;
    CvPoint p = cvPoint(cvRound(m_Prev.x),cvRound(m_Prev.y));
    CvPoint q;// = cvPoint(cvRound(m_Curr.x),cvRound(m_Curr.y));

    if(m_bTag)
        line_color = CV_RGB(255,0,0);
    
    /* Here we lengthen the arrow by a user-defined factor. */
    q.x = (int) (p.x + dfArrowScale * m_dfMagnitude * cos(m_dfAngle));
    q.y = (int) (p.y + dfArrowScale * m_dfMagnitude * sin(m_dfAngle));

    /* Now we draw the main line of the arrow. */
    /* "frame" is the frame to draw on.
    * "p" is the point where the line begins.
    * "q" is the point where the line stops.
    * "CV_AA" means antialiased drawing.
    * "0" means no fractional bits in the center cooridinate or radius.
    */
    cvLine( frame, p, q, line_color, line_thickness, CV_AA, 0 );
    
    /* Now draw the tips of the arrow.  I do some scaling so that the
    * tips look proportional to the main line of the arrow.
    */
    p.x = (int) (q.x - dfHeadSize * cos(m_dfAngle + PI / 4));
    p.y = (int) (q.y - dfHeadSize * sin(m_dfAngle + PI / 4));
    cvLine( frame, p, q, line_color, line_thickness, CV_AA, 0 );
    
    p.x = (int) (q.x - dfHeadSize * cos(m_dfAngle - PI / 4));
    p.y = (int) (q.y - dfHeadSize * sin(m_dfAngle - PI / 4));
    cvLine( frame, p, q, line_color, line_thickness, CV_AA, 0 );

    
}
