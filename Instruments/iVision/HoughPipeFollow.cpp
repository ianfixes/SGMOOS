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

#include "HoughPipeFollow.h"
#include <time.h>
#include <algorithm>
using namespace std;


CHoughPipeFollow::CHoughPipeFollow() : CRecordableVisionProcessor()
{
    m_sName = "PIPE";
    m_bUseCanny = false;
    m_bOverlayLines = true;
    m_bOverlayEdges = true;
    m_bTargetDiameterSet = false;
    m_nThreshold = 90;

}

CHoughPipeFollow::~CHoughPipeFollow()
{
}

bool CHoughPipeFollow::Initialize(CvCapture* capture, CProcessConfigReader& config)
{
    double dfTemp;
    if(config.GetConfigurationParam("TARGETDIAMETER",dfTemp))
        SetTargetDiameter(dfTemp);
    

    config.GetConfigurationParam("THRESHOLD",m_nThreshold);

    bool bTemp;
    if(config.GetConfigurationParam("Overlay", bTemp))
    {
        m_bOverlayLines = bTemp;
        m_bOverlayEdges = bTemp;
    }
    

    bool ret = CRecordableVisionProcessor::Initialize(capture,config);
    return ret;

}

bool CHoughPipeFollow::ProcessFrame(IplImage* frame, MOOSMSG_LIST& Msgs, double dfGrabTime)
{

    double dfStartTime = MOOSTime();
    CvMemStorage* storage = cvCreateMemStorage(0);

    
    CvSize fullsize = cvGetSize(frame);
    CvSize downsampsize = cvSize(fullsize.width/m_nDownsampleFactor, fullsize.height/m_nDownsampleFactor);


    IplImage *downsample_frame = cvCreateImage( downsampsize, frame->depth, 3 );
    cvResize(frame, downsample_frame);
    //Convert to Grayscale
    IplImage *gray_frame = cvCreateImage( downsampsize, IPL_DEPTH_8U, 1 );
    cvCvtColor(downsample_frame, gray_frame, CV_BGR2GRAY );
    cvReleaseImage(&downsample_frame);

    //Edge Detection
    IplImage* edges = cvCreateImage(downsampsize, IPL_DEPTH_8U, 1 );
    if(m_bUseCanny)
        cvCanny( gray_frame, edges, 50, 200, 3 );
    else
    {
        CvSeq* contours = 0;
        cvThreshold(gray_frame, gray_frame, m_nThreshold, 255, CV_THRESH_BINARY_INV);

#ifdef CHEVRON_HACK
        //Chevron Video Hack
        int nHUDHeight = 96;
        cvSetImageROI(gray_frame, cvRect(0,0,downsampsize.width,nHUDHeight/m_nDownsampleFactor));
        cvSetZero(gray_frame);
        cvResetImageROI(gray_frame);
#endif

        cvFindContours(gray_frame,storage,&contours,sizeof(CvContour),CV_RETR_EXTERNAL);
        cvZero(edges);
        if(contours)
        {
            cvDrawContours(edges,contours,cvScalarAll(255),cvScalarAll(255),100);
        }
    
        // contours get drawn along the edges.  we don't want this, remove 'em.
        int H = 0;
#ifdef CHEVRON_HACK
        H = nHUDHeight/m_nDownsampleFactor;
#endif
        cvLine( edges, cvPoint(0,H), cvPoint(edges->width-1,H), cvScalar(0.0), 3, CV_AA, 0 );

        H = edges->height-1;
        cvLine( edges, cvPoint(0,H), cvPoint(edges->width-1,H), cvScalar(0.0), 3, CV_AA, 0 );
        cvLine( edges, cvPoint(0,0), cvPoint(0,H), cvScalar(0.0), 3, CV_AA, 0 );
        cvLine( edges, cvPoint(edges->width-1,0), cvPoint(edges->width-1,H), cvScalar(0.0), 3, CV_AA, 0 );

    
    }
    //Hough Transform
    LINE_VEC lines;
    DoHoughTransform(edges, storage, lines);
    LINE_VEC leftRight=FilterDistinctLines(lines, fullsize);

    //MOOSTrace("%d\n",leftRight.size());

    if(leftRight.size()>=2)
    {
        std::sort(leftRight.begin(), leftRight.end(), Left2Right);

        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_LEFT_RHO", leftRight[0].dfRho, dfGrabTime));
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_LEFT_THETA", leftRight[0].dfTheta, dfGrabTime));
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_RIGHT_RHO", leftRight[1].dfRho, dfGrabTime));
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_RIGHT_THETA", leftRight[1].dfTheta, dfGrabTime));

        //Average thetas to get our pipe's theta.
        //Shift so that it wraps around the horizontal case.
        double ThetaWrapped=0.0;
        for(int i=0; i<2; i++)
            ThetaWrapped += (leftRight[i].dfTheta > PI/2.0) ?
                leftRight[i].dfTheta-PI : 
                leftRight[i].dfTheta;

/*      Do we need to unwrap it again?
        if(ThetaWrapped < 0.0)
            ThetaWrapped += PI;
*/
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_THETA", ThetaWrapped , dfGrabTime));

        CvPoint topleft,topright,bottomleft,bottomright;
        CMOOSCvLine top(0.0,PI/2.0), bottom((double)(fullsize.height-1),PI/2.0);

        topleft = top.FindIntersection(leftRight[0]);
        topright = top.FindIntersection(leftRight[1]);
        bottomleft = bottom.FindIntersection(leftRight[0]);
        bottomright = bottom.FindIntersection(leftRight[1]);

        double topwidth = (topright.x - topleft.x) * cos(ThetaWrapped);
        double bottomwidth = (bottomright.x - bottomleft.x) * cos(ThetaWrapped);
        //Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_TOP_WIDTH", topwidth, dfGrabTime));
        //Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_BOTTOM_WIDTH", bottomwidth, dfGrabTime));

        double dfWidth = (topwidth+bottomwidth)/2.0;
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_WIDTH", dfWidth, dfGrabTime));

        double dfRange = GetRange(m_dfTargetDiameter, dfWidth, m_dfAngularRes);

        if(m_bTargetDiameterSet)
            Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_RANGE", dfRange, dfGrabTime));


        double dfHorizontalPos=(topright.x+topleft.x+bottomright.x+bottomleft.x)/4.0;
        Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_BEARING", (dfHorizontalPos-fullsize.width/2)*m_dfAngularRes, dfGrabTime));

        if(m_bRecordVideo  && m_bOverlayLines)
        {
            CvPoint pt1, pt2;
            leftRight[0].GetImageEdgePoints(pt1,pt2);
            cvLine(frame, pt1, pt2, CV_RGB(255,0,0), 3, CV_AA, 0 );
            leftRight[1].GetImageEdgePoints(pt1,pt2);
            cvLine(frame, pt1, pt2, CV_RGB(0,255,0), 3, CV_AA, 0 );
            
            // Draw a "+" on the "center" of the pipe;
            pt1.x=pt2.x=dfHorizontalPos;
            pt1.y=fullsize.height/2 + 10;
            pt2.y=fullsize.height/2 - 10;
            cvLine(frame, pt1, pt2, CV_RGB(255,255,0), 1, CV_AA, 0 );

            pt1.y=pt2.y=fullsize.height/2;
            pt1.x=dfHorizontalPos+10;
            pt2.x=dfHorizontalPos-10;
            cvLine(frame, pt1, pt2, CV_RGB(255,255,0), 1, CV_AA, 0 );
        }
    }
    cvReleaseMemStorage(&storage);
    cvReleaseImage(&gray_frame);
    
    Msgs.push_back(CMOOSMsg(MOOS_NOTIFY,m_sName+"_PROC_TIME", MOOSTime()-dfStartTime, dfGrabTime));

    if(m_bRecordVideo)
    {
        if(m_bOverlayEdges)
        {
            IplImage* edges2 = cvCreateImage(fullsize, IPL_DEPTH_8U, 1 );
            cvResize(edges,edges2);
            cvSet(frame, CV_RGB(255,255,255), edges2);
            cvReleaseImage(&edges2);

        }
        CRecordableVisionProcessor::ProcessFrame(frame, Msgs, dfGrabTime);

    }


    cvReleaseImage(&edges);
    return true;
}

