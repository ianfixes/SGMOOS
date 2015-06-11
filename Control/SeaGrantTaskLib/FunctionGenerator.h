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
// CFunctionGeneratorTask.h: interface for the CFunctionGeneratorTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(FUNCTION_GENERATOR_INCLUDED)
#define FUNCTION_GENERATOR_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MOOSBehaviour.h"

/**
 * Generates control commands based on simple functions (sine, square, etc)
 */
class CFunctionGenerator : public CSGMOOSBehaviour  
{
public:
    CFunctionGenerator();
    virtual ~CFunctionGenerator();

    virtual bool SetParam(string sParam, string sVal);
    virtual bool Run(CPathAction &DesiredAction);
    virtual bool RegularMailDelivery(double);
protected:

    /**
     * Function Base Class - Constant Function
     */
    class CFunction
    {
    public:
        CFunction();
        virtual bool SetParam(string sParam, string sVal);
        virtual double Run(double dfTime);
        virtual bool IsValid();
    protected:
        double m_dfAmplitude;  
    };
    
    
    /**
     * For producing sine functions
     */
    class CSinusoid : public CFunction
    {
      public:
        CSinusoid();
        virtual bool SetParam(string sParam, string sVal);
        virtual double Run(double dfTime);
        virtual bool IsValid();

      protected:
        double m_dfPeriod;
        double m_dfOffset;
    };
    
    /**
     * For producing square wave functions
     */
    class CSquareWave : public CSinusoid
    {
      public:
        virtual double Run(double dfTime);

    };




    virtual bool Initialise();
    WhichDOF m_nMyDOF;
    CFunction* m_pFunction;




};

#endif // !defined(FUNCTION_GENERATOR_INCLUDED)
