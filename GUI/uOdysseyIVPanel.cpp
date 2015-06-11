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
#ifndef uOIV_APP__INCLUDED_
#define uOIV_APP__INCLUDED_

#include "wxMOOSLib.h"

using namespace std;

class uOdysseyIVPanel : public wxMOOSApp
{
  protected:
	void LayoutPanels();
};

void uOdysseyIVPanel::LayoutPanels()
{

	wxSizerFlags DefaultFlags;
	DefaultFlags.Expand().Border();

	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);

	wxString sName;
	sName.FromAscii(m_sAppName.c_str());
	m_MainWindow = new CMOOSMainFrame(this, _T("Odyssey IV Control Panel"), wxDefaultPosition, wxSize(850,650));

	m_MainWindow->Connect( ID_Quit, wxEVT_COMMAND_MENU_SELECTED,
		(wxObjectEventFunction) &CMOOSMainFrame::OnQuit );
	m_MainWindow->Connect( ID_About, wxEVT_COMMAND_MENU_SELECTED,  wxCommandEventHandler(CMOOSMainFrame::OnConnectEvent));


	wxBoxSizer *TopRow = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *ControlCol = new wxBoxSizer(wxVERTICAL);

	m_Panels[string("Helm")] = new CSGHelmPanel(m_MainWindow, &m_Comms);
	ControlCol->Add(m_Panels["Helm"], DefaultFlags);

	m_Panels[string("Supervisory")] = new CSupervisoryPanel(m_MainWindow, &m_Comms);
	ControlCol->Add(m_Panels["Supervisory"], DefaultFlags);
/*
	m_Panels[string("Remote")] = new CRemoteControlPanel(m_MainWindow, &m_Comms);
	ControlCol->Add(m_Panels["Remote"], DefaultFlags);
*/
	m_Panels[string("Thrusters")] = new CActuationPanel(m_MainWindow, &m_Comms, "THRUSTERS", _T("Thrusters"));
	ControlCol->Add(m_Panels["Thrusters"], DefaultFlags);

	m_Panels[string("RTU")] = new CActuationPanel(m_MainWindow, &m_Comms, "RTU", _T("RTU"));
	ControlCol->Add(m_Panels["RTU"], DefaultFlags);

	

	TopRow->Add(ControlCol,wxSizerFlags(1).Expand().Border());// DefaultFlags);

	wxBoxSizer *SummaryCol = new wxBoxSizer(wxVERTICAL);

	m_Panels[string("MissionSummary")] = new CMissionSummaryPanel(m_MainWindow, &m_Comms);
	SummaryCol->Add(m_Panels["MissionSummary"], DefaultFlags);

	m_Panels[string("NavSummary")] = new CNavSummaryPanel(m_MainWindow, &m_Comms);
	SummaryCol->Add(m_Panels["NavSummary"], DefaultFlags);

	m_Panels[string("BatterySummary")] = new CBatterySummaryPanel(m_MainWindow, &m_Comms);
	SummaryCol->Add(m_Panels["BatterySummary"], wxSizerFlags(1).Expand().Border());

	TopRow->Add(SummaryCol, DefaultFlags);

	wxBoxSizer *DebugCol = new wxBoxSizer(wxVERTICAL);
	m_Panels[string("DebugLog")] = new CVarLogPanel(m_MainWindow, &m_Comms, string("MOOS_DEBUG"), _T("System Log"));
	DebugCol->Add(m_Panels["DebugLog"], wxSizerFlags(1).Expand().Border());

	m_Panels[string("Poke")] = new CMOOSPokePanel(m_MainWindow, &m_Comms);
	DebugCol->Add(m_Panels["Poke"], wxSizerFlags().Expand().Border());

	TopRow->Add(DebugCol,wxSizerFlags(1).Expand().Border());// DefaultFlags);

	mySizer->Add(TopRow, wxSizerFlags(1).Expand().Border());

	m_MainWindow->SetSizer(mySizer);
	m_MainWindow->Show(TRUE);
	SetTopWindow(m_MainWindow);

//	EnablePanels(false);

}

IMPLEMENT_APP(uOdysseyIVPanel);

#endif //!defined(uOIV_APP__INCLUDED_)

