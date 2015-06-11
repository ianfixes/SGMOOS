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

#include "SeaGrantGlobalHelper.h"
using namespace std;

bool ParseBoolean(bool& bVal,string sVal)
{

    MOOSToUpper(sVal);

    if(sVal=="TRUE" || sVal=="ON" || sVal=="YES" || sVal=="Y")
    {
        bVal=true;
        return true;
    }
    else if(sVal=="FALSE" || sVal=="OFF" || sVal=="NO" || sVal=="N")
    {
        bVal=false;
        return true;
    }

    return false;
}

bool Split(STRING_LIST& sList, string sStr, string sDelim)
{
    while(!sStr.empty())
    {
        sList.push_back(MOOSChomp(sStr, sDelim));
    }
    return !sList.empty();
}

bool Split(DOUBLE_LIST& dfList, string sStr, string sDelim)
{
    STRING_LIST sList;
    if(Split(sList, sStr, sDelim))
    {
        STRING_LIST::iterator i;
        for(i=sList.begin(); i!=sList.end(); i++)
        {
            if(MOOSIsNumeric(*i))
                dfList.push_back(atof(i->c_str()));
            else
                return false;
        }
    }
    else
        return false;

    return true;
}
