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
// MOOSBehaviour.h: interface for the CMOOSBehaviour class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SGMOOSBEHAVIOUR_H__1C10DD47_7690_4AEF_9174_0B0EA068A77D__INCLUDED_)
#define AFX_SGMOOSBEHAVIOUR_H__1C10DD47_7690_4AEF_9174_0B0EA068A77D__INCLUDED_

#include <MOOSLIB/MOOSLib.h>
#include "PathAction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
using namespace std;

typedef string FLAG_TYPE;
typedef list<FLAG_TYPE> FLAG_LIST;

//unless set all task will time out after this time...
//very important for safety
#define MOOS_DEFUALT_TIMEOUT 100

/**
 * FIXME: needs class description
 */
class CSGMOOSBehaviour  
{
public:
    CSGMOOSBehaviour();
    virtual ~CSGMOOSBehaviour();

    virtual void SetTime(double dfTimeNow);
    virtual bool ReInitialise();
    bool Start();
    void SetPriority(int nPriority);
    int GetPriority();
    virtual void SetName(string sName);
    double GetCreationTime();
    double GetStartTime();
    bool HasNewRegistration();
    void SetMissionFileReader(CProcessConfigReader* pMissionFileReader);
    string GetName();
    virtual bool SetParam(string sParam, string sVal);
    virtual bool GetNotifications(MOOSMSG_LIST & List);
    virtual bool GetRegistrations(STRING_LIST &List);
    virtual bool OnNewMail(MOOSMSG_LIST & NewMail);
    virtual bool RegularMailDelivery(double dfTimeNow);
    virtual bool Run(CPathAction & DesiredAction);

    void ProcessFlags();
    void ClearFlags();
    bool HasFlags() const;
    string GetChartNodeEntry();
    string GetChartEdgeEntry();

    bool ShouldRun();
    void NotTopLevel() {m_bTopLevel=false;}


protected:
    virtual bool OnStart();
    virtual bool Initialise();

    bool OnComplete();
    bool OnError(string sReason);
    bool OnTimeOut();

    bool DebugNotify(const string & sStr);
    double m_dfIterateTime;
//    virtual bool OnEvent(const string & sReason="",bool bVerbalNotify = true);
    virtual void Stop(const string & sReason="DONE\n");
    
    bool PeekMail(MOOSMSG_LIST & Mail,const string & sKey,CMOOSMsg & Msg);
    unsigned int m_nPriority;
    
    //list of messages to be sent to the outside world
    //when given the chance
    MOOSMSG_LIST m_Notifications;
    //STRING_LIST m_Registrations;
    
    CProcessConfigReader* m_pMissionFileReader;
    
    /**
        * Control structure for a single Degree Of Freedom
        */
    class ControlledDOF
    {
    public:
        ControlledDOF();
        bool IsStale(double dfTimeNow, double dfTaskStartTime, double dfTimeOut=10.0) const;
        bool IsValid() const;
        bool IsSet() const;
        double GetCurrent() const;
        double GetDesired() const;
        double GetErrorTime() const;
        double GetError() const;
        void SetTolerance(double dfTol);
        void SetDesired(double dfDesired);
        double GetDT();
        void SetCurrent(double dfCurrent, double dfTime);
        
    protected:
        double  m_dfDesired;
        double  m_dfCurrent;
        double  m_dfCurrentTime;
        double  m_dfTolerance;
        bool    m_bDesiredSet;
    };
    
    virtual void GetChartNodeContents(STRING_LIST& contents) const {}

    //status variables
    string      m_sName;
    string      m_sClassName;
    string      m_sLogPath;
    bool        m_bActive;
    bool        m_bComplete;
    bool        m_bNewRegistrations;
    

    void ThrowFlag(FLAG_TYPE flag);
    FLAG_TYPE m_sStartFlag;
    FLAG_TYPE m_sCompleteFlag;
    FLAG_TYPE m_sTimeoutFlag;
    FLAG_TYPE m_sErrorFlag;
//    string m_EventFlag;

    double m_dfTimeOut;
    double m_dfStartTime;
    double m_dfCreationTime;
    
    bool m_bInitialised;
    bool m_bTopLevel;

  private:
    static FLAG_LIST m_Flags;
};

typedef list<CSGMOOSBehaviour*> TASK_LIST;
typedef map<string,CSGMOOSBehaviour*> TASK_MAP;


#endif // !defined(AFX_MOOSBEHAVIOUR_H__1C10DD47_7690_4AEF_9174_0B0EA068A77D__INCLUDED_)
