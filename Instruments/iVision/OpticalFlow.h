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

#if !defined(MOOS_OPTICAL_FLOW_H)
#define MOOS_OPTICAL_FLOW_H

#include "RecordableVisionProcessor.h"

class CTrackedFeature
{
public:
    CTrackedFeature(CvPoint2D32f prev, CvPoint2D32f curr, float fErr=0.0);
    void DrawArrow(IplImage* frame, CvScalar line_color=CV_RGB(0,255,0), double dfHeadSize=6.0, double dfArrowScale=1.0);

    double GetMagnitude() const {return m_dfMagnitude;}
    void Tag(bool bTag) {m_bTag=bTag;}
    
    CvPoint2D32f m_Curr,m_Prev;
    double m_dfDX, m_dfDY, m_dfAngle, m_dfMagnitude;

protected:
    float m_fError;
    bool m_bTag;


};
typedef std::list<CTrackedFeature> TRACKED_FEATURE_LIST;

class COpticalFlow : public CRecordableVisionProcessor
{
public:
    COpticalFlow();
    virtual ~COpticalFlow();

    virtual bool Initialize(CvCapture* capture,CProcessConfigReader& config);
    virtual bool ProcessFrame(IplImage* frame, MOOSMSG_LIST& Msgs, double dfGrabTime);

protected:
    void FilterResults(TRACKED_FEATURE_LIST& tfl);
    void InterpretResults(TRACKED_FEATURE_LIST& tfl,MOOSMSG_LIST& Msgs, double dfGrabTime);

    class LucasKanade
    {
    public:
        LucasKanade();
        LucasKanade(const CvSize& ImageSize, int nFeatures);
        ~LucasKanade();
        void Init(const CvSize& ImageSize, int nFeatures);
        void SetImage(IplImage* frame);
        TRACKED_FEATURE_LIST Calculate(COpticalFlow::LucasKanade* ref);

    protected:
        IplImage *m_GrayImage,*m_EigImage,*m_TempImage,*m_Pyramid;

        int m_nFeatures;
        CvPoint2D32f* m_FeatureArr;
        char* m_cFoundFeature;
        float* m_fFeatureError;

        bool m_bPyramidValid;
        CvTermCriteria m_TermCriteria;
    };

    LucasKanade *m_LK_ref, *m_LK_cur;
    IplImage *m_SafeCopy, *m_Gray;

    bool m_bFirstRun;
    
private:
    bool _FilterPredicate(const CTrackedFeature& tf);
    double m_dfMean,m_dfStdDev;
};

#endif //MOOS_OPTICAL_FLOW_H
