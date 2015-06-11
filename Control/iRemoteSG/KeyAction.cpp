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
#include <iostream>
#include "KeyAction.h"

/******************************************************************************
KeyAction Abstract Base Class:
    Defines the interface for all all children & provides some default behaviors.
******************************************************************************/

CKeyAction::CKeyAction()
{
    m_bConfirmBeforeActing=false;
}

bool CKeyAction::OnKeystroke(MOOSVARMAP& Vars)
{
    MOOSTrace("Pressed [%s]\n", m_sDescription.c_str());
    return true;
}

bool CKeyAction::ReadConfig(std::string& sConfig)
{
    if(ParseConfigString(sConfig))
    {
        ReadOptions(sConfig);
        return true;
    }
    return false;
}

bool CKeyAction::ParseConfigString(std::string& sConfig)
{

    string sTemp;

    //Read Key Char
    sTemp = MOOSChomp(sConfig, "@");
    if(sTemp.size()==1)
        m_cKey=sTemp[0];
    else 
    {
        MOOSTrace("Invalid \"Key\" in KeyAction config %s)\n", sTemp.c_str());
        return false;
    }

    // Required Text Description
    sTemp = MOOSChomp(sConfig, ":");
    if(sTemp.empty())
        return false;

    m_sDescription = sTemp;
    
    return true;
}

void CKeyAction::ReadOptions(std::string& sConfig)
{
    while(!sConfig.empty())
    {
        string sOption = MOOSChomp(sConfig, ",");
        
        if(MOOSStrCmp(sOption, "CONFIRM"))
            m_bConfirmBeforeActing=true;
        else
            MOOSTrace("Unknown Option: %s\n", sOption.c_str());
    }
}

bool CKeyAction::Confirm()
{
    if(!m_bConfirmBeforeActing)
        return true;

    PrintConfirmMessage();

    int nConfirm = MOOSGetch();
    bool bConfirm = (char)nConfirm == 'y';
    MOOSTrace(bConfirm ? "Confirmed\n" : "Cancelled\n");
    return bConfirm;
}

void CKeyAction::PrintConfirmMessage()
{
    MOOSTrace("Should I do it?\n");
}

/******************************************************************************
CustomStringKeyAction Class:
    Posts a user specified (typed into iRemote) string value into the DB.
******************************************************************************/

CCustomStringKeyAction::CCustomStringKeyAction()
{
    m_sPrompt="Enter String (no spaces, empty to cancel): ";
}

bool CCustomStringKeyAction::ParseConfigString(std::string& sConfig)
{
    if(!CKeyAction::ParseConfigString(sConfig))
        return false;

    if(!ReadMOOSVar(sConfig))
        return false;

    return true;
}

bool CCustomStringKeyAction::OnKeystroke(MOOSVARMAP& Vars)
{
    CKeyAction::OnKeystroke(Vars);

    MOOSTrace(m_sPrompt);
    cin >> m_sValue;
    char cDiscardedNewline = MOOSGetch();

    (void) cDiscardedNewline; //avoid compiler warning

    if(Confirm())
        return PublishValue(Vars);
    else
        return true;

}


bool CCustomStringKeyAction::ReadMOOSVar(std::string& sConfig)
{
    string sTemp;
    sTemp = MOOSChomp(sConfig, ",");

    if(sTemp.empty())
        return false;

    m_sMOOSVar = sTemp;

    return true;
}

bool CCustomStringKeyAction::PublishValue(MOOSVARMAP& Vars)
{
    if(Vars.find(m_sMOOSVar)!=Vars.end() || m_sValue.empty())
    {
        return Vars[m_sMOOSVar].Set(m_sValue, MOOSTime());
    }
    return false;

}

bool CCustomStringKeyAction::GetPublications(STRING_LIST& pub)
{
    if(m_sMOOSVar.empty())
        return false;

    pub.push_back(m_sMOOSVar);
    return true;
}


