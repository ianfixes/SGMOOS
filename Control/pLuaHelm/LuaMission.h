/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Ian Katz & others at MIT Sea Grant.
    contact: ijk5@mit.edu

    This file is part of SGMOOS.

    SGMOOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SGMOOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SGMOOS.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/
//
// LuaMission.h - interface for the CLuaMission class
//
/////////////////////////////////////


#if !defined(AFX_LUAMISSION_H_INCLUDED_)
#define AFX_LUAMISSION_H_INCLUDED_

#include <MOOSLIB/MOOSLib.h>
#include "MOOSLuaEnvironment.h"
#include <lua5.1/lua.hpp>
#include <string>

using namespace std;

typedef set<string> STRING_SET;


class CLuaMission : public CMOOSLuaEnvironment
{
public:
    CLuaMission(string sIdentifier,
                LUA_API_MAP api, 
                void (*OnError)(string, string),
                string sLuaLibDir);

    virtual ~CLuaMission();
    

    //mission name
    string Name();

    //whether we're done
    bool IsDone();

    //check loaded lua file for existence of proper functions
    bool IsValid();

    //initialize the mission -- get it to the point where it can tell us registrations
    bool Init();

    //a list of vars we need updates on
    bool Registrations(STRING_SET& sl);

    //pyGraph representation
    string ToGraph();

    //internally prepare the mission to run
    bool Prepare();

    //execute one tick of the mission.  moos variable writes are done thru the API
    bool Tick(MOOSMSG_LIST m);

    //tell the mission to stop everything
    void Cleanup();


protected:

    //wrapper for LuaFuncExists that prints an error message
    bool CheckFunction(bool initial, string func);


};

#endif // !defined(AFX_LUAMISSION_H_INCLUDED_)

