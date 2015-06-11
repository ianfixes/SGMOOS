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
// SensorDataFilter.h: interface for the CSensorDataFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SENSORDATAFILTER_H__89B90C2D_7FF2_46D9_929B_86B80CBE58CF__INCLUDED_)
#define AFX_SENSORDATAFILTER_H__89B90C2D_7FF2_46D9_929B_86B80CBE58CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>

typedef std::list<double> DOUBLE_LIST;

/**
 * FIXME: needs class description
 */
class CSensorDataFilter  
{
public:
	int GetMaxDataSize();
	void SetDataSize(int nSize);
	void SetWindowSize(int nSize);
	int GetDataSize();
	int GetWindowSize();
	bool Filter(double &dfVal);

	CSensorDataFilter(int nDataSize, int nWindowSize);
	CSensorDataFilter();
	virtual ~CSensorDataFilter();

protected:
	void Initialise();
	virtual bool FilterData(double &dfVal);
	int m_nDataSize;
	int m_nWindowSize;
	DOUBLE_LIST m_Data;
};

#endif // !defined(AFX_SENSORDATAFILTER_H__89B90C2D_7FF2_46D9_929B_86B80CBE58CF__INCLUDED_)
