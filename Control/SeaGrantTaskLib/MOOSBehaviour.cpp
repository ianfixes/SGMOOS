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
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif


// MOOSBehaviour.cpp: implementation of the CMOOSBehaviour class.
//
//////////////////////////////////////////////////////////////////////
#include "math.h"
#include <MOOSGenLib/MOOSGenLib.h>
#include "MOOSBehaviour.h"

#include <sstream>
#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSGMOOSBehaviour::CSGMOOSBehaviour()
{
    m_bComplete = false;
    m_bActive = false;
    m_bInitialised = false;

    m_sName="NotSet";
    m_sClassName = "Behaviour Base";

    m_pMissionFileReader = NULL;

    m_bNewRegistrations = true;

    m_dfTimeOut=MOOS_DEFUALT_TIMEOUT;
    m_dfStartTime=-1;

    m_dfCreationTime = MOOSTime();

    m_dfIterateTime = m_dfCreationTime;

    m_nPriority = 3;

    // we're on top, unless told otherwise
    m_bTopLevel = true;

}

CSGMOOSBehaviour::~CSGMOOSBehaviour()
{

}

FLAG_LIST CSGMOOSBehaviour::m_Flags = FLAG_LIST();

// Default for behaviors which don't have anything to intialise;
bool CSGMOOSBehaviour::Initialise()
{
    m_bInitialised = true;
    return m_bInitialised;
}

bool CSGMOOSBehaviour::Run(CPathAction &DesiredAction)
{
    return false;
}

bool CSGMOOSBehaviour::RegularMailDelivery(double dfTimeNow)
{
    return true;
}

bool CSGMOOSBehaviour::OnNewMail(MOOSMSG_LIST &NewMail)
{
    return true;
}

bool CSGMOOSBehaviour::GetRegistrations(STRING_LIST &List)
{

    // skip if I'm a component of another behavior
    if(!m_bTopLevel)
        return true;
  
    m_bNewRegistrations = false;
  
    return true;
}

bool CSGMOOSBehaviour::PeekMail(MOOSMSG_LIST &Mail, const string &sKey, CMOOSMsg &Msg)
{
    //note we adopt the policy of always taking the youngest message...
    MOOSMSG_LIST::iterator p,q;
    
    q = Mail.end();

    double dfT = -1.0;

    //iterate over all messages
    for(p = Mail.begin(); p != Mail.end(); p++)
    {
        //fheck matching key
        if(MOOSStrCmp(p->m_sKey, sKey))
        {
            if(dfT < p->GetTime())
            {
                dfT = p->GetTime();
                q = p;
            }
        }
    }
    
    //did we find one?
    if(q != Mail.end())
    {
        Msg = *q;
        return true;
    }
    else
    {
        return false;
    }
}

bool CSGMOOSBehaviour::GetNotifications(MOOSMSG_LIST &List)
{
    List.splice(List.begin(),m_Notifications);
    return true;
}


bool CSGMOOSBehaviour::ShouldRun()
{
/*
    if(m_bComplete)
        return false; //we have already run once and completed
*/
    if(!m_bActive)
    {
          return false;
    }
     

    if(m_dfStartTime==-1)
    {
        //yep we should go but haven't started as yet
        Start();
    }

    double dfTimeNow = MOOSTime();
    //look for timeouts!!!
    if(m_dfTimeOut!=-1 && ((dfTimeNow -m_dfStartTime)>m_dfTimeOut))
    {
        //hell no! we should have achieved our goal by now..
        OnTimeOut();
        return false;
    }
     
    //look for active task that is not receiving any updates
    //on its watched variables?
    if(!RegularMailDelivery(dfTimeNow))
    {
        string sStr = MOOSFormat("Task %s stops for lack of input\n", m_sName.c_str());

        DebugNotify(sStr);

        //publish error string...
        OnError(sStr);

        //now figure our what could have gone wrong...
        STRING_LIST Expected;

        GetRegistrations(Expected);

        string sTemp= "Not receiving one of: ";

        STRING_LIST::iterator p;
        for(p = Expected.begin();p!=Expected.end();p++)
        {
             sTemp += *p + " ";
        }


        DebugNotify(sTemp);

        return false;
    }

    return true;
}



bool CSGMOOSBehaviour::Start()
{
    m_bActive = true;
    
    m_dfStartTime = MOOSTime();
    
    if(m_bTopLevel)
    {
        DebugNotify(MOOSFormat("Task %s goes active",m_sName.c_str()));
    }
    
    //let derived classes do something
    return OnStart();

}


