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

    SGMOOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SGMOOS.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/
// PID.cpp: implementation of the CPID class.
//
//////////////////////////////////////////////////////////////////////


#include "PID.h"
#include <math.h>
#include <iostream>
#include <iomanip>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPID::CPID()
{
    m_dfKp              = 0;
    m_dfKd              = 0;
    m_dfKi              = 0;
    m_dfe               = 0;
    m_dfeSum            = 0;
    m_dfeOld            = 0;
    m_dfCalculatedDiff  = 0;
    m_dfDT              = 0;
    m_dfOldTime         = 0;
    m_dfOut             = 0;
    m_nIterations       = 0;
    m_dfIntegralLimit   = 0;
    m_dfOutputLimit     = 0;

    m_nHistorySize      = 20;
    m_dfGoal            = 0;

    m_bRunSincePublished = false;
    m_dfMeasuredDiffTimestamp = 0.0;
}


CPID::CPID(double dfKp,
           double dfKd,
           double dfKi,
           double dfIntegralLimit,
           double dfOutputLimit)
{
    m_dfKp              = dfKp;
    m_dfKd              = dfKd;
    m_dfKi              = dfKi;
    m_dfeSum            = 0;
    m_dfeOld            = 0;
    m_dfCalculatedDiff  = 0;
    m_dfDT              = 0;
    m_dfOldTime         = 0;
    m_dfe               = 0;
    m_dfOut             = 0;

    m_dfIntegralLimit   = dfIntegralLimit;
    m_dfOutputLimit     = dfOutputLimit;

    m_nIterations       = 0;

    m_bRunSincePublished = false;

}

// Copy constructor.
CPID::CPID(const CPID& pid)
{
    *this = pid; //use assignment operator
}

CPID::~CPID()
{

}

// assignment operator
CPID& CPID::operator = (const CPID& pid)
{
    SetGains(pid.m_dfKp, pid.m_dfKd, pid.m_dfKi);
    SetLimits(pid.m_dfIntegralLimit, pid.m_dfOutputLimit);
    SetName(pid.m_sName);
    return *this;
}

bool CPID::Run(double dfeIn, double dfErrorTime, double &dfOut)
{
    m_dfe = dfeIn;
    
    //figure out time increment...
    if(m_nIterations++ != 0)
    {
        
        m_dfDT = dfErrorTime - m_dfOldTime;

        if(m_dfDT < 0)
        {
            MOOSTrace("CPID::Run() : negative or zero sample time\n");
            return false;
        }
        else if(m_dfDT == 0)
        {
            //nothing to do...
            dfOut = m_dfOut;
            m_bRunSincePublished = false;
            return true;
        }
        CalculateDerivative(dfeIn);
    }
    else
    {
        //this is our first time through
        m_dfCalculatedDiff = 0;
    }
    
    // Use measured value if its newer than 2 seconds
    // calculated otherwise
    m_dfeDiff = (MOOSTime() - m_dfMeasuredDiffTimestamp < 2.0) 
        ? m_dfMeasuredDiff 
        : m_dfCalculatedDiff;
    
    if(m_dfKi > 0)
    {
        //calculate integral term  
        m_dfeSum += m_dfKi * m_dfe * m_dfDT;
        
        //prevent integral wind up...
        if(fabs(m_dfeSum) >= fabs(m_dfIntegralLimit))
        {
            int nSign = (int)(fabs(m_dfeSum) / m_dfeSum);
            m_dfeSum = nSign * fabs(m_dfIntegralLimit);
        }
    }
    else
    {
        m_dfeSum = 0;
    }
    
    //do pid control
    m_dfOut = m_dfKp * m_dfe
        + m_dfKd * m_dfeDiff 
        + m_dfeSum; //note Ki is already in dfeSum
    
    
    //prevent saturation..
    if(fabs(m_dfOut) >= fabs(m_dfOutputLimit))
    {        
        int nSign = (int)(fabs(m_dfOut) / m_dfOut);
        m_dfOut = nSign * fabs(m_dfOutputLimit);
    }
    
    //save old value..
    m_dfeOld    = m_dfe;
    m_dfOldTime = dfErrorTime;
    
    
    dfOut = m_dfOut;
    
    
    return (m_bRunSincePublished = true);
}

void CPID::CalculateDerivative(double dfeIn)
{
    //figure out differential
    double dfDiffNow = (dfeIn - m_dfeOld) / m_dfDT;
    m_DiffHistory.push_front(dfDiffNow);
    while(m_DiffHistory.size() >= m_nHistorySize)
    {
        m_DiffHistory.pop_back();
    }
    
    m_dfCalculatedDiff = 0;
    for(list<double>::iterator p = m_DiffHistory.begin(); p != m_DiffHistory.end(); p++)
    {
        m_dfCalculatedDiff += *p;   
    }
    m_dfCalculatedDiff /= m_DiffHistory.size();
    
}

void CPID::SetGains(double dfKp, double dfKd, double dfKi)
{
    m_dfKp      =   dfKp;
    m_dfKd      =   dfKd;
    m_dfKi      =   dfKi;
}

void CPID::SetLimits(double dfIntegralLimit, double dfOutputLimit)
{
   m_dfIntegralLimit = dfIntegralLimit;
   m_dfOutputLimit   = dfOutputLimit;
}

void CPID::SetName(const std::string sName)
{
    m_sName = sName;
}

void CPID::SetGoal(double dfGoal)
{
    m_dfGoal = dfGoal;
}


