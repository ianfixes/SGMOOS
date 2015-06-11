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


// SGRemote.cpp: implementation of the CSGRemote class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif


#include <MOOSLIB/MOOSLib.h>
#include <MOOSGenLib/MOOSGenLib.h>
#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include "SGRemote.h"
#include <stdio.h>
#include "../SeaGrantTaskLib/PathAction.h"

using namespace std;


#define WD_PERIOD 0.1
#define HUMAN_TIMEOUT 20.0


#ifdef _WIN32
DWORD WINAPI KSLoopProc( LPVOID lpParameter)
{
    MOOSTrace("starting keystroke thread...");
    CSGRemote* pMe = 	(CSGRemote*)lpParameter;
    return pMe->KeyStrokeLoop();	
}
#else
void * KSLoopProc( void * lpParameter)
{
    MOOSTrace("starting keystroke thread...");
    CSGRemote* pMe = 	(CSGRemote*)lpParameter;
    pMe->KeyStrokeLoop();	
    return NULL;
}
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////

CSGRemote::CSGRemote()
{
    m_dfTimeLastSent = MOOSTime();
    m_dfTimeLastAck  = MOOSTime();
}

CSGRemote::~CSGRemote()
{
    KEY_MAP::iterator i;
    for(i=m_KeyBindings.begin(); i!=m_KeyBindings.end(); i++)
    {
        delete i->second;
    }
    m_KeyBindings.clear();
}


bool CSGRemote::KeyStrokeLoop()
{

    while (true)
    {
        char cCmd = MOOSGetch();
                
        KEY_MAP_ITER k;
        k=m_KeyBindings.find(cCmd);
    
        if(k==m_KeyBindings.end())
        {
            MOOSTrace("No key binding for \'%c\' (%d)\n", cCmd, (int)cCmd);
        }
        else
        {
            if(!k->second->OnKeystroke(m_MOOSVars))
                MOOSTrace("Failed\n");

            PublishFreshMOOSVariables();
        }
    }
    return true;
}

bool CSGRemote::OnNewMail(MOOSMSG_LIST &NewMail)
{
    return UpdateMOOSVariables(NewMail);
}

bool CSGRemote::OnStartUp()
{

    SetDefaultKeyActions();

    if(!MakeCustomKeyActions())
    {
        MOOSTrace("Problem reading key actions\n");
        return false;
    }

    KEY_MAP_ITER i;
    STRING_LIST SubsNPubs;

    for(i = m_KeyBindings.begin(); i!=m_KeyBindings.end(); i++)
    {
        i->second->GetRegistrations(SubsNPubs);
        i->second->GetPublications(SubsNPubs);
    }

    STRING_LIST::iterator var;
    for(var=SubsNPubs.begin(); var!=SubsNPubs.end(); var++)
    {
        AddMOOSVariable(*var, *var, *var, 0.1);

    }
/*
    regs.sort();
    regs.unique();

    for(var=regs.begin(); var!=regs.end(); var++)
    {
        string pubname = "";
        //Check if we're both registering & publishing
        if(pubs.find(*var)!=pubs.end())
        {
            pubname = *var;
            pubs.remove(*var);
        }

        AddMOOSVariable(*var, *var, pubname, 0.1);
    }

    pubs.sort();
    pubs.unique();

    for(var=pubs.begin(); var!=pubs.end(); var++)
    {
        AddMOOSVariable(*var, "", *var, 0.1);
    }
*/
    RegisterMOOSVariables();
    if(!StartKeyStrokeWatcherThread())
    {
        MOOSTrace("Failed to start key stroke watcher thread\n");
        return false;
    }

    MOOSPause(1000);
    MOOSTrace("\n******** Welcome to the MOOS **********\n");

    return true;
}

