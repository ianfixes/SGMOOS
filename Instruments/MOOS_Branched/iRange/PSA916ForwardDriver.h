// PSA916ForwardDriver.h: interface for the CPSA916ForwardDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSA916FORWARDDRIVER_H__DAD01BAC_BB8B_4F16_A1BF_2C51C12FD261__INCLUDED_)
#define AFX_PSA916FORWARDDRIVER_H__DAD01BAC_BB8B_4F16_A1BF_2C51C12FD261__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PSA916Driver.h"

class CPSA916ForwardDriver : public CPSA916Driver  
{
public:
	CPSA916ForwardDriver();
	virtual ~CPSA916ForwardDriver();
	
	bool GetData();
	bool Initialise();
protected:
	CSensorDataFilter* CreateFilterFor(std::string sType);
private:
	std::string m_sNavPitch;
	std::string m_sNavRoll;
	double m_dfDepAngle;
	bool HandleRange(double &dfRange);
};

#endif // !defined(AFX_PSA916FORWARDDRIVER_H__DAD01BAC_BB8B_4F16_A1BF_2C51C12FD261__INCLUDED_)
