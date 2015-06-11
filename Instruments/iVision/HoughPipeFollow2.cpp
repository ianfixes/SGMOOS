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

#include "HoughPipeFollow2.h"
#include "kht/kht.h"
using namespace std;

bool CHoughPipeFollow2::Initialize(CvCapture* capture, CProcessConfigReader& config)
{
   bool bOk = CHoughPipeFollow::Initialize(capture, config);
   m_copyBuffer = new unsigned char[m_FullFrameSize.width * m_FullFrameSize.height];
   m_Xoff = (double)(m_FullFrameSize.width / 2);
   m_Yoff = (double)(m_FullFrameSize.height / 2);
   return bOk;
}



CHoughPipeFollow2::CHoughPipeFollow2() : CHoughPipeFollow()
{
    m_sName = "PIPE2";
}


CHoughPipeFollow2::~CHoughPipeFollow2()
{
    delete m_copyBuffer;
}

void CHoughPipeFollow2::DoHoughTransform(IplImage* EdgeImage, CvMemStorage* storage, LINE_VEC& lines)
{
    double dfTheta, dfX1, dfY1, dfRho;

    // Execute the proposed Hough transform voting scheme and the peak detection procedure.
    lines_list_t KHTlines;

    memcpy( m_copyBuffer, EdgeImage->imageData, EdgeImage->width*EdgeImage->height );
    kht( KHTlines, m_copyBuffer, (size_t)EdgeImage->width, (size_t)EdgeImage->height, 10, 2.0, 0.5, 0.002, 2 );

    size_t numLines = KHTlines.size();
    
    for(int i = 0; i<numLines; i++)
    {

        // KHT assumes the origin is in the center of the image, thus we must translate origin for opencv to make sense of it.
        dfTheta = MOOSDeg2Rad(KHTlines[i].theta);
        //dfTheta = MOOSDeg2Rad(lines[i].theta > 90.0 ? lines[i].theta - 180.0 : lines[i].theta);
        dfX1 = m_Xoff + KHTlines[i].rho * cos(dfTheta);
        dfY1 = m_Yoff + KHTlines[i].rho * sin(dfTheta);
        dfRho = cos(atan2(dfY1, dfX1)-dfTheta) * sqrt( dfX1*dfX1 + dfY1*dfY1);
        lines.push_back(CMOOSCvLine(dfRho, dfTheta));

    }
}
