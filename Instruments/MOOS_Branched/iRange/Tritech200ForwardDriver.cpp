// Tritech200ForwardDriver.cpp: implementation of the CTritech200ForwardDriver class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include <math.h>
#include "Tritech200ForwardDriver.h"
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTritech200ForwardDriver::CTritech200ForwardDriver()
{
    m_sChannelName = DEFAULT_CHANNEL_NAME;
}

CTritech200ForwardDriver::~CTritech200ForwardDriver()
{

}

bool CTritech200ForwardDriver::HandleRange(double& dfRange)
{
    return this->AddNotification(GetChannelName().c_str(), dfRange);
}

bool CTritech200ForwardDriver::Initialise()
{
    CTritech200Driver::Initialise();
    
    //now read what channel we are PORT or STARBOARD
    string sName;
    if( m_pMissionFileReader->GetConfigurationParam("CHANNEL", sName) )
    {
	MOOSTrace("Channel= %s\n", sName.c_str());
	SetChannelName( RANGE_PREFIX + sName );
    }
    else
	MOOSTrace("CTritech200ForwardDriver::Initialise(): WARNING - No Channel Name\n");
    
    return true;
}

void CTritech200ForwardDriver::SetChannelName(string sName)
{
    m_sChannelName = sName;
}

string CTritech200ForwardDriver::GetChannelName()
{
    return m_sChannelName;
}
