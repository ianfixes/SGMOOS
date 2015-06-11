// XbowDriver.h: interface for the CXbowDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XBOWDRIVER_H__ED9BC89A_B64C_45F6_99C4_0DD63F840178__INCLUDED_)
#define AFX_XBOWDRIVER_H__ED9BC89A_B64C_45F6_99C4_0DD63F840178__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "INSDriver.h"

class CXbowDriver : public CINSDriver  
{
public:
	bool INSPortReadCallBack(char * pData, int nBufferLen,int nRead);
	CXbowDriver();
	virtual ~CXbowDriver();
	bool GetStatus();
	bool SendMessage(CMOOSMsg Msg);
	bool Initialise();
	bool GetData();
 protected:
	double m_dfTemp;
};

#endif // !defined(AFX_XBOWDRIVER_H__ED9BC89A_B64C_45F6_99C4_0DD63F840178__INCLUDED_)
