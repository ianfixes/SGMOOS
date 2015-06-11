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

#include <wx/textdlg.h>
#include <wx/menu.h>

bool wxMOOSApp::OnInit()
{
	SetServer("LOCALHOST", 9000);
	m_sAppName="uControlPanel";
	LayoutPanels();

	return TRUE;
}

void wxMOOSApp::EnablePanels(bool bEnable)
{
	PANEL_MAP::iterator panel;
	for(panel=m_Panels.begin();panel!=m_Panels.end();panel++)
		panel->second->Enable(bEnable);
}

bool wxMOOSApp::Iterate()
{

	bool bOK = true;
	PANEL_MAP::iterator panel;
	for(panel=m_Panels.begin();panel!=m_Panels.end();panel++)
		bOK &= panel->second->Iterate();
	return bOK;
}
bool wxMOOSApp::OnConnectToServer()
{
	PANEL_MAP::iterator panel;
	STRING_LIST Subscriptions;
	for(panel=m_Panels.begin();panel!=m_Panels.end();panel++)
		panel->second->GetSubscriptions(Subscriptions);

	STRING_LIST::iterator sub;
	for(sub=Subscriptions.begin();sub!=Subscriptions.end();sub++)
		m_Comms.Register(*sub, 0.5);

	return true;
}

bool wxMOOSApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
	PANEL_MAP::iterator panel;
	for(panel=m_Panels.begin();panel!=m_Panels.end();panel++)
		panel->second->OnNewMail(NewMail);
	return true;
}

bool wxMOOSAPP_OnConnect(void * pParam)
{
    if(pParam!=NULL)
    {
		wxMOOSApp* pApp = (wxMOOSApp*)pParam;
		return pApp->OnConnectToServer();
    }
    return false;
}

bool wxMOOSApp::OnConnectRequest()
{
	wxTextEntryDialog* MOOSServerDialog = new wxTextEntryDialog(m_MainWindow,_T("Connect to MOOS Community"), _T("Enter the name or IP address"), wxString(m_sServerHost.c_str(), wxConvUTF8), wxOK | wxCANCEL);

	int nOk = MOOSServerDialog->ShowModal();
	if(nOk == wxID_OK)
	{
		m_sServerHost = string(MOOSServerDialog->GetValue().mb_str());

		//register a callback for On Connect
		m_Comms.SetOnConnectCallBack(wxMOOSAPP_OnConnect,this);
		//start the comms client....
		m_Comms.Run(m_sServerHost.c_str(),m_lServerPort,m_sAppName.c_str(),m_nCommsFreq);
		
		StartMOOSThread();
	}
	return true;
}
bool wxMOOSApp::StartMOOSThread()
{

	wxThreadError Result;

	// stop an existing thread... if applicable.
	if(m_MOOSAppThread!=NULL && m_MOOSAppThread->IsAlive())
	{
		Result = m_MOOSAppThread->Delete();
		if(Result != wxTHREAD_NO_ERROR)
		{
			Result = m_MOOSAppThread->Kill();
		}
		m_Comms.Close(true);
	}
	
	m_MOOSAppThread = new CMOOSThread(this);
	m_MOOSAppThread->Run();

	return m_MOOSAppThread->IsRunning();
}

void wxMOOSApp::MOOSLoop()
{
    MOOSMSG_LIST MailIn;
	
    while(1)
    {
        //look for mail
		double dfT1 = MOOSTime();

		if(m_Comms.Fetch(MailIn))
		{
			OnNewMail(MailIn);
		}

		if(m_Comms.IsConnected())
			Iterate();

//		EnablePanels(m_Comms.IsConnected());

        //store for derived class use the last time iterate was called;
        m_dfLastRunTime = HPMOOSTime();

        //sleep
        if(m_dfFreq>0)
        {			
			int nSleep = (int)(1000.0/m_dfFreq-1000*(m_dfLastRunTime-dfT1));
			
            if(nSleep>10)
            {
                MOOSPause(nSleep);
            }
        }
    }
}

bool wxMOOSApp::Notify(std::string sVarName, std::string sVal)
{
	return m_Comms.Notify(sVarName, sVal, MOOSTime());
}

bool wxMOOSApp::Notify(std::string sVarName, double dfVal)
{
	return m_Comms.Notify(sVarName, dfVal, MOOSTime());
}



//*******************************************************************
// Thread Portion of wxMOOSApp
//*******************************************************************
wxMOOSApp::CMOOSThread::CMOOSThread(wxMOOSApp* App) : m_MOOSObject(App)
{
	Create();
}

void* wxMOOSApp::CMOOSThread::Entry()
{
//	m_MOOSObject->Run("uControlPanel", "NoFileNecessary.moos");
	m_MOOSObject->MOOSLoop();
	return NULL;
}

//IMPLEMENT_APP(wxMOOSApp);

CMOOSMainFrame::CMOOSMainFrame(wxMOOSApp* app, const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame*)NULL,-1,title,pos,size)
{

	m_pApp = app;

	// create menubar
	wxMenuBar *menuBar = new wxMenuBar;
	// create menu
	wxMenu *menuFile = new wxMenu;
	// append menu entries
	menuFile->Append(ID_About,_T("&Connect To MOOS Community"));
	menuFile->AppendSeparator();
	menuFile->Append(ID_Quit,_T("E&xit"));
	// append menu to menubar
	menuBar->Append(menuFile,_T("&File"));
	// set frame menubar
	SetMenuBar(menuBar);

	// create frame statusbar
	CreateStatusBar();
	// set statusbar text
	SetStatusText(_T("Welcome to wxWindows!"));
}

void CMOOSMainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}


void CMOOSMainFrame::OnConnectEvent(wxCommandEvent& WXUNUSED(event))
{
	m_pApp->OnConnectRequest();
}

