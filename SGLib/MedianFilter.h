/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Rob Damus & others at MIT Sea Grant.
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
// MedianFilter.h: interface for the CMedianFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEDIANFILTER_H__0F1032B2_D244_46C3_BE46_1A6F946D7873__INCLUDED_)
#define AFX_MEDIANFILTER_H__0F1032B2_D244_46C3_BE46_1A6F946D7873__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SensorDataFilter.h"

/**
 * FIXME: needs class description
 */
class CMedianFilter : public CSensorDataFilter 
{
public:
	double GetMaxDelta();
	void SetMaxDelta(double dfDelta);
	CMedianFilter();
	CMedianFilter(int nDataSize, int nWindowSize);
	virtual ~CMedianFilter();
protected:
	double m_dfMaxDelta;
	bool FilterData(double &dfVal);
};

#endif // !defined(AFX_MEDIANFILTER_H__0F1032B2_D244_46C3_BE46_1A6F946D7873__INCLUDED_)