void CSGMOOSBehaviour::Stop(const string & sReason)
{
    DebugNotify(MOOSFormat("Task %s %s", m_sName.c_str(), sReason.c_str()));

    m_bComplete = true;
    m_bActive = false;
}

void CSGMOOSBehaviour::ProcessFlags()
{
    if(!m_bActive)
    {
        FLAG_LIST::iterator i;
        for(i=m_Flags.begin(); i!=m_Flags.end(); i++)
            if(*i == m_sStartFlag)
                Start();
    }
}

bool CSGMOOSBehaviour::SetParam(string sParam, string sVal)
{

    if(!m_bTopLevel)
        return false;

    MOOSToUpper(sParam);
    MOOSToUpper(sVal);

    if(sParam=="NAME")
    {
        SetName(sVal);
    }

    else if(sParam=="INITIALSTATE")
    {
        if(sVal =="ON")
        {
            m_sStartFlag = (FLAG_TYPE)"START_MISSION";
            m_bActive = true;
        }
        else
        {
            m_bActive = false;
        }
    }
    else if(sParam=="PRIORITY")
    {
        m_nPriority = atoi(sVal.c_str());
    }
    else if(sParam=="STARTFLAG")
    {
        m_sStartFlag = (FLAG_TYPE)sVal;
    }
    else if(sParam =="COMPLETEFLAG")
    {
        m_sCompleteFlag = (FLAG_TYPE)sVal;
    }
    else if(sParam =="TIMEOUTFLAG")
    {
        m_sTimeoutFlag = (FLAG_TYPE)sVal;
    }
    else if(sParam =="ERRORFLAG")
    {
        m_sErrorFlag = (FLAG_TYPE)sVal;
    }
    else if(sParam == "TIMEOUT")
    {
        if(sVal=="NEVER")
        {
            m_dfTimeOut = -1;
        }
        else
        {
            m_dfTimeOut = atof(sVal.c_str());
            if(m_dfTimeOut==0.0)
            {
                MOOSTrace("warning task set for zero timeout..is this intended?\n");
            }
        }
    }
    else
    {
        return false;
    }

    return true;

}

string CSGMOOSBehaviour::GetName()
{
    return m_sName;
}

void CSGMOOSBehaviour::SetMissionFileReader(CProcessConfigReader* pMissionFileReader)
{
    m_pMissionFileReader = pMissionFileReader;
}

bool CSGMOOSBehaviour::OnComplete()
{
    //we are completed!
    //default param is "DONE"
    ThrowFlag(m_sCompleteFlag);
    Stop("Done");

    return true;}

bool CSGMOOSBehaviour::OnTimeOut()
{
    ostringstream os;

    os<< "timed out after "<<m_dfTimeOut<<" seconds"<<endl<<ends;

    ThrowFlag(m_sTimeoutFlag);

    Stop(os.str());
    //os.rdbuf()->freeze(0);

    return true;
}

bool CSGMOOSBehaviour::OnError(string sReason)
{
/*
    string sKey = m_sName+"_ERROR";
    CMOOSMsg ErrorMsg(MOOS_NOTIFY,sKey.c_str(),sReason.c_str());
    m_Notifications.push_front(ErrorMsg);
*/

    ThrowFlag(m_sErrorFlag);
    Stop("FAILED: " + sReason);
    return true;
}

void CSGMOOSBehaviour::ClearFlags() 
{
    CSGMOOSBehaviour::m_Flags.clear();
}

bool CSGMOOSBehaviour::HasFlags() const 
{
    return !m_Flags.empty();
}

void CSGMOOSBehaviour::ThrowFlag(FLAG_TYPE flag)
{
    m_Flags.push_back(flag);
    MOOSTrace("FLAG: %s from %s\n", flag.c_str(), GetName().c_str());
}

bool CSGMOOSBehaviour::HasNewRegistration()
{
    return m_bNewRegistrations;
}

double CSGMOOSBehaviour::GetStartTime()
{
    return m_dfStartTime;
}

double CSGMOOSBehaviour::GetCreationTime()
{
    return m_dfCreationTime;
}

bool CSGMOOSBehaviour::DebugNotify(const string &sStr)
{
    CMOOSMsg DebugMsg(MOOS_NOTIFY,"MOOS_DEBUG",sStr.c_str());
    m_Notifications.push_front(DebugMsg);    

    MOOSTrace(sStr + "\n");

    return true;
}


void CSGMOOSBehaviour::SetName(string sName)
{
	m_sName = sName;
}

int CSGMOOSBehaviour::GetPriority()
{
	return m_nPriority;
}

void CSGMOOSBehaviour::SetPriority(int nPriority)
{
	m_nPriority = nPriority;
}

