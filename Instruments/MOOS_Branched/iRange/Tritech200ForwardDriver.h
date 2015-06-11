// Tritech200ForwardDriver.h: interface for the CTritech200ForwardDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRITECH200FORWARDDRIVER_H__513FEA2D_7056_4B47_9012_EEEECA6A83C2__INCLUDED_)
#define AFX_TRITECH200FORWARDDRIVER_H__513FEA2D_7056_4B47_9012_EEEECA6A83C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Tritech200Driver.h"
#define DEFAULT_CHANNEL_NAME "RANGE_FORWARD"
#define RANGE_PREFIX "RANGE_"

class CTritech200ForwardDriver : public CTritech200Driver  
{
public:
	std::string GetChannelName();
	void SetChannelName(std::string sName);
	CTritech200ForwardDriver();
	virtual ~CTritech200ForwardDriver();

	/**Implementation of the <code>CMOOSInstrument</code> interface*/
	bool Initialise();
protected:
	bool HandleRange(double& dfRange);
	std::string m_sChannelName;
};

#endif // !defined(AFX_TRITECH200FORWARDDRIVER_H__513FEA2D_7056_4B47_9012_EEEECA6A83C2__INCLUDED_)
