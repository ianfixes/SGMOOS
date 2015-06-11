// PSA916Driver.cpp: implementation of the CPSA916Driver class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include <MOOSLIB/MOOSLib.h>
#include "PSA916Driver.h"

using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPSA916Driver::CPSA916Driver()
{

}

CPSA916Driver::~CPSA916Driver()
{

}

bool CPSA916Driver::GetData()
{
    string sReply;
    double dfWhen;
    if( m_pPort->GetLatest(sReply,dfWhen) )
    {
		double dfRange;
        if( ParsePSA916Range(sReply, dfRange) )
		{
			return this->AddNotification("RANGE_ALTITUDE", dfRange);
		}
    }

	return false;
}

bool CPSA916Driver::GetStatus()
{
	return true;
}

bool CPSA916Driver::Initialise()
{
	CMOOSInstrumentDriver::Initialise();

	string  sReset;
    
    if(!sReset.empty())
    {
        if(!m_pPort->Write((char*)sReset.c_str(),sReset.size()))
        {
            MOOSTrace("failed sensor init\n");
            return false;
        }
    }
    
    MOOSPause(200);
    
    return true;
}

// parse parascientific depth sensor
bool CPSA916Driver::ParsePSA916Range(std::string &sReply, double &dfRange)
{  
    if(sReply.find("E")!=string::npos)
    {
        if(sReply.find("E1")!=string::npos)
        {
			if( this->IsVerbose() )
			{
				MOOSTrace("Range Instrument saw Error E1\n");
			}
        }
        else
        {
			if( this->IsVerbose() )
			{
				MOOSTrace("PSA916 out of range\n");
			}
        }
        
        return false;
    }
 
	MOOSChomp(sReply, "R");
	
	if( !sReply.empty() )
    {
        dfRange = atof(sReply.c_str());
		return true;
    }
    else
    {
        return false;
    }
}
