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
#include <wx/tokenzr.h>
using namespace std;

CSummaryPanel::CSummaryPanel(wxWindow* Parent, CMOOSCommClient* Comms) : CMOOSPanel(Parent, Comms)
{
}

CSummaryPanel::~CSummaryPanel()
{
	SUMMARY_MAP::iterator i;

	for(i=m_Rows.begin(); i!=m_Rows.end(); i++)
		delete i->second;

	m_Rows.clear();
}
bool CSummaryPanel::OnNewMail(MOOSMSG_LIST &NewMail)
{
	CMOOSMsg Msg;
	double dfNow = MOOSTime();
	SUMMARY_MAP::iterator i;

	for(i=m_Rows.begin(); i!=m_Rows.end(); i++)
	{
		wxTextAttr color(*wxBLACK); //(wxBLACK);

		if(m_MOOSComms->PeekMail(NewMail,i->first,Msg) && !Msg.IsSkewed(dfNow))
		{
			if(!Msg.IsSkewed(dfNow))
			{
				wxString sVal;
				sVal << Msg.GetDouble();
				i->second->SetValue(sVal);
			}
		}
//		i->second->SetStyle(0, i->second->GetLastPosition(),color);

	}


	return true;
}

void CSummaryPanel::GetSubscriptions(STRING_LIST& subs)
{
	SUMMARY_MAP::iterator i;

	for(i=m_Rows.begin(); i!=m_Rows.end(); i++)
		subs.push_back(i->first);
}

void CSummaryPanel::DoLayout()
{
	wxStaticBoxSizer* boxsizer = new wxStaticBoxSizer(wxVERTICAL,this,m_sLabel);
	m_flexgrid = new wxFlexGridSizer(m_nRows, 3, 5, 5);
	SetupRows();
	boxsizer->Add(m_flexgrid,wxSizerFlags(1).Expand().Right());
	m_flexgrid->AddGrowableCol(0,1);
	SetSizer(boxsizer);
}

void CSummaryPanel::CreateRow(wxString sLabel, std::string sVar, wxString sUnits)
{
	m_Rows[sVar] = new CSummaryRow(sLabel, sVar, sUnits, this);
}


CSummaryRow::CSummaryRow(wxString sLabel, std::string sVar, wxString sUnits, CSummaryPanel* Parent)
{
	m_Label = new wxStaticText(Parent, -1, sLabel);
	Parent->m_flexgrid->Add(m_Label, wxSizerFlags(1).Expand().Left());

	m_Value = new wxTextCtrl(Parent, -1, _T(""),
		wxDefaultPosition, wxDefaultSize,
		wxTE_READONLY | wxTE_RIGHT);
	Parent->m_flexgrid->Add(m_Value, wxSizerFlags().Right());

	m_Units = new wxStaticText(Parent, -1, sUnits);
	Parent->m_flexgrid->Add(m_Units, wxSizerFlags().Left());

	m_Timer.SetOwner(this);
	Connect(m_Timer.GetId(), wxEVT_TIMER,
		(wxObjectEventFunction) &CSummaryRow::OnTimeout );

	m_Value->Enable(false);

	m_nTimeout = 3000;

}

CSummaryRow::~CSummaryRow()
{
	delete m_Label;
	delete m_Value;
	delete m_Units;
}

void CSummaryRow::SetValue(wxString sVal)
{
	m_Value->SetValue(sVal);
	m_Value->Enable(true);
	m_Timer.Start(m_nTimeout, wxTIMER_ONE_SHOT);

}

void CSummaryRow::OnTimeout(wxCommandEvent event)
{
	m_Value->Enable(false);
}