bool CSGRemote::SetDefaultKeyActions()
{
    CKeyAction* pNewAction;
    string sConfig = " @AllStop:MOOS_MANUAL_OVERIDE,True";

    // Space bar is All Stop
    pNewAction=new CStringKeyAction();
    pNewAction->ReadConfig(sConfig);
    m_KeyBindings[' ']=pNewAction;

    // "R" restarts helm
    pNewAction=new CStringKeyAction();
    sConfig="R@Restart_Helm:RESTART_HELM,True,Confirm";
    pNewAction->ReadConfig(sConfig);
    m_KeyBindings['R']=pNewAction;

    // 'o' relinqishes manual control
    pNewAction=new CStringKeyAction();
    sConfig="o@Start_Mission:MOOS_MANUAL_OVERIDE,False,Confirm";
    pNewAction->ReadConfig(sConfig);
    m_KeyBindings['o']=pNewAction;

    // 'o' relinqishes manual control
    pNewAction=new CStringKeyAction();
    sConfig="V@Restart_Nav:RESTART_NAV,True,Confirm";
    pNewAction->ReadConfig(sConfig);
    m_KeyBindings['V']=pNewAction;

    // 'G' Restarts Logger
    pNewAction=new CStringKeyAction();
    sConfig="V@Restart_Logger:LOGGER_RESTART,True,Confirm";
    pNewAction->ReadConfig(sConfig);
    m_KeyBindings['G']=pNewAction;

    // '*' is Nav Summary
    pNewAction=new CSummaryKeyAction();
    sConfig="*@Nav_Summary:NAV_X,NAV_Y,NAV_Z,NAV_DEPTH,NAV_ALTITUDE,NAV_YAW";
    pNewAction->ReadConfig(sConfig);
    m_KeyBindings['*']=pNewAction;

    // 'q' Quits
    pNewAction=new CQuitKeyAction();
    sConfig="q@Quit";
    pNewAction->ReadConfig(sConfig);
    m_KeyBindings['q']=pNewAction;

    // '?' print help message
    pNewAction=new CHelpKeyAction(&m_KeyBindings);
    sConfig="?@Help";
    pNewAction->ReadConfig(sConfig);
    m_KeyBindings['?']=pNewAction;

    return true;
}

bool CSGRemote::MakeCustomKeyActions()
{
    STRING_LIST sParams;
    
    if(m_MissionReader.GetConfiguration(GetAppName(),sParams))
    {
        STRING_LIST::iterator p;
        
        for(p = sParams.begin();p!=sParams.end();p++)
        {
            CKeyAction* pNewAction;
            string sLine = *p;
            string sTok,sVal;
            m_MissionReader.GetTokenValPair(sLine,sTok,sVal);
            
            MOOSToUpper(sTok);
            if(sTok=="STRINGACTION")
                pNewAction=new CStringKeyAction();
            else if(sTok=="CUSTOMSTRINGACTION")
                pNewAction=new CCustomStringKeyAction();
            else if(sTok=="NUMERICACTION")
                pNewAction=new CNumericKeyAction();
            else if(sTok=="CUSTOMNUMERICACTION")
                pNewAction=new CCustomNumericKeyAction();
            else if(sTok=="NUMERICSTEPACTION")
                pNewAction=new CNumericStepKeyAction();
            else if(sTok=="SUMMARYACTION")
                pNewAction=new CSummaryKeyAction();
            else if(sTok=="QUITACTION")
                pNewAction=new CQuitKeyAction();
            else if(sTok=="HELPACTION")
                pNewAction=new CHelpKeyAction(&m_KeyBindings);
            else // This isn't a vaild action, skip it
                continue;

            if(!pNewAction->ReadConfig(sVal))
            {
                MOOSTrace("ERROR (%s)\n", sLine.c_str());
                return false;
            }
            char cNewKey='\0';
            if((cNewKey=pNewAction->GetKeyBinding())=='\0')
            {
                MOOSTrace("Key Binding Not Set (%s)\n", sLine.c_str());
                return false;
            }

            if(m_KeyBindings.find(cNewKey)!=m_KeyBindings.end())
                MOOSTrace("WARNING: Duplicate Key Action (%c)... Overwriting\n", cNewKey);

            m_KeyBindings[cNewKey]= pNewAction;
            pNewAction=NULL;
            

        }
    }
    return true;
}

bool CSGRemote::StartKeyStrokeWatcherThread()
{
#ifdef _WIN32
    m_hKeyStrokeWatcherThread = ::CreateThread(
        NULL,
        0,
        KSLoopProc,
        this,
        CREATE_SUSPENDED,
        &m_nKeyStrokeWatcherThreadID);

    ResumeThread(m_hKeyStrokeWatcherThread);
    return true;
#else
    return (pthread_create(& m_nKeyStrokeWatcherThreadID,NULL,KSLoopProc,this)==0);
#endif

}
