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

CRemoteControlPanel::CRemoteControlPanel(wxWindow* Parent, CMOOSCommClient* Comms) : CMOOSPanel(Parent, Comms)
{
	m_dfSurgeForce = m_dfSwayForce = m_dfYawForce = 0.;
	m_dfForceStep = 5.0;

	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(wxVERTICAL,this,_T("Remote Control"));

	//Buttons
	wxBoxSizer* translational_sizer = new wxBoxSizer(wxVERTICAL);

	wxButton* ForwardButton = new wxButton(this,-1,_T("Surge Forward"));
	translational_sizer->Add(ForwardButton, wxSizerFlags().Center().Top());
	Connect( ForwardButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CRemoteControlPanel::OnForward );

	wxBoxSizer* translational_sizer_mid = new wxBoxSizer(wxHORIZONTAL);

	wxButton* SwayLeftButton = new wxButton(this,-1,_T("Sway Left"));
	translational_sizer_mid->Add(SwayLeftButton, wxSizerFlags().Left());
	Connect( SwayLeftButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CRemoteControlPanel::OnSwayLeft );

	wxButton* StopButton = new wxButton(this,-1,_T("Stop"));
	translational_sizer_mid->Add(StopButton, wxSizerFlags().Center());
	Connect(StopButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CRemoteControlPanel::OnStop );

	wxButton* SwayRightButton = new wxButton(this,-1,_T("Sway Right"));
	translational_sizer_mid->Add(SwayRightButton, wxSizerFlags().Right());
	Connect( SwayRightButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CRemoteControlPanel::OnSwayRight );

	translational_sizer->Add(translational_sizer_mid, wxSizerFlags());

	wxButton* BackwardButton = new wxButton(this,-1,_T("Surge Back"));
	translational_sizer->Add(BackwardButton, wxSizerFlags().Bottom().Center());
	Connect(BackwardButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CRemoteControlPanel::OnBackward );

	sizer->Add(translational_sizer, wxSizerFlags());

	wxStaticBoxSizer *rotational_sizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_T("Yaw"));

	wxButton* YawLeftButton = new wxButton(this,-1,_T("Turn Left"));
	rotational_sizer->Add(YawLeftButton, wxSizerFlags().Left());
	Connect(YawLeftButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CRemoteControlPanel::OnYawLeft );

	wxButton* YawRightButton = new wxButton(this,-1,_T("Turn Right"));
	rotational_sizer->Add(YawRightButton, wxSizerFlags().Right());
	Connect(YawRightButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CRemoteControlPanel::OnYawRight );

	sizer->Add(rotational_sizer, wxSizerFlags());

	wxStaticBoxSizer *heave_sizer = new wxStaticBoxSizer(wxVERTICAL,this,_T("Heave"));

	wxButton* HeaveUpButton = new wxButton(this,-1,_T("Heave Up"));
	heave_sizer->Add(HeaveUpButton, wxSizerFlags().Left());
	Connect(HeaveUpButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CRemoteControlPanel::OnHeaveUp );

	wxButton* HeaveDownButton = new wxButton(this,-1,_T("Heave Down"));
	heave_sizer->Add(HeaveDownButton, wxSizerFlags().Right());
	Connect(HeaveDownButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CRemoteControlPanel::OnHeaveDown );

	sizer->Add(heave_sizer, wxSizerFlags());


	SetSizer(sizer);

}

void CRemoteControlPanel::OnForward(wxCommandEvent event)
{
	m_dfSurgeForce += m_dfForceStep;
}

void CRemoteControlPanel::OnBackward(wxCommandEvent event)
{
	m_dfSurgeForce -= m_dfForceStep;
}

void CRemoteControlPanel::OnSwayRight(wxCommandEvent event)
{
	m_dfSwayForce += m_dfForceStep;
}

void CRemoteControlPanel::OnSwayLeft(wxCommandEvent event)
{
	m_dfSwayForce -= m_dfForceStep;
}

void CRemoteControlPanel::OnYawRight(wxCommandEvent event)
{
	m_dfYawForce -= m_dfForceStep;
}

void CRemoteControlPanel::OnYawLeft(wxCommandEvent event)
{
	m_dfYawForce += m_dfForceStep;
}

void CRemoteControlPanel::OnHeaveUp(wxCommandEvent event)
{
	m_dfHeaveForce += m_dfForceStep;
}

void CRemoteControlPanel::OnHeaveDown(wxCommandEvent event)
{
	m_dfHeaveForce -= m_dfForceStep;
}
void CRemoteControlPanel::OnStop(wxCommandEvent event)
{
	m_dfSurgeForce=0.0;
	m_dfSwayForce=0.0;
	m_dfYawForce=0.0;
	m_dfHeaveForce=0.0;
}

bool CRemoteControlPanel::Iterate()
{
	m_MOOSComms->Notify("MANUAL_SWAY", m_dfSwayForce);
	m_MOOSComms->Notify("MANUAL_SURGE", m_dfSurgeForce);
	m_MOOSComms->Notify("MANUAL_YAW", m_dfYawForce);
	m_MOOSComms->Notify("MANUAL_HEAVE", m_dfHeaveForce);

	return true;
}
/*
bool CRemoteControlPanel::OnNewMail(MOOSMSG_LIST &NewMail)
{
	CMOOSMsg Msg;
	double dfNow = MOOSTime();

	if(PeekMail(NewMail,"MISSIONS_LOADED",Msg))
	{

		m_NextMission->Clear();
		wxString sVal = wxString(Msg.GetString().c_str(), wxConvUTF8);
		wxArrayString missions = wxStringTokenize(sVal, wxT(","));

		m_NextMission->Append(missions);

	}

	return true;
}
*/
