/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Justin Eskesen & others at MIT Sea Grant.
    contact: jge@mit.edu

    This file is part of SGMOOS.

    SGMOOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SGMOOS.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/
//
// C++ Implementation: MOOSHydrolabDriver
//
// Description: Implementation of CMOOSHydrolabDriver class.
//
//
// Author: Justin G Eskesen <jge@mit.edu>,(C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include <iostream>
#include <string.h>
//#include "fnmatch.h"

#include <MOOSLIB/MOOSLib.h>
#include <MOOSGenLib/MOOSGenLib.h>
#include "MOOSHydrolabDriver.h"

using namespace std;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSHydrolabDriver::CMOOSHydrolabDriver()
{
	m_dfCollectionPeriod = 10;           // wait 10 seconds by default
	m_dfPrevCollectTime = MOOSTime();
	m_bVerbose=1; //set to false to stop all those darn messages
}

CMOOSHydrolabDriver::~CMOOSHydrolabDriver()
{

}


 
/////////////////////////////////////////////
///this is where it all happens..

bool CMOOSHydrolabDriver::Initialise()
{

	double dfCurrTime;								// Current time
	double dfStartTime;								// Time we started initialization
	double dfWakeupTime;							// Last time we tried waking up Hydrolab  
	string sWhat;									// Contains output from Hydrolab

	string sCursorPositionQuery = "\033[6n";
	string sCursorPositionResponse = "\033[1;1R\r\n";

	dfStartTime = dfWakeupTime = dfCurrTime = MOOSTime();
  
	// set terminating character to n because we're looking for sCursorPositionQuery
	m_pPort->SetTermCharacter('n'); 

	// Give three minutes to initialise Hydrolab
	while (dfStartTime + 180 > dfCurrTime)
    {     

		// Read from Hydrolab
		if(m_pPort->GetLatest(sWhat,dfCurrTime))
        {
			// Send cursor position response immediately no matter what
			m_pPort->Write(sCursorPositionResponse.c_str(), (size_t)sCursorPositionResponse.size());

			// Looking for cursor position queries
			if (sWhat.find(sCursorPositionQuery) != string::npos)
            {
              	if(m_bVerbose)
				{
					MOOSTrace("CMOOSHydrolabDriver::InitialiseSensor - Hydrolab reply matched cursor position query\n");
				}
            }
			else
            {
				if(m_bVerbose)
				{
					MOOSTrace("CMOOSHydrolabDriver::InitialiseSensor - Hydrolab reply didn't match cursor position query\n");
				}
				// We might have Hydrolab data now so we get ready for lines of data
				m_pPort->SetTermCharacter(0xa);

				// Check if string indicates Hydrolab is initialized
				if (IsHydrolabInitialized(sWhat)) 
                {
					MOOSTrace("CMOOSHydrolabDriver::InitialiseSensor - Hydrolab is initialized\n");
					return true;
                }
              
			}

   
        }
		else
        {

			dfCurrTime = MOOSTime();

			// Hydrolab is supposed to send queries every 45 seconds
			// If we haven't heard anything for 45 seconds, something's wrong!
			// Out of desperation, send a string of characters in hopes Hydrolab wakes up
			if (dfWakeupTime + 45 < dfCurrTime) 
            {
				// Wake up MiniSonde by sending string of characters
				// Use sCursorPositionResponse on off chance it will initialize Hydrolab
				m_pPort->Write(sCursorPositionResponse.c_str(), (size_t)sCursorPositionResponse.size());
				dfWakeupTime = dfCurrTime;
				
				if(m_bVerbose)
				{	
					MOOSTrace("CMOOSHydrolabDriver::InitialiseSensor - Sent wakeup string\n");
				}
            }

        }

    }
 
	return false;
}


bool CMOOSHydrolabDriver::GetData(MOOSVARMAP& Readings)
{
	//gets data and places it in a string variable sHydroLabMessage for access.
              
	double dfWhen;
	int nMaxTries = 10;
	int nTries = 0;
	std::string sWhat;


	// Read Hydrolab data until we receive a valid line of data
	do 
    {
		// Give up?
		if (++nTries > nMaxTries) 
        {
			if(m_bVerbose)
			{
				MOOSTrace("CMOOSHydrolabDriver::GetData - Exceeded max number of attempts (%d) to read Hydrolab\n",
				     nMaxTries);
			}
			return false;
        }
      
		// Read from Hydrolab
		if(!m_pPort->GetLatest(m_sHydroLabMessage,dfWhen))
        {
          return false;
        }
		else
		{
			//MOOSTrace("CMOOSHydrolabDriver::GetData - Received Msg="+m_sHydroLabMessage+"\n");
			return true;
		}
		// Clean up Hydrolab output
		//sWhat = RemoveANSIControlChars(sWhat);
        
    } while (true);//!IsHydrolabStringValid(sWhat));   // Have we received VALID Hydrolab data?


	// Validated Hydrolab data, now parse data from it
	ParseString(sWhat, Readings);

	return true;
}


