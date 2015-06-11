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
// RTUMonitor.cpp: implementation of the CRTUMonitor class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include "RTUMonitor.h"

//we should never get an angle of 720 in normal operation
#define NULL_ANGLE 720

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRTUMonitor::CRTUMonitor()
{
    m_sClassName ="RTU Monitor";
    m_dfDesired  = NULL;
    m_dfActual   = NULL;
    //m_dfError    = NULL;

}

CRTUMonitor::~CRTUMonitor()
{
    Cleanup();
}

void CRTUMonitor::Cleanup()
{
    MOOSTrace("%s is releasing its dynamically allocated memory... just an FYI...", 
              m_sClassName.c_str());

    //de-allocate arrays
    if (m_dfDesired != NULL) 
    {
        delete m_dfDesired;
        m_dfDesired = NULL;
    }
    if (m_dfActual != NULL) 
    {
        delete m_dfActual;
        m_dfActual = NULL;
    }
    //if (m_dfError != NULL) 
    //{
    //    delete m_dfError;
    //    m_dfError = NULL;
    //}

    MOOSTrace("Done releasing!\n");

    m_bGotInitialDesired = false;  
    m_bGotInitialActual  = false;  

}

bool CRTUMonitor::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;
    double t;

    //put new data at head of arrays

    if (PeekMail(NewMail, "DESIRED_RTU", Msg))
    {
        m_dfDesired[m_iDataStart] = Msg.GetDouble();
        if (!m_bGotInitialDesired)
        {
            MOOSTrace("Got Desired RTU angle: %f\n", m_dfDesired[m_iDataStart]);
        }
        m_bGotInitialDesired = true;
    }

    //repeat for "actual angle"
    if(PeekMail(NewMail, "RTU_POSITION", Msg))
    { 
        t = MOOSTime();
        if(!Msg.IsSkewed(t))
        {
            m_dfActual[m_iDataStart] = Msg.GetDouble();
            m_dfLastUpdate = t;
            if (!m_bGotInitialActual)
            {
                MOOSTrace("Got Actual RTU angle: %f\n", m_dfActual[m_iDataStart]);
            }
            m_bGotInitialActual = true;
        }
    }

    return CSGMOOSBehaviour::OnNewMail(NewMail);

}

bool CRTUMonitor::GetRegistrations(STRING_LIST &List)
{

    // List.push_front(VARIABLES WE SUBSCRIBE TO);
    List.push_front("DESIRED_RTU");
    List.push_front("RTU_POSITION");
    
    //always call base class version
    CSGMOOSBehaviour::GetRegistrations(List);

    return true;
}


bool CRTUMonitor::Run(CPathAction &DesiredAction)
{
    int nextStart, nextEnd;
    char sNum[255];

    if (!GotInitialData())
    {
        //continue to hold... your RTU is important to us
        return true;
    } 

    if (m_iDataStart != m_iDataEnd)
    {
        //sustain desired value if there has been no update
        if (NULL_ANGLE == m_dfDesired[m_iDataStart])
        {
            m_dfDesired[m_iDataStart] = m_dfDesired[incr(m_iDataStart)];
        }
        
        //sustain actual value if there has been no update
        if (NULL_ANGLE == m_dfActual[m_iDataStart])
        {
            m_dfActual[m_iDataStart] = m_dfActual[incr(m_iDataStart)];
        }
        
    }
    else if (NULL_ANGLE == m_dfDesired[m_iDataStart]
             || NULL_ANGLE == m_dfActual[m_iDataStart])
    {
        MOOSTrace("Error! no new data and no previous data to draw from!\n");
    }
    

    //calculate cumulative error and place in error[0]
    double dfError = FoldWrappedArray(&CRTUMonitor::SumError, 0.0);
    MOOSTrace("Current error: %03.6f\tCumulative error: %03.6f\tAverage: %03.6f\n",
              fabs(m_dfDesired[m_iDataStart] - m_dfActual[m_iDataStart]),
              dfError, dfError / DataLength());
        

    //get all 4 max/min values
    double dfMaxDesired = FoldWrappedArray(&CRTUMonitor::MaxDesired, -720);
    double dfMinDesired = FoldWrappedArray(&CRTUMonitor::MinDesired, 720);
    double dfMaxActual  = FoldWrappedArray(&CRTUMonitor::MaxActual,  -720);
    double dfMinActual  = FoldWrappedArray(&CRTUMonitor::MinActual,  720);

    //if desired range is outside actual range, check our stuck threshold
    if ((dfMinDesired > dfMaxActual + m_dfStuckErrorRange) 
        || (dfMaxDesired < dfMinActual - m_dfStuckErrorRange))
    {
        if (dfMaxActual - dfMinActual < m_dfStuckErrorRange)
        {
            sprintf(sNum, "%f to %f", dfMinActual, dfMaxActual);
            DebugNotify((string)"StuckError: actual range is " + sNum + "\n");
            sprintf(sNum, "%f to %f", dfMinDesired, dfMaxDesired);
            DebugNotify((string)"StuckError: deisred range is " + sNum + "\n");
            sprintf(sNum, "%f, which is less than %f", (dfMaxActual - dfMinActual), m_dfStuckErrorRange);
            DebugNotify((string)"StuckError: RTU seems stuck in a range of " + sNum + "\n");
            OnComplete();
        }
    }

    //if cumulative error is above something, abort
    if (m_dfPeakError * m_iDatapoints < dfError)
    {
        DebugNotify("Peak RTU error exceeded!");
        OnComplete();
    }

    //move desired and actual arrays down
    //check if buffer is full
    nextStart = decr(m_iDataStart);

    if (nextStart != m_iDataEnd)
    {
        //simple... just extend
        nextEnd = m_iDataEnd;
    }
    else
    {
        nextEnd = decr(m_iDataEnd);
    }

    m_dfDesired[nextStart] = NULL_ANGLE;
    m_dfActual[nextStart] = NULL_ANGLE;
    //m_dfError[nextStart] = 0;

    m_iDataStart = nextStart;
    m_iDataEnd = nextEnd;

    //MOOSTrace("Buffer now runs from %d to %d\n", m_iDataStart, m_iDataEnd);


    return true;
}

