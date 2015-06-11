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
// PID.h: interface for the CPID class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PID_H__0DC4321A_A987_4499_8902_AAE9F515E921__INCLUDED_)
#define AFX_PID_H__0DC4321A_A987_4499_8902_AAE9F515E921__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <list>
#include <fstream>
#include <MOOSGenLib/MOOSGenLib.h> 
#include <MOOSLIB/MOOSLib.h>
//using namespace std;

/**
 * FIXME: needs class description
 */
class CPID  
{
public:
    void SetGoal(double dfGoal);
    void SetName(const std::string sName);
    void SetLimits(double dfIntegralLimit, double dfOutputLimit);
    bool Run(double dfeIn,double dfErrorTime,double & dfOut);
    uint m_nHistorySize;

    std::list<double> m_DiffHistory;
    CPID();
    CPID( double dfKp,
          double dfKd,
          double dfKi,
          double dfIntegralLimit,
          double dfOutputLimit);
    
    // copy constructor
    CPID(const CPID& pid);
    
    virtual ~CPID();
    //assignment operator, necessary because ofstream data member
    CPID& operator=(const CPID& pid);

    void SetGains(double dfKp,double dfKd,double dfKi);
    bool ReadConfig(CProcessConfigReader& config);
    bool GetNotifications(MOOSMSG_LIST & List);
    bool GetRegistrations(STRING_LIST & Reg);
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    void CalculateDerivative(double dfeIn);


protected:

    bool UseMeasuredDeriv();

    double m_dfKi;
    double m_dfKd;
    double m_dfKp;
    double m_dfe;
    double m_dfeSum;
    double m_dfeOld;
    double m_dfeDiff;
    double m_dfDT;
    double m_dfOldTime;
    double m_dfOut;
    double m_dfIntegralLimit;
    double m_dfOutputLimit;

    bool m_bRunSincePublished;
    std::string m_sName;
    double m_dfGoal;
    uint m_nIterations;

    std::string m_sDerivativeVariable;
    double m_dfMeasuredDiff;
    double m_dfCalculatedDiff;
    double m_dfMeasuredDiffTimestamp;
 
   static bool PeekMail(MOOSMSG_LIST &Mail, 
                        const std::string &sKey, 
                        CMOOSMsg &Msg,
                        bool bErase = false, 
                        bool bFindYoungest = false);

};

#endif // !defined(AFX_PID_H__0DC4321A_A987_4499_8902_AAE9F515E921__INCLUDED_)
