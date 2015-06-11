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
#ifndef WX_MOOS_APP__INCLUDED_
#define WX_MOOS_APP__INCLUDED_

#include "wxMOOSLib.h"
#include <wx/app.h>
#include <wx/thread.h>
#include <wx/window.h>
#include <wx/frame.h>

using namespace std;

typedef map<string, CMOOSPanel*> PANEL_MAP;

//Forward Declaration
class wxMOOSApp;


/**
 * FIXME: needs class description
 */
class CMOOSMainFrame : public wxFrame
{
public:
  
    CMOOSMainFrame(wxMOOSApp* app, const wxString& title, const wxPoint& pos, const wxSize& size);
    void OnQuit(wxCommandEvent& event);
    void OnConnectEvent(wxCommandEvent& event);
    
protected:
    wxMOOSApp* m_pApp;
};


/**
 * FIXME: needs class description
 */
class wxMOOSApp : public wxApp, public CMOOSApp
{
 public:
  
    bool Notify(std::string sVarName, std::string sVal);
    bool Notify(std::string sVarName, double Val);
    bool OnConnectRequest();
    void MOOSLoop();
    virtual bool OnConnectToServer();

  protected:
    /**
     * FIXME: needs class description
     */
    class CMOOSThread : public wxThread
    {
    public:
        CMOOSThread(wxMOOSApp* App);
        void* Entry();
    protected:
        wxMOOSApp* m_MOOSObject;
    };
    
    virtual void LayoutPanels()=0;
    bool Iterate();
    void EnablePanels(bool bEnable);
    CMOOSThread* m_MOOSAppThread;
    bool StartMOOSThread();
    
    virtual bool OnNewMail(MOOSMSG_LIST & NewMail);
    virtual bool OnInit();
    
    CMOOSMainFrame* m_MainWindow;
    
    PANEL_MAP m_Panels;
};

enum
{
	ID_Quit=1,
	ID_About
};
#endif //!defined(WX_MOOS_APP__INCLUDED_)

