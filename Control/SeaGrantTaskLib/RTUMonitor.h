/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Ian Katz & others at MIT Sea Grant.
    contact: ijk5@mit.edu

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
// RTUMonitor.h: interface for the CRTUMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(RTU_MONITOR_H)
#define RTU_MONITOR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MOOSBehaviour.h"
#include <math.h>

/**
 * FIXME: needs class description
 */
class CRTUMonitor : public CSGMOOSBehaviour  
{
public:
    CRTUMonitor();
    virtual ~CRTUMonitor();
    
    virtual bool GetRegistrations(STRING_LIST &List);
    virtual bool SetParam(string sParam, string sVal);
    virtual bool OnNewMail(MOOSMSG_LIST &NewMail);
    virtual bool Run(CPathAction &DesiredAction);
    virtual bool RegularMailDelivery(double);

protected:
    //array pointer to our circular buffer
    double* m_dfDesired;
    double* m_dfActual;
    //double* m_dfError;
    
    double m_dfMaxDelay;
    double m_dfPeakError;
    double m_dfStuckErrorRange;
    double m_dfLastUpdate;

    void Cleanup();

    int m_iDatapoints;

    int m_iDataStart;
    int m_iDataEnd;

    int DataLength() {return (m_iDataEnd - m_iDataStart + m_iDatapoints) % m_iDatapoints + 1;}

    bool m_bGotInitialDesired;
    bool m_bGotInitialActual;

    bool GotInitialData() {return m_bGotInitialDesired && m_bGotInitialActual;}

    int incr(int index) {return (index + 1) % m_iDatapoints;}
    int decr(int index) {return (index - 1 + m_iDatapoints) % m_iDatapoints;}

    virtual void GetChartNodeContents(STRING_LIST& contents) const;


public:

    double FoldWrappedArray(double (*f)(CRTUMonitor* r, double acc, int index), 
                            double init);

    
    //work functions
    static double SumError(CRTUMonitor* r, double acc, int index) 
    {return acc + fabs(r->m_dfDesired[index] - r->m_dfActual[index]);}

    static double MaxDesired(CRTUMonitor* r, double acc, int index)
    {return fmax(acc, r->m_dfDesired[index]);}

    static double MinDesired(CRTUMonitor* r, double acc, int index)
    {return fmin(acc, r->m_dfDesired[index]);}

    static double MaxActual(CRTUMonitor* r, double acc, int index)
    {return fmax(acc, r->m_dfActual[index]);}

    static double MinActual(CRTUMonitor* r, double acc, int index)
    {return fmin(acc, r->m_dfActual[index]);}


    //static double AboveThreshold(CRTUMonitor* r, double acc, int index);


};

#endif //RTU_MONITOR_H
