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

#ifndef SUMMARY_PANEL__INCLUDED_
#define SUMMARY_PANEL__INCLUDED_

#include "MOOSPanel.h"
class CSummaryPanel;

/**
 * FIXME: needs class description
 */
class CSummaryRow : private wxEvtHandler
{
  public:
	CSummaryRow(wxString sLabel, std::string sVar, wxString sUnits, CSummaryPanel* Parent);
	~CSummaryRow();
	void SetValue(wxString sVal);
  private:
	void OnTimeout(wxCommandEvent event);
	int m_nTimeout;

	wxStaticText *m_Label, *m_Units;
	wxTextCtrl* m_Value;
	wxTimer m_Timer;
};

typedef std::map< std::string, CSummaryRow* > SUMMARY_MAP;

class CSummaryPanel : public CMOOSPanel
{
  public:
	CSummaryPanel(wxWindow* Parent, CMOOSCommClient* Comms);
	virtual ~CSummaryPanel();

	virtual bool OnNewMail(MOOSMSG_LIST &NewMail);
	virtual void GetSubscriptions(STRING_LIST& subs);
	friend class CSummaryRow;
  protected:
	virtual void SetupRows()=0;
	virtual void DoLayout();
	void CreateRow(wxString sLabel, std::string sVar, wxString sUnits=_T(""));
	SUMMARY_MAP m_Rows;
	wxString m_sLabel;
	int m_nRows;
	wxFlexGridSizer* m_flexgrid;

//  friend CSummaryRow;
};

#endif //!defined(SUMMARY_PANEL__INCLUDED_)
