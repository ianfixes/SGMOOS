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

CMOOSPokePanel::CMOOSPokePanel(wxWindow* Parent, CMOOSCommClient* Comms) : CMOOSPanel(Parent, Comms)
{
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(wxVERTICAL,this,_T("Poke the MOOS DB"));

	wxBoxSizer* Row1 = new wxBoxSizer(wxHORIZONTAL);
	
	Row1->Add(new wxStaticText(this, -1, _T("VarName:")), wxSizerFlags());//.Border());

	m_VarName=new wxComboBox(this, -1);
	Row1->Add(m_VarName, wxSizerFlags(1).Right().Expand());
	
	//Refresh button
	wxButton* ReloadButton = new wxButton(this,-1,_T("Refresh"));
	Row1->Add(ReloadButton, wxSizerFlags().Border());
	Connect( ReloadButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
		(wxObjectEventFunction) &CMOOSPokePanel::DoRefresh );


	sizer->Add(Row1, wxSizerFlags().Expand());

	wxBoxSizer* Row2 = new wxBoxSizer(wxHORIZONTAL);

	Row2->Add(new wxStaticText(this, -1, _T("Value:")), wxSizerFlags());//.Border());

	m_Value=new wxTextCtrl(this, -1, _T(""),
		wxDefaultPosition, wxDefaultSize,
		wxTE_PROCESS_ENTER);

	Connect(m_Value->GetId(), wxEVT_COMMAND_TEXT_ENTER,
		(wxObjectEventFunction) &CMOOSPokePanel::DoPoke);

	Row2->Add(m_Value, wxSizerFlags(1).Right().Expand());

	sizer->Add(Row2, wxSizerFlags().Expand().Border());

	SetSizer(sizer);
	Center();

}

void CMOOSPokePanel::DoPoke(wxCommandEvent event)
{
	double dfVal;
	wxString sVal = m_Value->GetValue();


	//if its numeric post it as a number
	if(sVal.ToDouble(&dfVal))
		m_MOOSComms->Notify(std::string(m_VarName->GetValue().mb_str()), dfVal);
	else
		m_MOOSComms->Notify(std::string(m_VarName->GetValue().mb_str()), std::string(m_Value->GetValue().mb_str()));

	
}
void CMOOSPokePanel::DoRefresh(wxCommandEvent event)
{
	MOOSMSG_LIST Msgs;
	if(m_MOOSComms->ServerRequest("VAR_SUMMARY",Msgs))
	{
		m_VarName->Clear();
		wxString sVal = wxString(Msgs.begin()->GetString().c_str(), wxConvUTF8);
		wxArrayString Vars = wxStringTokenize(sVal, wxT(","));

		m_VarName->Append(Vars);
	}

}
