//
// C++ Interface: ReefExporerThrusters
//
// Description: 
//
//
// Author: Justin G Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ReefExporerThrusters_H
#define ReefExporerThrusters_H

/**
Class to define the iterface of between MOOS instrument
	@author Justin G Eskesen <jge@mit.edu>
*/

#include <MOOSGenLib/MOOSGenLib.h>
#include <MOOSLIB/MOOSLib.h>
#include "ActuatorSet.h"
#include "MOOSJRKerrMotorDriver.h"

using namespace std;
typedef list< pair<string, CMOOSJRKerrMotorDriver> > JRKERR_LIST;

class CReefExporerThrusters : public CActuatorSet
{
public:
    CReefExporerThrusters();
    ~CReefExporerThrusters();

    void GetRegistrations(STRING_LIST& reg);
    void GetPublications(STRING_LIST& pub);
    virtual void SetSerial(CMOOSSerialPort* port);
    bool Initialise();
    bool Run(MOOSVARMAP& VarMap);

    bool GetStatus(string& sDescription);

    bool Enable(bool bSetStatus);


protected:

    bool OnTimeOut();
    JRKERR_LIST m_Motors;
    bool m_dfWinchDefault;  //in case value is not commanded.
};

#endif
