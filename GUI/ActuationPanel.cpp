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

CActuationPanel::CActuationPanel(wxWindow* Parent, CMOOSCommClient* Comms, std::string sPrefix, wxString sLabel) 
: CMOOSPanel(Parent, Comms)
{

	m_sPrefix = sPrefix;

	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(wxVERTICAL,this,sLabel);

    // Next mission
	wxBoxSizer* status_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* status_label = new wxStaticText(this, -1, _T("Status: "));
	status_sizer->Add(status_label, wxSizerFlags().Center());

	m_Status = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	status_sizer->Add(m_Status,wxSizerFlags(1).Expand());

	sizer->Add(status_sizer, wxSizerFlags().Expand());

	//Buttons
	wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton* EnableButton = new wxButton(this,-1,_T("Enable"));
	button_sizer->Add(EnableButton, wxSizerFlags(1).Expand());
	Connect( EnableButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CActuationPanel::OnEnableButtonClicked );

	wxButton* DisableButton = new wxButton(this,-1,_T("Disable"));
	button_sizer->Add(DisableButton, wxSizerFlags(1).Expand());
	Connect( DisableButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CActuationPanel::OnDisableButtonClicked );

	sizer->Add(button_sizer, wxSizerFlags().Expand());

	SetSizer(sizer);
	Center();

	m_Subscriptions.push_back(m_sPrefix +"_STATUS");

}


void CActuationPanel::OnEnableButtonClicked(wxCommandEvent event)
{
	SendEnableCommand(true);
}

void CActuationPanel::OnDisableButtonClicked(wxCommandEvent event)
{
	SendEnableCommand(false);
}

void CActuationPanel::SendEnableCommand(bool bEnable)
{
	string sVal = bEnable ? "TRUE" : "FALSE";
	wxString sPrompt = bEnable ? _T("Enable") : _T("Disable");

	m_MOOSComms->Notify(m_sPrefix+"_ENABLE", sVal);

}

bool CActuationPanel::OnNewMail(MOOSMSG_LIST &NewMail)
{
	CMOOSMsg Msg;
	double dfNow = MOOSTime();

	if(m_MOOSComms->PeekMail(NewMail,m_sPrefix+"_STATUS",Msg))
	{
		if(!Msg.IsSkewed(dfNow))
		{
			m_Status->SetValue(wxString(Msg.GetString().c_str(), wxConvUTF8));
		}
	}

	return true;
}