void CCustomStringKeyAction::PrintConfirmMessage()
{
    MOOSTrace("Really set %s to \"%s\"? ", m_sMOOSVar.c_str(), m_sValue.c_str());
}

/******************************************************************************
StringKeyAction Class:
    Posts a constant string value into the DB.
******************************************************************************/
bool CStringKeyAction::ParseConfigString(std::string& sConfig)
{
    if(!CCustomStringKeyAction::ParseConfigString(sConfig))
        return false;

    if(sConfig.empty())
        return false;

    m_sValue=MOOSChomp(sConfig,",");

    return true;
    
}


bool CStringKeyAction::OnKeystroke(MOOSVARMAP& Vars)
{
    CKeyAction::OnKeystroke(Vars);

    if(Confirm())
        return PublishValue(Vars);

    return true;
}



/******************************************************************************
NumericKeyAction Class:
    Posts a constant string value into the DB.
******************************************************************************/

bool CNumericKeyAction::ParseConfigString(std::string& sConfig)
{
    // Required Text Description
    if(!CCustomStringKeyAction::ParseConfigString(sConfig))
        return false;

    if(sConfig.empty())
        return false;

    m_sValue=MOOSChomp(sConfig,",");

    if(!MOOSIsNumeric(m_sValue))
    {
        MOOSTrace("Trying to initialize numeric key action to non-numeric type\n");
        return false;
    }

    m_dfValue=atof(m_sValue.c_str());

    return true;
}

bool CNumericKeyAction::OnKeystroke(MOOSVARMAP& Vars)
{
    CKeyAction::OnKeystroke(Vars);
    
    if(Confirm())
        return PublishValue(Vars);
    return true;
}

/******************************************************************************
CustomNumericKeyAction Class:
    Posts a user specified (typed into iRemote) string value into the DB.
******************************************************************************/
bool CCustomNumericKeyAction::OnKeystroke(MOOSVARMAP& Vars)
{
    CKeyAction::OnKeystroke(Vars);

    MOOSTrace("Enter Numeric Value: ");
    cin >> m_sValue;
    char cDiscardedNewline = MOOSGetch();
    (void) cDiscardedNewline; //avoid compiler error

    if(!m_sValue.empty() && MOOSIsNumeric(m_sValue))
    {
        if(Confirm())
        {
            m_dfValue=atof(m_sValue.c_str());
            return PublishValue(Vars);
        }
        return true;
    }
    return false;
}

bool CCustomNumericKeyAction::PublishValue(MOOSVARMAP& Vars)
{
    if(Vars.find(m_sMOOSVar)!=Vars.end() || m_sValue.empty())
    {
        return Vars[m_sMOOSVar].Set(m_dfValue, MOOSTime());
    }
    return false;

}

void CCustomNumericKeyAction::PrintConfirmMessage()
{
    MOOSTrace("Really set %s to \"%f\"? ", m_sMOOSVar.c_str(), m_dfValue);
}

/******************************************************************************
NumericStepKeyAction Class:
    Reads A Variable, increments it by a value, & re-posts it.
******************************************************************************/

CNumericStepKeyAction::CNumericStepKeyAction()
{
    m_bDoWrap=false;
}

bool CNumericStepKeyAction::ParseConfigString(std::string& sConfig)
{
    if(!CCustomNumericKeyAction::ParseConfigString(sConfig))
        return false;

    // Read Step
    string sTemp = MOOSChomp(sConfig, ",");

    if(sTemp.empty())
        return false;

    if(MOOSIsNumeric(sTemp))
        m_dfStep = atof(sTemp.c_str());
    else
        return false;

    //Read Wrap (optional)
    sTemp = MOOSChomp(sConfig, ",");

    if(!sTemp.empty())
        if(MOOSIsNumeric(sTemp))
        {
            m_bDoWrap = true;
            m_dfWrapAt = atof(sTemp.c_str());
        }

    return true;
}

