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

#if !defined(MOOS_HoughPipeFollow_H)
#define MOOS_HoughPipeFollow_H

#include <opencv/cv.h>
#include "RecordableVisionProcessor.h"
#include <MOOSLIB/MOOSLib.h>

class CMOOSCvLine
{
  public:
    CMOOSCvLine(double rho, double theta);
    ~CMOOSCvLine(){};
    void GetImageEdgePoints(CvPoint& pt1, CvPoint& pt2);
    CvPoint FindIntersection(const CMOOSCvLine& other) const;

    double dfRho,dfTheta;

};

bool Left2Right(const CMOOSCvLine &l1,const CMOOSCvLine &l2);

double GetRange(double dfPhysicalWidth, double dfPixelWidth, double dfRadsPerPixel);

typedef std::vector<CMOOSCvLine> LINE_VEC;

class CHoughPipeFollow: public CRecordableVisionProcessor
{
  public:
    CHoughPipeFollow();
    virtual ~CHoughPipeFollow();

    virtual bool Initialize(CvCapture* capture,CProcessConfigReader& config);
    virtual bool ProcessFrame(IplImage* frame, MOOSMSG_LIST& Msgs, double dfGrabTime);

  protected:

    virtual void DoHoughTransform(IplImage* EdgeImage, CvMemStorage* storage, LINE_VEC& lines);
    LINE_VEC FilterDistinctLines(const LINE_VEC& lines_in, const CvSize& imageSize);
    void SetTargetDiameter(double dfVal);

    bool m_bUseCanny;
    bool m_bOverlayLines, m_bOverlayEdges;

    double m_dfTargetDiameter;
    bool m_bTargetDiameterSet;

    int m_nThreshold;
};

#endif //MOOS_HoughPipeFollow_H