bool CRTUMonitor::RegularMailDelivery(double dfTimeNow)
{
    if (0.0 == dfTimeNow)
        return true;

    //true if no desired angle has been set.  
    //if (!m_bGotInitialDesired)
    //    return true;
    //if it has, we care about updates 

    //MOOSTrace("RTU Monitor input delay: %f > (%f - %f = %f)\n", 
    //          m_dfMaxDelay, dfTimeNow, m_dfLastUpdate, dfTimeNow - m_dfLastUpdate);

    return m_dfMaxDelay > (dfTimeNow - m_dfLastUpdate);
}

bool CRTUMonitor::SetParam(string sParam, string sVal)
{

    //MOOSTrace("RTUMonitor getting conf details: %s = %s\n", sParam.c_str(), sVal.c_str());

    //params will be the thresholds for aborts and the seconds to monitor

    //allocate array desired angle, actual angle, and error
    // --- array length is apptick * number of seconds to monitor

        
    MOOSToUpper(sParam);
    
    //if the error is ever over this amount, bomb out
    if(sParam == "PEAK_ERROR_DEGREES")
    {
        m_dfPeakError = atof(sVal.c_str());
        MOOSTrace(string("Maximum error is ") + sVal + " degrees (average)\n");
        return true;
    }

    //if RTU doesn't leave a range of this size, and it SHOULD HAVE, bomb out
    if (sParam == "STUCK_ERROR_DEGREES")
    {
        m_dfStuckErrorRange = atof(sVal.c_str());
        MOOSTrace(string("Range of 'stuck' is ") + sVal + " degrees\n");
        return true;
    }

    //max amount of time to wait for data to come from rtu_position
    if (sParam == "MAX_DELAY")
    {
        m_dfMaxDelay = atof(sVal.c_str());
        MOOSTrace(string("Max Delay on RTU data is ") + sVal + " seconds\n");
        return true;
    }
   
    //number of data points to use
    if(sParam == "BUFFER_LENGTH")
    {
        m_iDatapoints = atoi(sVal.c_str());
        MOOSTrace(string("Buffer length is ") + sVal + " points\n");

        Cleanup();
   
        m_dfDesired = new double[m_iDatapoints];
        m_dfActual = new double[m_iDatapoints];
        //m_dfError = new double[m_iDatapoints];

        m_iDataStart = 0;
        m_iDataEnd = 0;

        return true;
    }



    return CSGMOOSBehaviour::SetParam(sParam, sVal);

}


void CRTUMonitor::GetChartNodeContents(STRING_LIST& contents) const
{
    contents.push_back(MOOSFormat("Peak Error: ±%f°", m_dfPeakError));
    contents.push_back(MOOSFormat("'Stuck' range: ±%f°", m_dfStuckErrorRange));
}



//iterate over a circular array to produce a single result
double CRTUMonitor::FoldWrappedArray(double (*f)(CRTUMonitor* r, double acc, int index), 
                                     double init)
{
    int i;

    if (m_iDataEnd >= m_iDataStart)
    {
        //easy case... straight iterate
        for (i = m_iDataStart; i <= m_iDataEnd; ++i)
        {
            init = (*f)(this, init, i);
        }
    }
    else
    {
        //iterate from start to the end of the array
        for (i = m_iDataStart; i < m_iDatapoints; ++i)
        {
            init = (*f)(this, init, i);
        }

        for (i = 0; i <= m_iDataEnd; ++i)
        {
            init = (*f)(this, init, i);
        }
    }
    
    return init;
}