void CHoughPipeFollow::DoHoughTransform(IplImage* EdgeImage, CvMemStorage* storage, LINE_VEC& lines)
{
    CvSeq* HoughLines = cvHoughLines2(EdgeImage, storage, CV_HOUGH_STANDARD, 1, CV_PI/360, 50/m_nDownsampleFactor, 50, 0 );
    
    for(int i=0; i<HoughLines->total; i++)
    {
        float* fTmp = (float*)cvGetSeqElem(HoughLines,i);
        lines.push_back(CMOOSCvLine(fTmp[0]*(double)m_nDownsampleFactor,fTmp[1]));
    }
}

LINE_VEC CHoughPipeFollow::FilterDistinctLines(const LINE_VEC& lines_in, const CvSize& imageSize)
{
    LINE_VEC out;
    for(LINE_VEC::const_iterator i=lines_in.begin(); i!=lines_in.end(); i++)
    {
        bool bOK = true;
        for(LINE_VEC::iterator j=out.begin(); j!=out.end(); j++)
        {
            CvPoint intersect = i->FindIntersection(*j);
            if((intersect.y < imageSize.height * 2 && intersect.y >= -imageSize.height) &&
               (intersect.x < imageSize.width  * 2 && intersect.x >= -imageSize.width))
            {
                bOK = false;
                break;
            }
        }
        if(bOK)
            out.push_back(*i);

        if(out.size() > 1)
            break;
    }

    return out;
}

