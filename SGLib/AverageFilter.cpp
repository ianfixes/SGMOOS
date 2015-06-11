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


// AverageFilter.cpp: implementation of the CAverageFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "AverageFilter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAverageFilter::CAverageFilter()
{

}

CAverageFilter::~CAverageFilter()
{

}

bool CAverageFilter::FilterData(double &dfVal)
{
	double dfTotal = 0;
	DOUBLE_LIST::iterator p;
		
	//average over the window size
	if(GetWindowSize() != GetDataSize())
	{
		int nCnt = 0;
		//inherently start at last value we added
		for( p = m_Data.begin(); p != m_Data.end(); p++)
		{
			//only interested in traversing this deep
			if( ++nCnt > GetWindowSize() )
				break;
			dfTotal += *p;
		}
		
		//only interested in the window's view of the data
		dfTotal /= GetWindowSize();
	}
	else
	{
		for(p = m_Data.begin(); p!= m_Data.end(); p++)
		{
			dfTotal += *p;
		}

		//divide by total in store
		dfTotal /= GetDataSize();
		
	}

	//keep track of the filtered value
	dfVal = dfTotal;
	return true;
}

CAverageFilter::CAverageFilter(int nDataSize, int nWindowSize):CSensorDataFilter(nDataSize, nWindowSize)
{

}
