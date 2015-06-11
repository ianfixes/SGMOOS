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
// MedianFilter.cpp: implementation of the CMedianFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "MedianFilter.h"
#include <vector>
#include <algorithm>
#include <math.h>

using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMedianFilter::CMedianFilter()
{
	m_dfMaxDelta = 5.0;
}

CMedianFilter::CMedianFilter(int nDataSize, int nWindowSize):CSensorDataFilter(nDataSize, nWindowSize)
{
	m_dfMaxDelta = 5.0;
}

CMedianFilter::~CMedianFilter()
{

}

bool CMedianFilter::FilterData(double &dfVal)
{
	double dfMedian = 0;
	//vector to copy store
    vector<double> LocalVec;

	//only interested in the data over the window size
	if(GetWindowSize() != GetDataSize())
	{
		int nCnt = 0;
		for(DOUBLE_LIST::iterator p = m_Data.begin(); p != m_Data.end(); p++)
		{
			if( ++nCnt > GetWindowSize() )
				break;
			LocalVec.push_back(*p);
		}
		//sort
		sort(LocalVec.begin(), LocalVec.end());
		
		//note if window size is even, then median is avg of middle
		int nIdx = GetWindowSize() / 2;
		if( (GetWindowSize() % 2) == 0 )
		{
			double dfVal = LocalVec[nIdx];
			//base case is nIdx = 1;
			dfVal += LocalVec[nIdx - 1];

			dfMedian = dfVal / 2;
		}
		else
		{
			dfMedian = LocalVec[nIdx];
		}
	}
	else
	{
		for(DOUBLE_LIST::iterator p = m_Data.begin(); p != m_Data.end(); p++)
		{
			LocalVec.push_back(*p);
		}
		//sort
		sort(LocalVec.begin(), LocalVec.end());

		int nIdx = GetDataSize() / 2;
		if( (GetDataSize() % 2) == 0 )
		{
			double dfVal = LocalVec[nIdx];
			//base case is nIdx = 1;
			dfVal += LocalVec[nIdx - 1];

			dfMedian = dfVal / 2;
		}
		else
		{
			dfMedian = LocalVec[nIdx];
		}
	}
	
    bool bAccept = fabs(dfVal - dfMedian) < GetMaxDelta();
    
    if(bAccept)
    {
		//keep track of the filtered value
		dfVal = dfMedian;
	}

	return bAccept;
}


void CMedianFilter::SetMaxDelta(double dfDelta)
{
	this->m_dfMaxDelta = dfDelta;
}

double CMedianFilter::GetMaxDelta()
{
	return m_dfMaxDelta;
}
