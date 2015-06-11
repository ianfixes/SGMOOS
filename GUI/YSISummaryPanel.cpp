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
#include "wxMOOSLib.h"

using namespace std;
CYSISummaryPanel::CYSISummaryPanel(wxWindow* Parent, CMOOSCommClient* Comms) : CSummaryPanel(Parent, Comms)
{
	m_sLabel = _T("Water Quality");
	m_nRows = 9;
	DoLayout();
}

void CYSISummaryPanel::SetupRows()
{
	CreateRow(_T("Depth"),	string("YSI_DEPTH"), _T("m"));
	CreateRow(_T("Temperature"),	string("YSI_TEMP"), _T("C"));
	CreateRow(_T("pH"),	string("YSI_PH"), _T(""));
	CreateRow(_T("Dissolved Oxygen"),		string("YSI_DO"), _T("mg/L"));
	CreateRow(_T("Dissolved Oxygen"),	string("YSI_DOSAT"), _T("%"));
	CreateRow(_T("Turbidity"),	string("YSI_TURBID"), _T("NTU"));
	CreateRow(_T("Salinity"),	string("YSI_SAL"), _T("ppt"));
	CreateRow(_T("Conductivity"),	string("YSI_COND"), _T("mS/cm"));
	CreateRow(_T("Specific Cond."),	string("YSI_SPCOND"), _T("mS/cm"));
}

