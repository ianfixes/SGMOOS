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
// FaceVerticalPipeTask.h: interface for the CFaceVerticalPipeTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(FACE_VERTICAL_PIPE_H)
#define FACE_VERTICAL_PIPE_H

#include "MOOSBehaviour.h"

/**
 * FIXME: needs class description
 */
class CFaceVerticalPipe : public virtual CSGMOOSBehaviour  
{
public:
    CFaceVerticalPipe();
    virtual ~CFaceVerticalPipe();

    virtual bool OnNewMail(MOOSMSG_LIST &NewMail);
    virtual bool Run(CPathAction &DesiredAction);
    virtual bool GetRegistrations(STRING_LIST &List);
    virtual bool RegularMailDelivery(double dfTimeNow);

protected:
    bool ShouldReport();
    bool m_bNewBearing;
    double m_dfBearing, m_dfTimeLastSet, m_dfStarveTime; 


};

#endif // !defined(FACE_VERTICAL_PIPE_H)