bool CMOOSHydrolabDriver::IsHydrolabInitialized(std::string sHydrolabString)
{
    return true;
}


// Should make this a pass by reference...
string CMOOSHydrolabDriver::RemoveANSIControlChars(std::string sHydrolabString)
{

	int nEscapeCharIndex = 0;   // index of last found escape character
	int nHIndex = 0;            // index of last found H character


	string sEscapeChar = "\033";
  
	// Loop as long as:
	// 1) There's a left bracket in the string that we haven't found
	// 2) The last H character we found is ahead of the last left bracket character we found
	while ( ( (nEscapeCharIndex = sHydrolabString.find(sEscapeChar)) >= 0 ) &&
          ( (nHIndex = sHydrolabString.find("H", nEscapeCharIndex)) > nEscapeCharIndex) )
    {
          // Replace block of formatting characters with one space
          sHydrolabString.replace(nEscapeCharIndex, nHIndex - nEscapeCharIndex + 1, " ");
    }
  
	return sHydrolabString;

}


bool CMOOSHydrolabDriver::IsHydrolabStringValid(std::string sHydrolabString)
{
	return true;
}


bool CMOOSHydrolabDriver::ParseString(std::string sString, MOOSVARMAP& Data)
{

	string sToken;               // Next token from input string

	// Get tokens and store them in variables
	//
	// Tokens will correspond to the following pieces of data:
	//
	// Token  |   Data
	// ----------------
	//   1    |   Time
	//   2    |   "Ext"
	//   3    |   "Batt:" + Voltage
	//   4    |   Time
	//   5    |   Temp 
	//   6    |   DO%
	//   7    |   SpCond (S/cm)
	//   8    |   Dep25 (meters)
	//   9    |   pH (units)
	//   10   |   ORP (mV)

	return true;
}
bool CMOOSHydrolabDriver::ConfigureHydroLab()
{
	MOOSVARMAP Readings;
	std::string sVarsNameString;

	while(true)
	{
		bool bData = GetData(Readings);
		if (!bData)
		{
			if(m_bVerbose)
			{
				MOOSTrace("CMOOSHydrolabDriver::ConfigureHydroLab - Get Data Failed - Retrying - Is HydroLab connected and in TTY mode?\nIn Console Setup->Display->TTY\n");
			}
			m_pPort->Write("h",1);
			MOOSPause(10000);
		}
		else
		{
			if (m_sHydroLabMessage.size() >0)
			{
				break;
			}
			else
			{
				if(m_bVerbose)
				{
					MOOSTrace("CMOOSHydrolabDriver::ConfigureHydroLab - Message Is < 0\n");
				}
			}
		}
	//	return false;
	}
	//MOOSTrace("CMOOSHydrolabDriver::ConfigureHydroLab - Data Retrieved\n");
	if(!CheckTTYMode())
	{
		if(m_bVerbose)
		{
			MOOSTrace("CMOOSHydrolabDriver:: Hydrolab is not in TTY Mode.  Please Correct.  In Console Setup->Display->TTY\n");
		}
		//Place it in TTY made by Setup>Display>TTY
		return false;
	}
	if(m_bVerbose)
	{
		MOOSTrace("CMOOSHydrolabDriver::ConfigureHydroLab - TTY Mode Confirmed\n");
	}
	GetNamesString(sVarsNameString);
	return true;

}

bool CMOOSHydrolabDriver::CheckTTYMode()
{
	string sEscapeChar = "\033";
	int nEscape;
	nEscape = m_sHydroLabMessage.find(sEscapeChar);
	//MOOSTrace("CheckTTYMode - nEscape=%i\n",nEscape);
	if (nEscape >= 0)
	{
		return false;
	}
	else
	{
		return true;
	}

}