void CHoughPipeFollow::SetTargetDiameter(double dfVal)
{
    m_dfTargetDiameter = dfVal;
    m_bTargetDiameterSet = true;
}

CMOOSCvLine::CMOOSCvLine(double rho, double theta)
{
    dfRho=rho;
    dfTheta = theta;
    //dfTheta = (theta>PI/2.0) ? theta-PI : theta;
}

void CMOOSCvLine::GetImageEdgePoints(CvPoint& pt1, CvPoint& pt2)
{
    double a = cos(dfTheta), b = sin(dfTheta);
    double x0 = a*dfRho, y0 = b*dfRho;
    pt1.x = cvRound(x0 + 5000*(-b));
    pt1.y = cvRound(y0 + 5000*(a));
    pt2.x = cvRound(x0 - 5000*(-b));
    pt2.y = cvRound(y0 - 5000*(a));
}

CvPoint CMOOSCvLine::FindIntersection(const CMOOSCvLine& other) const
{
    CvPoint intersection;

    double y = (this->dfRho/cos(this->dfTheta) - other.dfRho/cos(other.dfTheta))/
               (tan(this->dfTheta) - tan(other.dfTheta));

    double x = cvRound((dfRho-y*sin(dfTheta))/cos(dfTheta));

    intersection.x=cvRound(x);
    intersection.y=cvRound(y);
    return intersection;

}

bool Left2Right(const CMOOSCvLine &l1, const CMOOSCvLine &l2)
{
    CMOOSCvLine xAxis(0.0,PI/2.0);
    CvPoint l1Intercept = l1.FindIntersection(xAxis);
    CvPoint l2Intercept = l2.FindIntersection(xAxis);
    return l1Intercept.x < l2Intercept.x;
}

double GetRange(double dfPhysicalWidth, double dfPixelWidth, double dfRadsPerPixel)
{
    return dfPhysicalWidth / (2.0 * tan(dfPixelWidth * dfRadsPerPixel / 2.0));
}