bool CSGMOOSBehaviour::ReInitialise()
{

    m_bComplete = false;
    m_bActive = false;

    m_sName="NotSet";

    m_pMissionFileReader = NULL;

    m_bNewRegistrations = true;

    m_dfTimeOut=MOOS_DEFUALT_TIMEOUT;
    
    m_dfStartTime=-1;

    m_dfCreationTime = MOOSTime();

    m_dfIterateTime = m_dfCreationTime;

    m_nPriority = 3;

    return true;
}

void CSGMOOSBehaviour::SetTime(double dfTimeNow)
{
	m_dfIterateTime = dfTimeNow;
}

//overload this to do something as task is starting
bool CSGMOOSBehaviour::OnStart()
{
    return true;
}

string CSGMOOSBehaviour::GetChartNodeEntry()
{
    STRING_LIST Contents;
    string sLabel;
    STRING_LIST::iterator i;
    
    Contents.push_back(m_sName);
    Contents.push_back("("+m_sClassName+")");
    GetChartNodeContents(Contents);
    Contents.push_back(MOOSFormat("priority: %d",m_nPriority));

    for(i=Contents.begin();i!=Contents.end();i++)
    {
        if(i!=Contents.begin())
            sLabel+="|";
        sLabel+=*i;
    }

    string output = MOOSFormat("\t\"%s\"[shape=Mrecord, label=\"{%s}\"];\n",
        m_sName.c_str(), sLabel.c_str());
    return output;
}


string CSGMOOSBehaviour::GetChartEdgeEntry()
{
    string output = MOOSFormat("\t\"%s\"->\"%s\";\n", m_sStartFlag.c_str(), m_sName.c_str());

    if(!m_sCompleteFlag.empty())
        output += MOOSFormat("\t\"%s\"->\"%s\" [label=Complete];\n", m_sName.c_str(), m_sCompleteFlag.c_str());

    if(!m_sTimeoutFlag.empty())
        output += MOOSFormat("\t\"%s\"->\"%s\" [label=\"Timeout(%.1f)\"];\n", m_sName.c_str(), m_sTimeoutFlag.c_str(), m_dfTimeOut);

    if(!m_sErrorFlag.empty())
        output += MOOSFormat("\t\"%s\"->\"%s\" [label=Error];\n", m_sName.c_str(), m_sErrorFlag.c_str());

    return output;
}

/*******************************************************************/

CSGMOOSBehaviour::ControlledDOF::ControlledDOF()
{
    m_dfCurrent = 0.0;
    m_dfCurrentTime = -1.0;
    m_dfDesired = 0.0;
    m_dfTolerance = 0.0;
    m_bDesiredSet=false;
}

void CSGMOOSBehaviour::ControlledDOF::SetCurrent(double dfCurrent, double dfTime)
{
    m_dfCurrent     = dfCurrent;
    m_dfCurrentTime = dfTime;
}


void CSGMOOSBehaviour::ControlledDOF::SetDesired(double dfDesired)
{
    m_dfDesired  = dfDesired;
    m_bDesiredSet = true;
}

void CSGMOOSBehaviour::ControlledDOF::SetTolerance(double dfTol)
{
    m_dfTolerance = dfTol;
}

double CSGMOOSBehaviour::ControlledDOF::GetError() const
{
    double dfErr = m_dfDesired-m_dfCurrent;
    if(fabs(dfErr)<=m_dfTolerance)
    {
        return 0.0;
    }
    else
    {
        return dfErr;
    }

}

double CSGMOOSBehaviour::ControlledDOF::GetErrorTime() const
{
    return m_dfCurrentTime;
}

double CSGMOOSBehaviour::ControlledDOF::GetDesired() const
{
    return m_dfDesired;
}

double CSGMOOSBehaviour::ControlledDOF::GetCurrent() const
{
    return m_dfCurrent;
}

bool CSGMOOSBehaviour::ControlledDOF::IsValid() const
{
    return m_dfCurrentTime!=-1.0;
}

bool CSGMOOSBehaviour::ControlledDOF::IsSet() const
{
    return m_bDesiredSet;
}

bool CSGMOOSBehaviour::ControlledDOF::IsStale(double dfTimeNow, double dfTaskStartTime,double dfTimeOut) const
{
    //case 1: No mail at all...
    if(!IsValid()&& (dfTimeNow-dfTaskStartTime>dfTimeOut))
        return true;

    //case 2: Had mail but it has stopped..
    if(IsValid() && dfTimeNow-m_dfCurrentTime>dfTimeOut)
        return true;

    //then we are OK...
    return false;
}
