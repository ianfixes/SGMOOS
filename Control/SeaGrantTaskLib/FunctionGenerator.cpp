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
// CFunctionGenerator.cpp: implementation of the CFunctionGenerator class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include <math.h>
#include <iostream>
using namespace std;

#include "FunctionGenerator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CFunctionGenerator::CFunctionGenerator()
{

    m_bInitialised = false;
    m_sClassName = "Function Generator";
    m_pFunction=NULL;
}

CFunctionGenerator::~CFunctionGenerator()
{
    delete m_pFunction;
}

bool CFunctionGenerator::RegularMailDelivery(double dfTimeNow)
{
    return true;
}

bool CFunctionGenerator::Run(CPathAction &DesiredAction)
{

    if(!m_bInitialised)
    {
        Initialise();
    }
	
    DesiredAction.SetOpenLoop(
        m_nMyDOF, 
        m_pFunction->Run(MOOSTime()-m_dfStartTime),
        m_nPriority, 
        m_sName.c_str());

    return true;
}

bool CFunctionGenerator::SetParam(string sParam, string sVal)
{
    MOOSToUpper(sParam);
    MOOSToUpper(sVal);

    if(CSGMOOSBehaviour::SetParam(sParam,sVal))
        return true;

    //this is for us...
    if(MOOSStrCmp(sParam,"DOF"))
    {
        for(int i=0; i<6; i++)
            if(sVal==DOF_Names[i])
            {
                m_nMyDOF=(WhichDOF)i;
                break;
            }
    }
    else if(MOOSStrCmp(sParam,"FUNCTION"))
    {
        if(MOOSStrCmp(sVal,"Constant"))
            m_pFunction= new CFunction();
        else if(MOOSStrCmp(sVal,"Sinusoid"))
            m_pFunction= new CSinusoid();
        else if(MOOSStrCmp(sVal,"SquareWave"))
            m_pFunction= new CSquareWave();
        else
        {
            MOOSTrace("Unknown Function %s\n", sVal.c_str());
            return false;
        }
    }
    else if(m_pFunction != NULL)
        return m_pFunction->SetParam(sParam,sVal);
    else
        return false;

    return true;
}

bool CFunctionGenerator::Initialise()
{
    m_bInitialised = true;

    return true;
}

/***************************************************************
* Constant Function (Base Class)
***************************************************************/

CFunctionGenerator::CFunction::CFunction()
{
    m_dfAmplitude = 0.;
}


bool CFunctionGenerator::CFunction::SetParam(string sParam, string sVal)
{
    //this is for us...
    if(MOOSStrCmp(sParam,"Amplitude"))
    {
        m_dfAmplitude=atof(sVal.c_str());
    }
    else
        return false;

    return true;

}

double CFunctionGenerator::CFunction::Run(double dfTime)
{
    return m_dfAmplitude;
}

bool CFunctionGenerator::CFunction::IsValid()
{
    return m_dfAmplitude!=0.0;
}

/***************************************************************
* Sinusoid Function
***************************************************************/

CFunctionGenerator::CSinusoid::CSinusoid()
{
    //Set to unusable values so we can check if we've set them.
    m_dfPeriod = 0.;
    m_dfOffset = 0.;
}

bool CFunctionGenerator::CSinusoid::SetParam(string sParam, string sVal)
{
    //this is for us...
    if(MOOSStrCmp(sParam,"Period"))
        m_dfPeriod=atof(sVal.c_str());
    else if(MOOSStrCmp(sParam,"Offset"))
        m_dfOffset=atof(sVal.c_str());
    else
        return CFunctionGenerator::CFunction::SetParam(sParam, sVal);
    
    return true;
}

bool CFunctionGenerator::CSinusoid::IsValid()
{
    return  CFunctionGenerator::CFunction::IsValid() && m_dfPeriod!=0.0;
}

double CFunctionGenerator::CSinusoid::Run(double dfTime)
{
    return m_dfAmplitude*sin(dfTime * 2 * PI / m_dfPeriod) + m_dfOffset;
}

/***************************************************************
* Square Wave Function
***************************************************************/

double CFunctionGenerator::CSquareWave::Run(double dfTime)
{
    double dfVal = fmod(dfTime,m_dfPeriod)/m_dfPeriod < 0.5 ? 1.0 :-1.0;
    return m_dfAmplitude*dfVal + m_dfOffset;
}