bool CNumericStepKeyAction::OnKeystroke(MOOSVARMAP& Vars)
{
    CKeyAction::OnKeystroke(Vars);

    MOOSVARMAP::iterator MyVar;
    if((MyVar=Vars.find(m_sMOOSVar))!=Vars.end())
    {
        CMOOSVariable& V = MyVar->second;
        if(V.IsDouble() && (V.GetTime() > 0.0))
        {
            m_dfValue = V.GetDoubleVal() + m_dfStep;

            if(m_bDoWrap)
            {
                if(m_dfValue > m_dfWrapAt)
                    m_dfValue = -m_dfWrapAt + (m_dfValue-m_dfWrapAt);
                else if (m_dfValue <= -m_dfWrapAt)
                    m_dfValue = m_dfWrapAt - (-m_dfValue-m_dfWrapAt);
            }
            
            if(Confirm())
                return V.Set(m_dfValue, MOOSTime());
            else
                return true;
        }
    }
    return false;

}

/******************************************************************************
SummaryKeyAction Class:
    Posts a summary of MOOS Vars..
******************************************************************************/

bool CSummaryKeyAction::ParseConfigString(std::string& sConfig)
{
    // Required Text Description
    if(!CKeyAction::ParseConfigString(sConfig))
        return false;

    while(!sConfig.empty())
        m_Subscriptions.push_back(MOOSChomp(sConfig,","));

    return !m_Subscriptions.empty();
}

bool CSummaryKeyAction::OnKeystroke(MOOSVARMAP& Vars)
{
    CKeyAction::OnKeystroke(Vars);

    STRING_LIST::iterator i;
    for(i=m_Subscriptions.begin(); i!=m_Subscriptions.end(); i++)
    {
        cerr << *i;
        cerr << ": ";
        
        if(Vars.find(*i)!=Vars.end())
        {

            if(Vars[*i].GetTime()<0)
                cerr << "**********************";
            else
            {
                //print value
                if(Vars[*i].IsDouble())
                    cerr << Vars[*i].GetDoubleVal();
                else
                    cerr << Vars[*i].GetStringVal();
    

                //print age
                cerr << "  (" << Vars[*i].GetAge(MOOSTime()) << "s)";
            }
        }
        else
            cerr << "Not Found";

        cerr << endl;
    }
    return true;
}

bool CSummaryKeyAction::GetRegistrations(STRING_LIST& reg)
{
    STRING_LIST::iterator i;
    for(i=m_Subscriptions.begin(); i!=m_Subscriptions.end(); i++)
    {
        reg.push_back(*i);
    }
    return true;
}


/******************************************************************************
QuitKeyAction Class:
    Quits the currently running App (iRemote)
******************************************************************************/

CQuitKeyAction::CQuitKeyAction()
{
    m_bConfirmBeforeActing = true;
}

bool CQuitKeyAction::OnKeystroke(MOOSVARMAP& Vars)
{

    CKeyAction::OnKeystroke(Vars);

    if(Confirm())
    {
        MOOSTrace("Quitting...\n");
        exit(0);
    }
    return true;

}

void CQuitKeyAction::PrintConfirmMessage()
{
    MOOSTrace("Really Quit? ");
}

/******************************************************************************
HelpKeyAction Class:
    Lists the used keys & their bindings.
******************************************************************************/

CHelpKeyAction::CHelpKeyAction(KEY_MAP* pKeys)
{
    m_pKeys=pKeys;
}

bool CHelpKeyAction::OnKeystroke(MOOSVARMAP& Vars)
{

    CKeyAction::OnKeystroke(Vars);

    if(m_pKeys==NULL)
        return false;

    KEY_MAP_ITER i;
    for(i=m_pKeys->begin(); i!=m_pKeys->end(); i++)
    {
        MOOSTrace("\'%c\' - %s\n", i->first, i->second->GetDescription().c_str());
    }
    return true;
}

