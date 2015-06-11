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

CVarLogPanel::CVarLogPanel(wxWindow* Parent, CMOOSCommClient* Comms, std::string VarName, wxString Label)
: CMOOSPanel(Parent, Comms), m_sVarName(VarName)
{
	SetSize(500,500);

	if(Label.IsEmpty())
		Label = wxString(VarName.c_str(), wxConvUTF8);

	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(wxVERTICAL,this,Label);

	m_VarLog = new wxTextCtrl(
		this, -1, _T(""),
		wxDefaultPosition, wxDefaultSize,
		wxTE_MULTILINE | wxTE_READONLY | wxHSCROLL);

	sizer->Add(m_VarLog, wxSizerFlags(1).Expand());

	SetSizer(sizer);
	Center();

	m_Subscriptions.push_back(VarName);

}

bool CVarLogPanel::OnNewMail(MOOSMSG_LIST &NewMail)
{
	CMOOSMsg Msg;
	double dfNow = MOOSTime();
	if(m_MOOSComms->PeekMail(NewMail,m_sVarName,Msg))
	{
		if(!Msg.IsSkewed(dfNow))
		{
			wxDateTime MsgTime((time_t)Msg.GetTime());
			wxString sLine = _T("(")+MsgTime.FormatTime() + _T("):  ") +wxString((Msg.GetString()+"\n").c_str(), wxConvUTF8);
			m_VarLog->AppendText(sLine);
		}
	}

	return true;
}

