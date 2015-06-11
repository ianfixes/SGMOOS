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
// PathAction.cpp: implementation of the CPathAction class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>


#include "PathAction.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPathAction::CPathAction()
{

}

CPathAction::~CPathAction()
{

}

bool CPathAction::AllStop(const char* sTag, int nPriority)
{
    bool bOk = true;
    for(int i=0; i<6; i++)
        bOk &= SetOpenLoop((WhichDOF)i, 0.0, nPriority, sTag);
    
    return bOk;
}

bool CPathAction::DirectionSanityCheck(WhichDOF nDirection) const
{
    if((uint)nDirection >= sizeof(m_DOF) / sizeof(m_DOF[0]))
    {
        printf("CPathAction::Set() Hey! Subscript out of range! what kind of compiler is this?\n");
        return false;
    }

    return true;
}

bool CPathAction::LoopSanityCheck(WhichDOF nDirection, uint nPriority)
{

    if (!DirectionSanityCheck(nDirection))
        return false;


    if(nPriority >= m_DOF[nDirection].m_nPriority)
        return false;

    if(m_DOF[nDirection].m_nPriority == nPriority)
    {
        printf("CPathAction::Set() Squabbling twins!\n");
    }

    return true;

}

bool CPathAction::SetOpenLoop(WhichDOF nDirection, double dfVal, int nPriority,  const char* sTag)
{

    if (!LoopSanityCheck(nDirection, nPriority)) 
        return false;

    m_DOF[nDirection].m_dfVal = dfVal;
    m_DOF[nDirection].m_nPriority = nPriority;
    m_DOF[nDirection].m_bClosedLoop = false;
    m_DOF[nDirection].m_sTag = sTag;
    return true;


}

bool CPathAction::SetClosedLoop(WhichDOF nDirection, double dfError, double dfGoal, int nPriority,  const char* sTag)
{

    if (!LoopSanityCheck(nDirection, nPriority)) 
        return false;
    
    m_DOF[nDirection].m_dfGoal=dfGoal;
    m_DOF[nDirection].m_dfVal = dfError;
    m_DOF[nDirection].m_nPriority = nPriority;
    m_DOF[nDirection].m_bClosedLoop = true;
    m_DOF[nDirection].m_sTag = sTag;
    return true;

}

bool CPathAction::RunPID(WhichDOF nDirection, CPID* pid)
{
    // If we're running open loop, no need to run PID.
    if(!m_DOF[nDirection].m_bClosedLoop)
        return true;

    // If we're running closed loop, and there's no PID, that's a PROBLEM!
    if(pid == NULL)
        return false;

    pid->SetGoal(m_DOF[nDirection].m_dfGoal);  //For logging purposes

    double dfForce;
    if(pid->Run(m_DOF[nDirection].m_dfVal, MOOSTime(), dfForce))
    {
        m_DOF[nDirection].m_dfVal = dfForce;
        m_DOF[nDirection].m_bClosedLoop = false;
        return true;
    }
    else
    {
        m_DOF[nDirection].m_dfVal = 0.0;  // Command no force on this axis
        return false;
    }
}

double CPathAction::GetDesiredForce(WhichDOF nDirection) const
{
    if (DirectionSanityCheck(nDirection))
        return m_DOF[nDirection].m_dfVal;
    else
        return 0;

}

string CPathAction::GetTag(WhichDOF nDirection) const
{
    if (DirectionSanityCheck(nDirection))
        return m_DOF[nDirection].m_sTag;

    printf("CPathAction::Set() Hey! Subscript out of range! what kind of compiler is this?\n");
    return "error";

}

unsigned int CPathAction::GetPriority(WhichDOF nDirection) const
{
    if (DirectionSanityCheck(nDirection))
        return m_DOF[nDirection].m_nPriority;

    printf("CPathAction::Set() Hey! Subscript out of range! what kind of compiler is this?\n");
    return VERY_LARGE_NUMBER;

}

void CPathAction::Trace() const
{
    printf("Su=%7.3f Task=%s Priority=%d\n",
        m_DOF[SURGE].m_dfVal,
        m_DOF[SURGE].m_sTag.c_str(),
        m_DOF[SURGE].m_nPriority);

    printf("Sw=%7.3f Task=%s Priority=%d\n",
        m_DOF[SWAY].m_dfVal,
        m_DOF[SWAY].m_sTag.c_str(),
        m_DOF[SWAY].m_nPriority);

    printf("He=%7.3f Task=%s Priority=%d\n",
        m_DOF[HEAVE].m_dfVal,
        m_DOF[HEAVE].m_sTag.c_str(),
        m_DOF[HEAVE].m_nPriority);

    printf("Ro=%7.3f Task=%s Priority=%d\n",
        m_DOF[ROLL].m_dfVal,
        m_DOF[ROLL].m_sTag.c_str(),
        m_DOF[ROLL].m_nPriority);

    printf("Pi=%7.3f Task=%s Priority=%d\n",
        m_DOF[PITCH].m_dfVal,
        m_DOF[PITCH].m_sTag.c_str(),
        m_DOF[PITCH].m_nPriority);

    printf("Ya=%7.3f Task=%s Priority=%d\n",
        m_DOF[YAW].m_dfVal,
        m_DOF[YAW].m_sTag.c_str(),
        m_DOF[YAW].m_nPriority);
}

CPathAction::CDegreeOfFreedom::CDegreeOfFreedom():
    m_nPriority(VERY_LARGE_NUMBER),
    m_dfVal(0.0),
    m_bClosedLoop(false),
    m_sTag("None")
{
}
