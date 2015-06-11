// Tritech200Driver.h: interface for the CTritech200Driver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRITECH200DRIVER_H__826F90ED_413E_42CE_91D5_41358732D828__INCLUDED_)
#define AFX_TRITECH200DRIVER_H__826F90ED_413E_42CE_91D5_41358732D828__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MOOSInstrumentDriver.h"

class CTritech200Driver : public CMOOSInstrumentDriver  
{
public:
	static std::string sCMD_Z;
	CTritech200Driver();
	virtual ~CTritech200Driver();
	/**Interface to receiving status from the sensor via SENSOR_STATUS.  */
	bool GetStatus();
	/**Implementation of the <code>CMOOSInstrument</code> interface*/
	bool Initialise();
	/**Implementation of the <code>CMOOSInstrument</code> interface*/
	bool GetData();

protected:
	virtual bool HandleRange(double& dfRange);
	bool ParsePA200Range(std::string &sReply, double &dfRange);
};

#endif // !defined(AFX_TRITECH200DRIVER_H__826F90ED_413E_42CE_91D5_41358732D828__INCLUDED_)
