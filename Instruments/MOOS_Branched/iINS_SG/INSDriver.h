// INSDriver.h: interface for the CINSDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INSDRIVER_H__DD8C1099_4849_4346_9B24_D334C4C82D5D__INCLUDED_)
#define AFX_INSDRIVER_H__DD8C1099_4849_4346_9B24_D334C4C82D5D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MOOSLIB/MOOSLib.h>
#include <string>
using namespace std;




class CMOOSSerialPort;
class CProcessConfigReader;


class CINSDriver  
{
public:
	virtual bool INSPortReadCallBack(char * pData, int nBufferLen,int nRead) =0;
	void SetOffset(double dfOffset);
	
	CINSDriver();
	virtual ~CINSDriver();
	virtual bool GetStatus() = 0;
	virtual bool SendMessage(CMOOSMsg Msg) = 0;
	virtual bool Initialise() = 0;
	virtual bool GetData() = 0;
	
	string GetINSType();
	void SetINSType(string sType);
	bool SetSerialPort(CMOOSSerialPort *pPort);
	void SetMissionFileReader(CProcessConfigReader* pMissionFileReader);
	bool GetNotifications(MOOSMSG_LIST &List);
protected:
	double m_dfVehicleYToINSX;
	double m_dfMagneticOffset;

	int m_nTempCnt;
	//list of variable values from sensor
	MOOSMSG_LIST m_Notifications;
	string m_sType;
	bool m_bVerbose;
	CProcessConfigReader* m_pMissionFileReader;
	CMOOSSerialPort* m_pPort;


};

#endif // !defined(AFX_INSDRIVER_H__DD8C1099_4849_4346_9B24_D334C4C82D5D__INCLUDED_)
