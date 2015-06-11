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
// ConstantHeadingTask.h: interface for the CConstantHeadingTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(MAINTAIN_PIPE_RELATIVE_HEADING_H)
#define MAINTAIN_PIPE_RELATIVE_HEADING_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MaintainHeading.h"
#include "MaintainPipeDistance.h"
/**
 * FIXME: needs class description
 */
class CMaintainPipeRelHeading : 
    public CMaintainHeading, 
    public CMaintainPipeDistance
{
public:
    CMaintainPipeRelHeading();
    virtual ~CMaintainPipeRelHeading();

    virtual bool SetParam(string sParam, string sVal);
    virtual bool OnNewMail(MOOSMSG_LIST &NewMail);
    virtual bool GetRegistrations(STRING_LIST &List);
    virtual bool RegularMailDelivery(double dfTimeNow);
    virtual bool Run(CPathAction &DesiredAction);


protected:

};

#endif // !defined(MAINTAIN_PIPE_RELATIVE_HEADING_H)
