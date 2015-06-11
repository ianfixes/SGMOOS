// PSA916Driver.h: interface for the CPSA916Driver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSA916DRIVER_H__B5B827AB_EA09_4A78_A236_1559124544ED__INCLUDED_)
#define AFX_PSA916DRIVER_H__B5B827AB_EA09_4A78_A236_1559124544ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MOOSInstrumentDriver.h"

class CPSA916Driver : public CMOOSInstrumentDriver  
{
public:
	CPSA916Driver();
	virtual ~CPSA916Driver();
	
	/**Interface to receiving status from the sensor via SENSOR_STATUS.  */
	bool GetStatus();
	/**Implementation of the <code>CMOOSInstrument</code> interface*/
	bool Initialise();
	/**Implementation of the <code>CMOOSInstrument</code> interface*/
	bool GetData();

protected:
	bool ParsePSA916Range(std::string &sReply, double &dfRange);

};

#endif // !defined(AFX_PSA916DRIVER_H__B5B827AB_EA09_4A78_A236_1559124544ED__INCLUDED_)
