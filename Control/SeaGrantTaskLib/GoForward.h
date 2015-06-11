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
// MaintainHeadingTask.h: interface for the CGoForwardTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(GO_FORWARD_INCLUDED)
#define GO_FORWARD_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MOOSBehaviour.h"

/**
 * FIXME: needs class description
 */
class CGoForward : public virtual CSGMOOSBehaviour  
{
public:
    CGoForward();
    virtual ~CGoForward();

    virtual bool SetParam(string sParam, string sVal);
    virtual bool Run(CPathAction &DesiredAction);
    virtual bool RegularMailDelivery(double);
protected:
    virtual void GetChartNodeContents(STRING_LIST& contents) const;

    virtual bool Initialise();
    ControlledDOF m_SurgeDOF;

    bool m_bClosedLoop;  //Future Use
};

#endif // !defined(GO_FORWARD_INCLUDED)