bool CMOOSHydrolabDriver::GetNamesString(std::string& sVarNameString)
{
	m_bHeadersInitialized= false;
	while(!m_bHeadersInitialized)
	{	
		int nAttempts = 0;
		//pause character
		if(m_bVerbose)
		{
			MOOSTrace("CMOOSHydrolabDriver::GetHeaders() - Sending Header Sequence\n");
		}
		//string space = " ";
		m_pPort->Write(" ", 1);
		//string h = "h";
		MOOSPause(1000);
		//ask for headers
		m_pPort->Write("h",1);
		bool bHeadersFound = false;	
		bool bUnitsFound = false;
		string sTemp;
		string sToken;
		// relies on the fact that TIME and the units HHMMSS are first tokens
		while(!bHeadersFound || !bUnitsFound)
		{

			if (nAttempts > 5)
			{
				if(m_bVerbose)
				{
					MOOSTrace("CMOOSHydrolabDriver::GetHeaders() - Retrying To GetHeaders\n");
				}
				//retries getting headers.
				break;	
			}
			//MOOSTrace("CMOOSHydrolabDriver::GetHeaders()  Awaiting Headers and Units\n");
//			GetData(Readings);
			sTemp = m_sHydroLabMessage;
			if(sTemp.size() <= 0)
			{
				//MOOSTrace("CMOOSHydrolabDriver::GetHeaders() - Received null Msg\n");
				//MOOSPause();
				continue;
			}
			GetFirstToken(sTemp, sToken);
			//MOOSTrace("CMOOSHydrolabDriver::GetHeaders() - First Token =" + sToken + "\n Msg = " + sTemp + "\n");
			//int iTemp = sTemp.c_str()[0];
			if (sToken == "Time")
			{
				//MOOSTrace("CMOOSHydrolabDriver::GetHeaders() - Found Token Time\n");
				bHeadersFound = ProcessHeaders();
			}
			if (sToken == "HHMMSS")
			{
				//MOOSTrace("CMOOSHydrolabDriver::GetHeaders() - Found Token HHMMSS\n");
				bUnitsFound = ProcessUnits();
			}
			if (bHeadersFound && bUnitsFound)
			{
				//MOOSTrace("Headers and Units Found \n");
				m_bHeadersInitialized= true;
			}
			//messages sent every sec
			//MOOSPause(1000);
			nAttempts++;
		}
	}
	if(m_bVerbose)
	{
		MOOSTrace("CMOOSHydrolabDriver::GetHeaders() - Success\n");
	}
	return true;

}

bool CMOOSHydrolabDriver::ProcessHeaders()
{
	string sTemp;
	string sToken;
	sTemp = m_sHydroLabMessage;
	if(m_bVerbose)
	{
		MOOSTrace("CMOOSHydrolabDriver::ProcessHeaders() " + sTemp + "\n");
	}
	//index of header
	nHeaderCount = 0;
	while(sTemp.size()>0)
	{		
		GetFirstToken(sTemp, sToken);
		MOOSToUpper(sToken);
		m_IndexToHeader[nHeaderCount] = sToken;
		m_IndexToMOOSVar[nHeaderCount] = "HYDROLAB_" + sToken;		
		nHeaderCount++;
	}
	if(m_bVerbose)
	{
		MOOSTrace("CMOOSHydrolabDriver::ProcessHeaders() - Processed %i\n", nHeaderCount);
	}
	return true;
}
bool CMOOSHydrolabDriver::ProcessUnits()
{
	string sTemp;
	string sToken;
	sTemp = m_sHydroLabMessage;
	if(m_bVerbose)
	{
		MOOSTrace("CMOOSHydrolabDriver::ProcessUnits() " + sTemp + "\n");
	}
	//index of header
	nHeaderCount = 0;
	while(sTemp.size()>0)
	{		
		GetFirstToken(sTemp, sToken);
		m_IndexToUnits[nHeaderCount] = sToken;
		nHeaderCount++;
	}
	if(m_bVerbose)
	{
		MOOSTrace("CMOOSHydrolabDriver::ProcessUnits() - Processed %i\n", nHeaderCount);
	}

	return true;
}
bool CMOOSHydrolabDriver::GetFirstToken(std::string &message, std::string &token)
{
		MOOSTrimWhiteSpace(message);
		token = MOOSChomp(message, " ");
	return true;
}

bool CMOOSHydrolabDriver::ProcessData()
{
	string sTemp;
	string sToken;
	sTemp = m_sHydroLabMessage;
	int i =0;
	//char ** pEnd;
	while(sTemp.size()>0)
	{
		
		GetFirstToken(sTemp, sToken);
//		SetMOOSVar(m_IndexToHeader[i],atof(sToken.c_str()),MOOSTime());
		if(m_bVerbose)
		{
			MOOSTrace("CMOOSHydrolabDriver::ProcessData() - %s = %s\n", m_IndexToHeader[i].c_str(),sToken.c_str());
		}
		i++;
	}
	return true;
}


bool CMOOSHydrolabDriver::StartReadings()
{
//Doesn't matter if you send a 1 command while readings are already being taken, so we can just send it.
	m_pPort->Write("1", 1); //Run Menu -> Start Data Collection
    return true;
}
