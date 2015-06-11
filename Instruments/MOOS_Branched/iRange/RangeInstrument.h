// RangeInstrument.h: interface for the CRangeInstrument class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RANGEINSTRUMENT_H__729E6929_5059_4FA5_AA51_0210B63B1FDB__INCLUDED_)
#define AFX_RANGEINSTRUMENT_H__729E6929_5059_4FA5_AA51_0210B63B1FDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<string>
#include "MOOSInstrumentFamily.h"

class CRangeInstrument : public CMOOSInstrumentFamily
{
public:
	CRangeInstrument();
	virtual ~CRangeInstrument();

protected:
	/**Overriding base class implementation to provide framework for 
	MOOS variable initialization*/
	bool OnStartUp();
	/**Implementations are responsible for providing a factory-like
	way of generating the appropriate Driver based on the input string.
	e.g. sType = "PSA916" - then a new CPSA916Driver is returned.
	*/
	CMOOSInstrumentDriver* CreateDriverFor(std::string sType);
};

#endif // !defined(AFX_RANGEINSTRUMENT_H__729E6929_5059_4FA5_AA51_0210B63B1FDB__INCLUDED_)
