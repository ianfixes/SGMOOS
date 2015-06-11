// INSDriver.cpp: implementation of the CINSDriver class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include "INSDriver.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CINSDriver::CINSDriver()
{
	m_bVerbose =false;
}

CINSDriver::~CINSDriver()
{

}



bool CINSDriver::SetSerialPort(CMOOSSerialPort *pPort)
{

	m_pPort = pPort;
    //if port is verbose then so are we!
    m_bVerbose = m_pPort->IsVerbose();

    return m_pPort != NULL;
    
}

void CINSDriver::SetMissionFileReader(CProcessConfigReader* pMissionFileReader)
{
    m_pMissionFileReader = pMissionFileReader;
}

bool CINSDriver::GetNotifications(MOOSMSG_LIST &List)
{
	List.splice(List.begin(), m_Notifications);

	return true;
}

void CINSDriver::SetOffset(double dfOffset)
{
	m_dfMagneticOffset = dfOffset;
}
