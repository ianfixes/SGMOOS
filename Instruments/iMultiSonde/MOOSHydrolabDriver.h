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
// C++ Interface: MOOSHydrolabDriver
//
// Description: Definition of CMOOSHydrolabDriver class.
//
//
// Author: Justin G Eskesen <jge@mit.edu>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MOOSHYDROLABDRIVER_H
#define MOOSHYDROLABDRIVER_H

#include "MOOSMultisondeDriver.h"
using namespace std;

/**
 * FIXME: needs class description
 */
class CMOOSHydrolabDriver : public CMOOSMultisondeDriver
{
 public:
  CMOOSHydrolabDriver();
  virtual ~CMOOSHydrolabDriver();

 protected:

  bool Initialise();
  bool GetData(MOOSVARMAP& Readings);
  bool PublishData();


  bool IsHydrolabInitialized(std::string sHydrolabString);
  std::string RemoveANSIControlChars(std::string sHydrolabString);  // Clean up raw Hydrolab output
  bool IsHydrolabStringValid(std::string sHydrolabString);    
  bool ParseString(std::string sString, MOOSVARMAP& Data);
  bool ConfigureHydroLab();  //configures sensor types
  bool CheckTTYMode();
  bool GetNamesString(std::string& sVarNameString);
  bool ProcessHeaders();
  bool ProcessUnits();
  bool ProcessData();
  bool GetFirstToken(std::string &message, std::string &token);
  double m_dfCollectionPeriod;   // number of seconds to wait in between collections
  double m_dfPrevCollectTime;    // time of previous data collection
  std::string m_sHydroLabMessage;
  int nHeaderCount;
  bool m_bHeadersInitialized;
  std::map<int,std::string> m_IndexToHeader;
  std::map<int,std::string> m_IndexToMOOSVar;
  std::map<int,std::string> m_IndexToUnits;
  bool m_bVerbose;
  bool StartReadings();


};

#endif
