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
//////////////////////////    END_GPL    //////////////////////////////////
// MaintainAltitude.h: interface for the CMaintainAltitude class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEABEDTRACK_H__2E62CF87_B0B9_452E_95C1_F6542124A084__INCLUDED_)
#define AFX_SEABEDTRACK_H__2E62CF87_B0B9_452E_95C1_F6542124A084__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <MOOSGenLib/MOOSGenLib.h>
#include "MaintainDepth.h"

/**
 * FIXME: needs class description
 */
class CMaintainAltitude : public CMaintainDepth
{
  public:
    virtual bool SetParam(string sParam, string sVal);
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Run(CPathAction &DesiredAction);
    virtual bool GetRegistrations(STRING_LIST &List);
    virtual bool RegularMailDelivery(double dfTimeNow);

    CMaintainAltitude();
    virtual ~CMaintainAltitude();

  protected:
    virtual void GetChartNodeContents(STRING_LIST& contents) const;

    ControlledDOF m_AltitudeDOF;
    bool m_bAllowAltitudeChanges;

};

#endif // !defined(AFX_SEABEDTRACK_H__2E62CF87_B0B9_452E_95C1_F6542124A084__INCLUDED_)
