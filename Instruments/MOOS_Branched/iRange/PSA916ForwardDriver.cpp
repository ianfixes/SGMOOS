// PSA916ForwardDriver.cpp: implementation of the CPSA916ForwardDriver class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include <math.h>
#include "PSA916ForwardDriver.h"
using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPSA916ForwardDriver::CPSA916ForwardDriver()
{
	m_dfDepAngle = 15.0;
	m_bFilter = true;
	m_nFilterWindow = 5;

}

CPSA916ForwardDriver::~CPSA916ForwardDriver()
{

}

bool CPSA916ForwardDriver::GetData()
{
    string sReply;
    double dfWhen;
    if( m_pPort->GetLatest(sReply,dfWhen) )
    {
		double dfRange;
        if( ParsePSA916Range(sReply, dfRange) )
		{
			this->AddNotification("RANGE_FORWARD_RAW", dfRange);
			//handle the specific processing
			return HandleRange(dfRange);
		}
    }

	return false;
}

/**This method aims to back project the forward range from the range sensor
in order to back-project altitude and distance to targets in front of us*/
bool CPSA916ForwardDriver::HandleRange(double &dfRange)
{
	//if we are filtering the data, do so
	if( m_bFilter )
		m_pSensorFilter->Filter(dfRange);
	//notify MOOS
	this->AddNotification("RANGE_FORWARD", dfRange);

	//first we look for the variables of interest from MOOS
	CMOOSVariable* pRoll  = GetMOOSVar(m_sNavRoll);
	CMOOSVariable* pPitch = GetMOOSVar(m_sNavPitch);
	
	//we can't do the back projections
	if( !pRoll->IsFresh() && !pPitch->IsFresh())
		return false;

	double dfRollDeg  = MOOSRad2Deg(pRoll->GetDoubleVal()); 
        (void) dfRollDeg; //avoid warning for unused var
	double dfPitchDeg = MOOSRad2Deg(pPitch->GetDoubleVal());

	//let's work with the pitch for now
	double dfThetaFLADeg = m_dfDepAngle - dfPitchDeg;
	double dfThetaDeg	 = 90.0 - dfThetaFLADeg;
	
	double dfTheta = MOOSDeg2Rad(dfThetaDeg);
	double dfBPAlt = dfRange * cos(dfTheta);
	double dfHoriz = dfRange * sin(dfTheta);

	this->AddNotification("RANGE_BP_ALTITUDE",dfBPAlt);
	this->AddNotification("RANGE_HORIZ",dfHoriz);
		
	return true;
}

bool CPSA916ForwardDriver::Initialise()
{
	//call base class for filtering needs
	CMOOSInstrumentDriver::Initialise();

	//subscribe to variables that our InstrumentFamily will watch for us
	m_sNavRoll  = "NAV_ROLL";
	m_sNavPitch = "NAV_PITCH";
	this->AddRegistration(m_sNavRoll);
	this->AddRegistration(m_sNavPitch);

    //what is our depression angle 
    m_pMissionFileReader->GetConfigurationParam("DEPRESSIONANGLE", m_dfDepAngle);
	MOOSTrace("      Dep Angle = %3.2f\n", m_dfDepAngle);

	return true;
}

