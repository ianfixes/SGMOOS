// Tritech200Driver.cpp: implementation of the CTritech200Driver class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#pragma warning(disable : 4503)
#endif

#include <MOOSLIB/MOOSLib.h>
#include "Tritech200Driver.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
string CTritech200Driver::sCMD_Z = "#Z";

CTritech200Driver::CTritech200Driver()
{

}

CTritech200Driver::~CTritech200Driver()
{

}

bool CTritech200Driver::GetData()
{
    string sReply;
	
    if(m_pPort->IsStreaming())
    {
        MOOSTrace("Tritech must be configured to stream in HW\n");
        return false;
    }
    
    //ask the PA200 for a reading
    if( m_pPort->IsVerbose() )
	MOOSTrace("Tritech200 Tx: %s\n", sCMD_Z.c_str());
    
    //#Z polling
    m_pPort->Write( (char*)sCMD_Z.c_str(), sCMD_Z.size() );
    
    char Reply[16];
    
    //respect the max 5Hz of Tritech
    double dfTimeOut = 0.1;
    int nRead = m_pPort->ReadNWithTimeOut((char*)Reply, sizeof(Reply), dfTimeOut);

    if( m_pPort->IsVerbose() )
	MOOSTrace("Tritech200 Rx: %s\n", &Reply[0]);

    if( nRead > 0 )
    {
	string sRange( &Reply[0] );
    	double dfRange;
        if( ParsePA200Range(sRange, dfRange) )
	{
	    return HandleRange( dfRange );
	}
    }

    return false;
}

bool CTritech200Driver::HandleRange(double& dfRange)
{
    return this->AddNotification("RANGE_ALTITUDE", dfRange);
}

bool CTritech200Driver::GetStatus()
{
    return true;
}

bool CTritech200Driver::Initialise()
{
    return CMOOSInstrumentDriver::Initialise();
}


bool CTritech200Driver::ParsePA200Range(std::string &sReply, double &dfRange)
{  
    string sRange = MOOSChomp(sReply, "\r\n");
	
    if( !sRange.empty() )
    {
        dfRange = atof(sRange.c_str());
	//mind the decimal point
	dfRange /= 100.0;

	if( m_bFilter )
	    m_pSensorFilter->Filter( dfRange );

	if( m_pPort->IsVerbose() )
	    MOOSTrace("Tritech200 Range: %3.3f\n", dfRange);
	return true;
    }
    else
    {
	if( m_pPort->IsVerbose() )
	    MOOSTrace("Tritech200 FAIL Range: {%3.3f}=={%s}\n", dfRange, sRange.c_str());
        return false;
    }
}


