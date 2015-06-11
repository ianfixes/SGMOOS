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
// MOOSVision.h: interface for the CMOOSVision class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(MOOS_MOOSVision_H)
#define MOOS_MOOSVision_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MOOSLIB/MOOSLib.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "RecordableVisionProcessor.h"

/**
 * FIXME: needs class description
 */
class CMOOSVision : public CMOOSApp
{
public:

    CMOOSVision();
    virtual ~CMOOSVision();

protected:

    bool Iterate();
    bool OnStartUp();
    virtual bool OnCommandMsg(CMOOSMsg Msg);
    void MakeProcessors(std::string sArgs);
    void ClearProcessors();

    CvCapture* m_Capture;

    PROC_LIST m_Viz;
    int m_nVideoInputOffset;
    bool m_bIsPS3Eye;
    int m_nCameraIndex;

};

#endif // !defined(MOOS_MOOSVision_H)

