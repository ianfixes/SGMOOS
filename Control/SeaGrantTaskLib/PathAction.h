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
// PathAction.h: interface for the CPathAction class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHACTION_H__5ED641C5_EBE1_4606_9758_087B8E19F48A__INCLUDED_)
#define AFX_PATHACTION_H__5ED641C5_EBE1_4606_9758_087B8E19F48A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include "PID.h"
using namespace std;

#define VERY_LARGE_NUMBER 1000

const string DOF_Names[6] = {"SURGE", "SWAY", "HEAVE", "ROLL", "PITCH", "YAW"};

/**
 * The various degrees of freedom, enumerated.
 */
enum WhichDOF
{
    SURGE=0,
    SWAY,
    HEAVE,
    ROLL,
    PITCH,
    YAW,

    WhichDOF_min = SURGE,
    WhichDOF_max = YAW,
};

inline WhichDOF operator++(WhichDOF &d, int) 
{
    WhichDOF d2 = d;
    d = (WhichDOF) ((int)d + 1);
    return d2;
}

/**
 * FIXME: needs class description
 */
class CPathAction  
{
public:
    // CONSTRUCTION & DESTRUCTION
    CPathAction(); 
    CPathAction(CPID** m_PIDptrptr);
    virtual ~CPathAction();
    
    ///Print data on each degree of freedom to STDOUT
    void Trace() const;
    unsigned int GetPriority(WhichDOF nDirection) const;
    string GetTag(WhichDOF nDirection) const;
    double GetDesiredForce(WhichDOF nDirection) const;
    bool RunPID(WhichDOF nDirection, CPID* pid);
    double IsClosedLoop(WhichDOF nDirection) const { return m_DOF[nDirection].m_bClosedLoop; }
//    bool Set(WhichDOF nDirection,double dfVal,int nPriority, const char* sTag, bool bClosedLoop=false);

    ///Configure closed-loop operation
    bool SetClosedLoop(WhichDOF nDirection, double dfError, double dfGoal, int nPriority, const char* sTag);

    ///configure open-loop operation
    bool SetOpenLoop(WhichDOF nDirection, double dfVal, int nPriority, const char* sTag);

    bool AllStop(const char* sTag, int nPriority=0);

protected:
    CPID** m_PIDptrptr;

    /**
     * A structure defining parameters for a degree of freedom
     */
    class CDegreeOfFreedom
    {
    public:
        CDegreeOfFreedom(); ///< Initialize this structure to default values
        unsigned int m_nPriority;
        double m_dfVal;  ///< Error or Force, depending if loop is closed.
        double m_dfGoal; 
        bool m_bClosedLoop; ///< Flag for whether we are running closed-loop
        string m_sTag;
    };

    CDegreeOfFreedom m_DOF[6];

private:

    ///Do a sanity check on a direction
    bool DirectionSanityCheck(WhichDOF nDirection) const;    

    ///Do basic checks on the arguments to a SetOpenLoop or SetClosedLoop call
    bool LoopSanityCheck(WhichDOF nDirection, uint nPriority);

};

#endif // !defined(AFX_PATHACTION_H__5ED641C5_EBE1_4606_9758_087B8E19F48A__INCLUDED_)
