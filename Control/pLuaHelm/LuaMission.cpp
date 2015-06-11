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

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SGMOOS.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/

#include "LuaMission.h"

CLuaMission::CLuaMission(string sIdentifier,
                         LUA_API_MAP api, 
                         void (*OnError)(string, string),
                         string sLuaLibDir) 
  : CMOOSLuaEnvironment(sIdentifier, api, OnError, sLuaLibDir) {}

CLuaMission::~CLuaMission()
{

}


// return true if the mission passes some basic sanity checks
bool CLuaMission::IsValid()
{
    //validate: check existence of necessary functions 
    bool ret = true;
    ret = CheckFunction(ret, "Init");            //load any initial structs, know what moosvars are needed
    ret = CheckFunction(ret, "Name");            //what's this mission called?
    ret = CheckFunction(ret, "Registrations");   //list what moosvars are needed
    ret = CheckFunction(ret, "ToGraph");         //spit out a string representation for a graph
    ret = CheckFunction(ret, "Prepare");         //reset any pointers for the begnning of a mission
    ret = CheckFunction(ret, "Tick");            //take some moos vars, return some new ones
    ret = CheckFunction(ret, "IsDone");          //done?
    ret = CheckFunction(ret, "Cleanup");         //close any stray behaviors

    return ret;
}

//make sure a function exists and produce an error message if not
bool CLuaMission::CheckFunction(bool initial, string func)
{
    if (LuaFuncExists(func))        
    {
        return initial;
    }
    else
    {
        MOOSTrace("Lua Mission '%s': Missing function %s\n", m_sLuaFilename.c_str(), func.c_str());
        return false;
    }

}


/////////////////////////////////// C++ INTERFACE TO LUA MISSION FUNCTIONS


bool CLuaMission::Init()
{
    if (CallLuaVoid("Init", 1)) return false;

    bool ret = lua_toboolean(m_luaState, -1);              //retval on top of stack
    lua_pop(m_luaState, 1);                                //pop retval
    
    CallLuaCleanup();
    return ret;
}

string CLuaMission::Name()
{
    if (CallLuaVoid("Name", 1)) return "[NameError]";         //func, 1 return

    string ret = lua_tostring(m_luaState, -1);             //retval on top of stack
    lua_pop(m_luaState, 1);                                //pop retval

    CallLuaCleanup();
    return ret;
}

bool CLuaMission::Registrations(STRING_SET &sl)
{
    if (CallLuaVoid("Registrations", 1)) return false;

    if (!lua_istable(m_luaState, -1)) //if wrong retval GTFO
    {
        MOOSTrace("CLuaMission: %s:Registrations() Result not table!\n", 
                  m_sIdentifier.c_str());
        lua_pop(m_luaState, 1);
        CallLuaCleanup();
        return false;
    }

    
    string s;
    int t = lua_gettop(m_luaState); //table is on top
    lua_pushnil(m_luaState);        // first key 
    while (0 != lua_next(m_luaState, t)) 
    {
        s = lua_tostring(m_luaState, -1);
        MOOSTrace("Got registration %s\n", s.c_str());
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        sl.insert(s);
        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(m_luaState, 1);
    }
    
    //remove table itself
    lua_pop(m_luaState, 1);
    CallLuaCleanup();
    return true;
}

string CLuaMission::ToGraph()
{
    if (CallLuaVoid("ToGraph", 1)) return "";

    string ret = lua_tostring(m_luaState, -1);             //retval on top of stack
    lua_pop(m_luaState, 1);                                //pop retval

    CallLuaCleanup();
    return ret;
}

bool CLuaMission::Prepare()
{
    if (CallLuaVoid("Prepare", 1)) return false;

    bool ret = lua_toboolean(m_luaState, 1);
    lua_pop(m_luaState, 1);
    
    CallLuaCleanup();
    return ret;
}

bool CLuaMission::Tick(MOOSMSG_LIST m)
{
    //this is probably the most serious lua function call in here... lot of arg setup

    int myErrHandler = SetLuaErrHandler();

    lua_getglobal(m_luaState, "Tick"); //func to call

    //build a nested table from MOOS messages, which will be the only arg to this func call
    lua_newtable(m_luaState);
    int outertop = lua_gettop(m_luaState);

    int key = 0; //this will be incremented immediately because lua is 1-indexed
    for (MOOSMSG_LIST::iterator it = m.begin(); it != m.end(); ++it) 
    {
        key++;
        lua_pushnumber(m_luaState, key); //the key for the nested table 

        //make the "value", the nested table
        lua_newtable(m_luaState);
        int innertop = lua_gettop(m_luaState);

        //add vars
        BuildLuaTable("time",      it->GetTime(), innertop);
        BuildLuaTable("name",      it->GetName(), innertop);
        BuildLuaTable("source",    it->GetSource(), innertop);
        BuildLuaTable("community", it->GetCommunity(), innertop); 

        //value
        if (it->IsDouble())
        {
            BuildLuaTable("isnumber", true, innertop);
            BuildLuaTable("value",    it->GetDouble(), innertop);
        }
        else
        {
            BuildLuaTable("isnumber", false, innertop);
            BuildLuaTable("value",    it->GetString(), innertop);
        }

        //finish the entry in this table
        lua_settable(m_luaState, outertop);
    }
    //if it matters, "key" now holds the number of MOOS messages added
    //MOOSTrace("C++ says that it pushed %d MOOS messages to %s\n", key, m_sIdentifier.c_str());

    CallLua(1, 1, myErrHandler);         //func, 1 args, 1 return

    bool ret = lua_toboolean(m_luaState, -1);              //retval on top of stack
    lua_pop(m_luaState, 1);                                //pop retval

    CallLuaCleanup();
    return ret;
}

bool CLuaMission::IsDone()
{
    string name = Name();

    CallLuaVoid("IsDone", 1);         //func, 1 return

    bool ret = lua_toboolean(m_luaState, -1);              //retval on top of stack

    //MOOSTrace("CLuaMission::IsDone for %s returns %s\n", 
    //          name.c_str(), (ret ? "True" : "False"));


    lua_pop(m_luaState, 1);                                //pop retval
    CallLuaCleanup();
    return ret;
}

void CLuaMission::Cleanup()
{
    CallLuaVoid("Cleanup", 0);         //func, 0 return

    CallLuaCleanup();
}
