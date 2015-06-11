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

CSupervisoryPanel::CSupervisoryPanel(wxWindow* Parent, CMOOSCommClient* Comms) : CMOOSPanel(Parent, Comms)
{
	m_dfPositionStep = 1.0;
	m_dfHeadingStep = 10.0;

	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(wxVERTICAL,this,_T("Supervisory Control"));

	//Buttons
	wxBoxSizer* translational_sizer = new wxBoxSizer(wxVERTICAL);

	wxButton* ForwardButton = new wxButton(this,-1,_T("Forward"));
	translational_sizer->Add(ForwardButton, wxSizerFlags().Center().Top());
	Connect( ForwardButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CSupervisoryPanel::OnForward );

	wxBoxSizer* translational_sizer_mid = new wxBoxSizer(wxHORIZONTAL);

	wxButton* SwayLeftButton = new wxButton(this,-1,_T("Left"));
	translational_sizer_mid->Add(SwayLeftButton, wxSizerFlags().Left());
	Connect( SwayLeftButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CSupervisoryPanel::OnSwayLeft );


	wxButton* SwayRightButton = new wxButton(this,-1,_T("Right"));
	translational_sizer_mid->Add(SwayRightButton, wxSizerFlags().Right());
	Connect( SwayRightButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CSupervisoryPanel::OnSwayRight );

	translational_sizer->Add(translational_sizer_mid, wxSizerFlags());

	wxButton* BackwardButton = new wxButton(this,-1,_T("Back"));
	translational_sizer->Add(BackwardButton, wxSizerFlags().Bottom().Center());
	Connect(BackwardButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CSupervisoryPanel::OnBackward );

	sizer->Add(translational_sizer, wxSizerFlags());

	wxStaticBoxSizer *rotational_sizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_T("Heading"));

	wxButton* YawLeftButton = new wxButton(this,-1,_T("Turn Left"));
	rotational_sizer->Add(YawLeftButton, wxSizerFlags().Left());
	Connect(YawLeftButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CSupervisoryPanel::OnYawLeft );

	wxButton* YawRightButton = new wxButton(this,-1,_T("Turn Right"));
	rotational_sizer->Add(YawRightButton, wxSizerFlags().Right());
	Connect(YawRightButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CSupervisoryPanel::OnYawRight );

	sizer->Add(rotational_sizer, wxSizerFlags());

	wxBoxSizer* altitude_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText *altLabel = new wxStaticText(this, -1, _T("Altitude"));
	altitude_sizer->Add(altLabel, wxSizerFlags(1).Expand().Left());

	m_AltitudeTxt=new wxTextCtrl(this, -1, _T(""),
		wxDefaultPosition, wxDefaultSize,
		wxTE_PROCESS_ENTER | wxTE_RIGHT);
	Connect(m_AltitudeTxt->GetId(), wxEVT_COMMAND_TEXT_ENTER,
		(wxObjectEventFunction) &CSupervisoryPanel::OnAltitude);
	altitude_sizer->Add(m_AltitudeTxt, wxSizerFlags().Right());
	sizer->Add(altitude_sizer, wxSizerFlags());


	wxBoxSizer* depth_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText *depthLabel = new wxStaticText(this, -1, _T("Depth"));
	depth_sizer->Add(depthLabel, wxSizerFlags(1).Expand().Left());

	m_DepthTxt=new wxTextCtrl(this, -1, _T(""),
		wxDefaultPosition, wxDefaultSize,
		wxTE_PROCESS_ENTER | wxTE_RIGHT);
	Connect(m_DepthTxt->GetId(), wxEVT_COMMAND_TEXT_ENTER,
		(wxObjectEventFunction) &CSupervisoryPanel::OnDepth);
	depth_sizer->Add(m_DepthTxt, wxSizerFlags().Right());
	sizer->Add(depth_sizer, wxSizerFlags());


	SetSizer(sizer);

}

void CSupervisoryPanel::OnForward(wxCommandEvent event)
{
	m_MOOSComms->Notify("MANUAL_MOVE_WAYPOINT_FORWARD", m_dfPositionStep);
}

void CSupervisoryPanel::OnBackward(wxCommandEvent event)
{
	m_MOOSComms->Notify("MANUAL_MOVE_WAYPOINT_FORWARD", -m_dfPositionStep);
}

void CSupervisoryPanel::OnSwayRight(wxCommandEvent event)
{
	m_MOOSComms->Notify("MANUAL_MOVE_WAYPOINT_STARBOARD", m_dfPositionStep);
}

void CSupervisoryPanel::OnSwayLeft(wxCommandEvent event)
{
	m_MOOSComms->Notify("MANUAL_MOVE_WAYPOINT_STARBOARD", -m_dfPositionStep);
}

void CSupervisoryPanel::OnYawRight(wxCommandEvent event)
{
	m_MOOSComms->Notify("MANUAL_HEADING_STEP", m_dfHeadingStep);
}

void CSupervisoryPanel::OnYawLeft(wxCommandEvent event)
{
	m_MOOSComms->Notify("MANUAL_HEADING_STEP", -m_dfHeadingStep);
}

void CSupervisoryPanel::OnAltitude(wxCommandEvent event)
{
	wxString sAlt = m_AltitudeTxt->GetValue();
	double dfAlt;

	if(sAlt.ToDouble(&dfAlt))
		m_MOOSComms->Notify("MANUAL_ALTITUDE", dfAlt);
}

void CSupervisoryPanel::OnDepth(wxCommandEvent event)
{
	wxString sDepth = m_DepthTxt->GetValue();
	double dfDepth;

	if(sDepth.ToDouble(&dfDepth))
		m_MOOSComms->Notify("MANUAL_DEPTH", dfDepth);
}