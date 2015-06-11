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
#include "wx/tokenzr.h"

CSGHelmPanel::CSGHelmPanel(wxWindow* Parent, CMOOSCommClient* Comms) : CMOOSPanel(Parent, Comms)
{
	m_bCommandMySQL = true;

	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(wxVERTICAL,this,_T("Mission Control"));

    // Next mission
	wxBoxSizer* next_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* next_mission_label = new wxStaticText(this, -1, _T("Choose The Next Mission"));
	next_sizer->Add(next_mission_label, wxSizerFlags().Border());

	m_NextMission = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize); //wxSize(200,30));
	Connect( m_NextMission->GetId(), 
		wxEVT_COMMAND_CHOICE_SELECTED,
		(wxObjectEventFunction) &CSGHelmPanel::OnNextMissionSelect );

	next_sizer->Add(m_NextMission,wxSizerFlags(1).Border());
	sizer->Add(next_sizer, wxSizerFlags(1).Expand().Border());

	//Buttons
	wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton* ReloadButton = new wxButton(this,-1,_T("Reload"));
	button_sizer->Add(ReloadButton, wxSizerFlags().Border());
	Connect( ReloadButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CSGHelmPanel::OnReloadButtonClicked );

	wxButton* StartButton = new wxButton(this,-1,_T("Start"));
	button_sizer->Add(StartButton, wxSizerFlags().Border());
	Connect( StartButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CSGHelmPanel::OnStartButtonClicked );

	wxButton* StopButton = new wxButton(this,-1,_T("Stop"));
	button_sizer->Add(StopButton, wxSizerFlags(1).Border());
	Connect( StopButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CSGHelmPanel::OnStopButtonClicked );

	sizer->Add(button_sizer, wxSizerFlags(1).Expand());


	wxBoxSizer* status_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_TimerText = new wxStaticText(this, -1, _T("Mission Stopped"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	status_sizer->Add(m_TimerText, wxSizerFlags().Center());
	sizer->Add(status_sizer, wxSizerFlags().Center());

	SetSizer(sizer);
	Center();

	m_Subscriptions.push_back(std::string("MOOS_DEBUG"));
	m_Subscriptions.push_back(std::string("MISSIONS_LOADED"));

	
	m_Timer = new wxTimer(this);
	Connect(m_Timer->GetId(), wxEVT_TIMER,
		(wxObjectEventFunction) &CSGHelmPanel::UpdateTimer );

}

void CSGHelmPanel::OnNextMissionSelect(wxCommandEvent event)
{
	m_MOOSComms->Notify("NEXT_MISSION", std::string(event.GetString().mb_str()));
}

void CSGHelmPanel::OnReloadButtonClicked(wxCommandEvent event)
{
	m_MOOSComms->Notify("RESTART_HELM", "TRUE");
}

void CSGHelmPanel::OnStartButtonClicked(wxCommandEvent event)
{
	m_MOOSComms->Notify("MOOS_MANUAL_OVERIDE", "FALSE");

	if(m_bCommandMySQL)
		m_MOOSComms->Notify("MYSQL_CMD",std::string("Start,")+(std::string)(m_NextMission->GetString(m_NextMission->GetCurrentSelection()).mb_str()));

	m_dfTimerStartTime=MOOSTime();
	wxCommandEvent NullEvent;
	UpdateTimer(NullEvent);
	m_Timer->Start(1000); //update every second.
}

void CSGHelmPanel::OnStopButtonClicked(wxCommandEvent event)
{
	m_MOOSComms->Notify("MOOS_MANUAL_OVERIDE", "TRUE");

	if(m_bCommandMySQL)
		m_MOOSComms->Notify("MYSQL_CMD",std::string("Stop"));

	m_Timer->Stop();
	m_TimerText->SetLabel(_T("Mission Stopped"));
}

void CSGHelmPanel::UpdateTimer(wxCommandEvent event)
{
	int nElapsed = (int)(MOOSTime()-m_dfTimerStartTime);
	
	wxString sElapsedTime;
	sElapsedTime.Printf(wxT("Elapsed Time: %d:%02d"), nElapsed/60, nElapsed%60);
	m_TimerText->SetLabel(sElapsedTime);
}



bool CSGHelmPanel::OnNewMail(MOOSMSG_LIST &NewMail)
{
	CMOOSMsg Msg;

	if(m_MOOSComms->PeekMail(NewMail,"MISSIONS_LOADED",Msg))
	{

		m_NextMission->Clear();
		wxString sVal = wxString(Msg.GetString().c_str(), wxConvUTF8);
		wxArrayString missions = wxStringTokenize(sVal, wxT(","));

		m_NextMission->Append(missions);

	}

	return true;
}



