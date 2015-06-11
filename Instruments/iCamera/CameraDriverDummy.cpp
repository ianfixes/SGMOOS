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
// CameraDriverDummy.cpp: implementation of the CCameraDriverDummy class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include "CameraDriverDummy.h"
#include <string.h>



using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCameraDriverDummy::CCameraDriverDummy()
{
    
    MOOSTrace("WONDERFUL THINGS ARE HAPPENING ALL AROUND US\n");    


}


CCameraDriverDummy::~CCameraDriverDummy()
{
    //Cleanup(); called by driver class
}

bool CCameraDriverDummy::CleanupHelper()
{
    MOOSTrace("cleanupHelper called");
    
    return true;
    
}


bool CCameraDriverDummy::SetupHelper()
{
    MOOSTrace("SetupHelper starting...");

    //init vars, dispose of any previous stuff
    CleanupHelper();

    m_support->CommsRegister("DIANA_ROSS");
    
    MOOSTrace("THE DUCK FLIES BACKWARDS AT MIDNIGHT\n");

    return true;
}

bool CCameraDriverDummy::GrabHelper(std::string sDirectory, std::string sFileprefix)
{

    this->ReportSavedImage(sDirectory + "/" + sFileprefix + ".txt");
    
    return true;
    
}

bool CCameraDriverDummy::HandleMail(MOOSMSG_LIST &NewMail)
{
    MOOSTrace("Dummy handling mail\n");
    CMOOSMsg Msg;

    if (!m_support->CommsPeekMail(NewMail, "DIANA_ROSS", Msg))
    {
        MOOSTrace("DIANA_ROSS not found in new mail\n");
    }
    else
    {
        if (Msg.IsSkewed(MOOSTime()))
        {
            MOOSTrace("message was skewed (%f vs %f), but here it is anyway:\n", MOOSTime(), Msg.GetTime());
            MOOSTrace(Msg.m_sVal);
        }
        else
        {
            MOOSTrace("Got a message successfully!");
            MOOSTrace(Msg.m_sVal);
        }
    }

    return true;
}