bool CPID::GetNotifications(MOOSMSG_LIST& List)
{
    if(!m_bRunSincePublished)
        return false;

    string sPrefix = m_sName;
    MOOSToUpper(sPrefix);
    sPrefix += (string)"_PID_";
    
    string sVar = sPrefix+(string)"KP";
    List.push_back(CMOOSMsg(MOOS_NOTIFY, sVar.c_str(), m_dfKp));

    sVar = sPrefix + "KD";
    List.push_back(CMOOSMsg(MOOS_NOTIFY, sVar.c_str(), m_dfKd));

    sVar = sPrefix + "KI";
    List.push_back(CMOOSMsg(MOOS_NOTIFY, sVar.c_str(), m_dfKi));

    sVar = sPrefix + "DESIRED";
    List.push_back(CMOOSMsg(MOOS_NOTIFY, sVar.c_str(), m_dfGoal));

    sVar = sPrefix + "ERROR";
    List.push_back(CMOOSMsg(MOOS_NOTIFY, sVar.c_str(), m_dfe));

    sVar = sPrefix + "DERIV";
    List.push_back(CMOOSMsg(MOOS_NOTIFY, sVar.c_str(), m_dfeDiff));

    sVar = sPrefix + "INTEGRAL";
    List.push_back(CMOOSMsg(MOOS_NOTIFY, sVar.c_str(), m_dfeSum/m_dfKi));

    m_bRunSincePublished = false;
    return true;
}

bool CPID::ReadConfig(CProcessConfigReader& config)
{
    string sNameUpper = m_sName;
    MOOSToUpper(sNameUpper);

    config.GetConfigurationParam(MOOSFormat("%s_PID_DERIV_VAR", sNameUpper.c_str()),m_sDerivativeVariable);

    // New way, Read in a vector of gains as a single param.
    // [ proportional, derivative, integral, integral_limit, output_limit ]
    vector<double> GainsVector;
    int nVecRows, nVecColumns = 0;

    if(config.GetConfigurationParam(MOOSFormat("%s_PID", sNameUpper.c_str()), GainsVector, nVecRows, nVecColumns))
    {
        if (GainsVector.size() == 5)
        //if(nVecRows==5 && nVecColumns==1)  // was there the proper # of values?
        {
            SetGains(GainsVector[0], GainsVector[1], GainsVector[2]);
            SetLimits(GainsVector[3], GainsVector[4]);
            return true;
        }
        else 
        {
            MOOSTrace("%s_PID_PARAMS has wrong number of elements, needs 5.\n", m_sName.c_str());
            return false;
        }
    }
    
    // Plan B: the old way, every gain has its own param in config line.
    const string sGain[5] = {"KP","KD","KI","INTEGRAL_LIMIT","MAX"};
    int nParamsRead = 0;
    
    for(int i = 0; i < 5; i++)
    {
        string sParamName = MOOSFormat("%s_PID_%s", m_sName.c_str(), sGain[i].c_str());
        if(config.GetConfigurationParam(sParamName,GainsVector[i]))
            nParamsRead++;
    }
    
    if(nParamsRead == 5)
    {
        SetGains(GainsVector[0], GainsVector[1], GainsVector[2]);
        SetLimits(GainsVector[3], GainsVector[4]);
        return true;
    }

    else if (nParamsRead == 0)
    {
        MOOSTrace("PID for %s not set.  Open loop control only (if any) over this axis.\n", m_sName.c_str());
        return false;
    }

    MOOSTrace("Incorrect # of parameters for %s PID", m_sName.c_str());
    return false;
}

bool CPID::GetRegistrations(STRING_LIST & Reg)
{
    if(UseMeasuredDeriv())
    {
        Reg.push_back(m_sDerivativeVariable);
    }
    return true;
}

bool CPID::OnNewMail(MOOSMSG_LIST &NewMail)
{
    if (!UseMeasuredDeriv())
    {
        return true;
    }
    
    CMOOSMsg Msg;
    double dfNow = MOOSTime();
    
    if(PeekMail(NewMail, m_sDerivativeVariable, Msg))
    {
        if(!Msg.IsSkewed(dfNow))
        {
            m_dfMeasuredDiff = Msg.GetDouble();
            m_dfMeasuredDiffTimestamp = MOOSTime();//Msg.GetTime();
        }
    }
    return true;
}


// RIPPED OFF FROM MOOSCommClient... temporary.
bool CPID::PeekMail(MOOSMSG_LIST &Mail,
                                const std::string &sKey, 
                                CMOOSMsg &Msg,
                                bool bRemove,
                                bool bFindYoungest )
{
    MOOSMSG_LIST::iterator p;
    MOOSMSG_LIST::iterator q = Mail.end();

    double dfYoungest = -1;

    for(p = Mail.begin(); p != Mail.end(); p++)
    {
        if(p->m_sKey == sKey)
        {
            //might want to consider more than one msg....
            if(bFindYoungest)
            {
                if(p->m_dfTime > dfYoungest)
                {
                    dfYoungest = p->m_dfTime;
                    q = p;
                }
            }
            else
            {
                //simply take first
                q=p;
                break;
            }
            
        }
    }
    
    if(q != Mail.end())
    {
        Msg = *q;
        
        if(bRemove)
        {
            //Mail.erase(p);
            Mail.erase(q);
        }
        return true;
    
    }

    return false;
}


bool CPID::UseMeasuredDeriv()
{
    return !m_sDerivativeVariable.empty();
}
