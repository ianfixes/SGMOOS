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
#ifndef SGHELM_PANEL__INCLUDED_
#define SGHELM_PANEL__INCLUDED_

#include "MOOSPanel.h"

/**
 * FIXME: needs class description
 */
class CSGHelmPanel : public CMOOSPanel
{
  public:
	CSGHelmPanel(wxWindow* Parent, CMOOSCommClient* Comms);
	virtual bool OnNewMail(MOOSMSG_LIST &NewMail);

  protected:
	void OnNextMissionSelect(wxCommandEvent event);
	void OnReloadButtonClicked(wxCommandEvent event);
	void OnStartButtonClicked(wxCommandEvent event);
	void OnStopButtonClicked(wxCommandEvent event);
	void UpdateTimer(wxCommandEvent event);


	wxChoice* m_NextMission;
	wxTextCtrl* m_HelmLog;
	wxStaticText* m_TimerText;
	wxTimer* m_Timer;
	bool m_bCommandMySQL;
	double m_dfTimerStartTime;
};
#endif //!defined(SGHELM_PANEL__INCLUDED_)
