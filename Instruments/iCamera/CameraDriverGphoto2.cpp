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
// CameraDriverGphoto2.cpp: implementation of the CCameraDriverGphoto2 class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include "CameraDriverGphoto2.h"
#include <string.h>

#define PROMPT "gphoto2: {gphoto2:"


using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCameraDriverGphoto2::CCameraDriverGphoto2()
{
    MOOSTrace("MANY FINE BOOKS HAVE BEEN WRITTEN IN PRISON\n");    
    
    m_cb = NULL;
}


CCameraDriverGphoto2::~CCameraDriverGphoto2()
{
    //Cleanup(); called by driver class
}

bool CCameraDriverGphoto2::CleanupHelper()
{
    //initialize all variables to nothing
    if (m_cb)
    {
        m_cb->tSend("set-config capture off\r");
        m_cb->vWaitFor(PROMPT, "\nsetting capture off...", "done!", "failed");
    }


    delete m_cb;

    m_cb = NULL;

    return m_cb == NULL;
    
}

bool CCameraDriverGphoto2::GotAllConfigs()
{
    return !m_sGphotoBinary.empty();
}


//read what we need from config file
bool CCameraDriverGphoto2::GetConfigs()
{
    //executable for gphoto
    if (!m_support->ConfigRead("GPHOTO_BINARY", m_sGphotoBinary))
    {
        MOOSTrace("Configuration variable GPHOTO_BINARY not found!\n");
    }


    return GotAllConfigs();

}


bool CCameraDriverGphoto2::SetupHelper()
{
    std::string tmp;

    //init vars, dispose of any previous stuff
    CleanupHelper();

    //bail out if not all conifgs are supplied
    if (GetConfigs())
    {
        MOOSTrace("All necessary config vars have been supplied :)\n");
    }
    else
    {
        MOOSTrace("Some config vars missing, setup is aborting\n");
        return false;
    }
   
    m_cb = new CLIBuster();
    m_cb->init(m_sGphotoBinary + " --shell");
    m_cb->setDebug(false);
    m_cb->setLogUser(false);
    m_cb->setTimeout(60);

    m_cb->tSend("set-config capture on\r");
    m_cb->vWaitFor(PROMPT, "\nsetting capture on...", "done!", "failed");

    m_bChangedDirectory = false;

    return true;
}

void CCameraDriverGphoto2::CreateCameraRegisters()
{
    return;
}


bool CCameraDriverGphoto2::HandleMail(MOOSMSG_LIST &NewMail)
{
    MOOSTrace("Got new mail in Gphoto2!\n");

    return true;
}

bool CCameraDriverGphoto2::GrabHelper(std::string sDirectory, std::string sFileprefix)
{
    std::string tmp;
    std::string sFilename;

    //change to directory if we haven't yet
    if (!m_bChangedDirectory)
    {
        tmp = "lcd ";
        m_cb->tSend(tmp + sDirectory.c_str());
        tmp = "\nsetting output directory to ";
        m_cb->vWaitFor(PROMPT, tmp + sDirectory, "done!", "failed");
        m_bChangedDirectory = true;
    }


    //get image
    m_cb->tSend("capture-image-and-download\r");
    m_cb->vWaitFor(PROMPT, "\ncapturing and downloading image...", "done!", "failed"); 
    
    //get filename from output
    tmp = CLIBuster::striptags(m_cb->buffer(), "Deleting '", "'");

    //rename saved file
    sFilename = sFileprefix + ".jpg";
    printf("\nRenaming: '%s' to '%s.jpg'", tmp.c_str(), sFilename.c_str());
    rename(tmp.c_str(), sFilename.c_str());

    return true;

}
