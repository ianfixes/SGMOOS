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
// SensorDataFilter.cpp: implementation of the CSensorDataFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "SensorDataFilter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define MAX_DATA_SIZE 1024
#define DEFAULT_DATA_SIZE 10 //1 seconds of data @ 10Hz
#define DEFAULT_WINDOW_SIZE 10 //1 second of data @ DEFAULT

CSensorDataFilter::CSensorDataFilter()
{
	Initialise();
}

CSensorDataFilter::~CSensorDataFilter()
{

}

bool CSensorDataFilter::Filter(double &dfVal)
{
	//add the value to our store of data
	//like a stack, we add to the top
	m_Data.push_front(dfVal);

	//do we have enough data to scan the window?
	if( m_Data.size() < GetWindowSize() )
		return false;

	//mind the max size
	if( m_Data.size() > GetDataSize() )
		m_Data.pop_back();


	return FilterData(dfVal);
}

/**
*The window and size of our data store can be different, but are by default the 
*same size.  However this constructor lets you vary the window and data size
*so use cautiously.  You will not be able to make a window size that is larger
*than the data store that backs this filter (list<double> m_Data) - instead the 
*window size is adjusted to be the size of the store
*/
CSensorDataFilter::CSensorDataFilter(int nDataSize, int nWindowSize)
{
	Initialise();

	if( (nDataSize > 0) && (nWindowSize > 0) )
	{
		if( (nDataSize <= MAX_DATA_SIZE) )
		{
			m_nDataSize = nDataSize;
			
			if( (nWindowSize > m_nDataSize) )
			{
				m_nWindowSize = m_nDataSize;
			}
			else
			{
				m_nWindowSize = nWindowSize;
			}
			
		}
	}	
}

int CSensorDataFilter::GetWindowSize()
{
	return m_nWindowSize;
}

int CSensorDataFilter::GetDataSize()
{
	return m_nDataSize;
}

void CSensorDataFilter::SetWindowSize(int nSize)
{
	m_nWindowSize = nSize;
}

void CSensorDataFilter::SetDataSize(int nSize)
{
	m_nDataSize = nSize;
}

bool CSensorDataFilter::FilterData(double &dfVal)
{
	//base class
	return true;
}

void CSensorDataFilter::Initialise()
{
	this->m_nDataSize = DEFAULT_DATA_SIZE;
	this->m_nWindowSize = DEFAULT_WINDOW_SIZE;
}

int CSensorDataFilter::GetMaxDataSize()
{
	return MAX_DATA_SIZE;
}
