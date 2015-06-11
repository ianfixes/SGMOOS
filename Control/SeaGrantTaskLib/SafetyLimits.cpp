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
// SafetyLimits.cpp: implementation of the CSafetyLimits class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include "SafetyLimits.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSafetyLimits::CSafetyLimits()
{
    m_sClassName="Safety Limits";
}

CSafetyLimits::~CSafetyLimits()
{
    MOOSTrace(m_sClassName + " is releasing its dynamically allocated memory.. just an FYI\n");
    LIMITS_MAP::iterator lim;
    for(lim = m_LimitedDOFs.begin(); lim != m_LimitedDOFs.end(); lim++)
    {
        if (lim->second.max) delete lim->second.max;
        if (lim->second.min) delete lim->second.min;
    }
    m_LimitedDOFs.clear();
}

bool CSafetyLimits::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;

    LIMITS_MAP::iterator lim;
    for(lim = m_LimitedDOFs.begin(); lim != m_LimitedDOFs.end(); lim++)
    {
        if(PeekMail(NewMail, lim->first, Msg))
        { 
            if(!Msg.IsSkewed(MOOSTime()))
            {
                if (lim->second.max) lim->second.max->SetCurrent(Msg.GetDouble(), Msg.GetTime());
                if (lim->second.min) lim->second.min->SetCurrent(Msg.GetDouble(), Msg.GetTime());
            }
        }
    }
    return CSGMOOSBehaviour::OnNewMail(NewMail);

}
bool CSafetyLimits::GetRegistrations(STRING_LIST &List)
{

    LIMITS_MAP::iterator lim;
    for(lim = m_LimitedDOFs.begin(); lim != m_LimitedDOFs.end(); lim++)
    {
        List.push_front(lim->first);
    }
    
    //always call base class version
    CSGMOOSBehaviour::GetRegistrations(List);

    return true;
}

void CSafetyLimits::GetChartNodeContents(STRING_LIST& contents) const
{
    string sMsg, sMin, sMax;

    LIMITS_MAP::const_iterator lim;
    for(lim = m_LimitedDOFs.begin(); lim != m_LimitedDOFs.end(); lim++)
    {

        sMsg = lim->first.c_str();

        if (lim->second.max)
        {
            sMsg = sMsg + MOOSFormat(" ≤ %f", lim->second.max->GetDesired());
        }

        if (lim->second.min)
        {
            sMsg = MOOSFormat("%f ≤ ", lim->second.min->GetDesired()) + sMsg;
        }

        MOOSTrace((string)"Building chart node: " + sMsg + "\n");
        contents.push_back(sMsg);

    }
}


bool CSafetyLimits::Run(CPathAction &DesiredAction)
{
    string sMsg;

    LIMITS_MAP::iterator lim;
    for(lim = m_LimitedDOFs.begin(); lim != m_LimitedDOFs.end(); lim++)
    {
        if (lim->second.max && !lim->second.max->isOK())
        {
            sMsg = MOOSFormat("%s: %f is over limit of %f",
                              lim->first.c_str(), 
                              lim->second.max->GetCurrent(),
                              lim->second.max->GetDesired());
                              
            DebugNotify(MOOSFormat("SAFETY LIMIT REACHED (%s)\n", sMsg.c_str()));

            CMOOSMsg oops(MOOS_NOTIFY,"SAFETY_MESSAGE", sMsg);
            m_Notifications.push_front(oops);

            OnComplete();
        }

        if (lim->second.min && !lim->second.min->isOK())
        {
            sMsg = MOOSFormat("%s: %f is under limit of %f",
                              lim->first.c_str(), 
                              lim->second.min->GetCurrent(),
                              lim->second.min->GetDesired());
                              
            DebugNotify(MOOSFormat("SAFETY LIMIT REACHED (%s)\n", sMsg.c_str()));

            CMOOSMsg oops(MOOS_NOTIFY,"SAFETY_MESSAGE", sMsg);
            m_Notifications.push_front(oops);
            
            OnComplete();
        }
    }
    return true;
}

bool CSafetyLimits::RegularMailDelivery(double dfTimeNow)
{

    LIMITS_MAP::iterator lim;
    for(lim = m_LimitedDOFs.begin(); lim != m_LimitedDOFs.end(); lim++)
    {
        if (lim->second.max && lim->second.max->IsStale(dfTimeNow,GetStartTime()))
            return false;

        if (lim->second.min && lim->second.min->IsStale(dfTimeNow,GetStartTime()))
            return false;
    }
    return true;
}

bool CSafetyLimits::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);

    if (sParam != "RANGE") return CSGMOOSBehaviour::SetParam(sParam, sVal);

    string sVar = MOOSChomp(sVal, ":");
    string sMin = MOOSChomp(sVal, ",");
    string sMax = sVal;

    Limits lim;
    lim.min = 0;
    lim.max = 0;

    if ("" != sMin)
    {
        lim.min = new CMinDOF(atof(sMin.c_str()));
        MOOSTrace(string("Minimum ") + sVar + " is " + sMin + "\n");
    }

    if ("" != sMax)
    {
        lim.max = new CMaxDOF(atof(sMax.c_str()));
        MOOSTrace(string("Maximum ") + sVar + " is " + sMax + "\n");
    }

    m_LimitedDOFs[sVar] = lim;
    return true;
}
