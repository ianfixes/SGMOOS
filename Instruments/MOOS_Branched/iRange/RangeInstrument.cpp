// RangeInstrument.cpp: implementation of the CRangeInstrument class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include <MOOSLIB/MOOSLib.h>
#include "RangeInstrument.h"
#include "PSA916Driver.h"
#include "PSA916ForwardDriver.h"
#include "Tritech200Driver.h"
#include "Tritech200ForwardDriver.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRangeInstrument::CRangeInstrument()
{
    //some sensible defaults (missionfile can overwrite this)
    SetAppFreq(2);
    SetCommsFreq(8);
}

CRangeInstrument::~CRangeInstrument() 
{
    
}

CMOOSInstrumentDriver* CRangeInstrument::CreateDriverFor(std::string sType)
{

    if(MOOSStrCmp(sType,"PSA916"))
    {		
        return new CPSA916Driver;
	}
	else if (MOOSStrCmp(sType,"PSA916FORWARD"))
		return new CPSA916ForwardDriver;
	else if (MOOSStrCmp(sType,"TRITECH200"))
		return new CTritech200Driver;
	else if (MOOSStrCmp(sType,"TRITECH200FORWARD"))
		return new CTritech200ForwardDriver;
	else
		return NULL;
}


bool CRangeInstrument::OnStartUp()
{
	//note the addition of MOOSVariables before the base class call
	//this supports backwards compatibility with Simulator
	double dfPeriod = 0.1;
    AddMOOSVariable("Range", "SIM_ALTITUDE", "RANGE_ALTITUDE", dfPeriod);
    AddMOOSVariable("RANGE_RAW","","RANGE_RAW",0.1);
    
	//call the InstrumentFamily' base class for driver creation
	return CMOOSInstrumentFamily::OnStartUp();
}


