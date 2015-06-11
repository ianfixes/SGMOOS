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
#if !defined(KEYAction_H)
#define KEYAction_H

#include<MOOSLIB/MOOSLib.h>

using namespace std;

/**
 * FIXME: needs class description
 */
class CKeyAction
{
  public:
    CKeyAction();
    virtual ~CKeyAction(){}
    virtual bool OnKeystroke(MOOSVARMAP& Vars);
    string GetDescription() {return m_sDescription;}
    bool ReadConfig(std::string& sConfig);

    // Has no registrations or publications by default.
    virtual bool GetRegistrations(STRING_LIST& reg) {return true;}
    virtual bool GetPublications(STRING_LIST& pub) {return true;}
    
    char GetKeyBinding() {return m_cKey;}

  protected:
    virtual bool ParseConfigString(std::string& sConfig);
    virtual void ReadOptions(std::string& sConfig);
    bool Confirm();
    virtual void PrintConfirmMessage();
    bool m_bConfirmBeforeActing;
    char m_cKey;
    std::string m_sDescription;

};

typedef map<char, CKeyAction*> KEY_MAP;
typedef KEY_MAP::iterator KEY_MAP_ITER;

/**
 * FIXME: needs class description
 */
class CCustomStringKeyAction : public CKeyAction
{
  public:
    CCustomStringKeyAction();
    virtual bool OnKeystroke(MOOSVARMAP& Vars);
    virtual bool GetPublications(STRING_LIST& pub);

  protected:
    bool ReadMOOSVar(std::string& sConfig);
    bool PublishValue(MOOSVARMAP& Vars);
    virtual void PrintConfirmMessage();
    virtual bool ParseConfigString(std::string& sConfig);

    string m_sMOOSVar;
    string m_sValue;
    string m_sPrompt;
};


/**
 * FIXME: needs class description
 */
class CStringKeyAction : public CCustomStringKeyAction
{
  public:
    CStringKeyAction(){};
    virtual bool OnKeystroke(MOOSVARMAP& Vars);
  protected:
    virtual bool ParseConfigString(std::string& sConfig);
};

class CCustomNumericKeyAction : public CCustomStringKeyAction
{
  public:
    CCustomNumericKeyAction(){};
    virtual bool OnKeystroke(MOOSVARMAP& Vars);
    bool PublishValue(MOOSVARMAP& Vars);

  protected:
    virtual void PrintConfirmMessage();
    double m_dfValue;
};

/**
 * FIXME: needs class description
 */
class CNumericKeyAction : public CCustomNumericKeyAction
{
  public:
    CNumericKeyAction(){};
    virtual bool OnKeystroke(MOOSVARMAP& Vars);
  protected:
    virtual bool ParseConfigString(std::string& sConfig);

};

/**
 * FIXME: needs class description
 */
class CNumericStepKeyAction : public CCustomNumericKeyAction
{
  public:
    CNumericStepKeyAction();
    virtual bool OnKeystroke(MOOSVARMAP& Vars);

  protected:    

    virtual bool ParseConfigString(std::string& sConfig);
    double m_dfStep;
    bool m_bDoWrap;
    double m_dfWrapAt;
    
};

/**
 * FIXME: needs class description
 */
class CSummaryKeyAction : public CKeyAction
{
  public:
    CSummaryKeyAction(){};
    virtual bool OnKeystroke(MOOSVARMAP& Vars);
    virtual bool GetRegistrations(STRING_LIST& reg);
  protected:
    virtual bool ParseConfigString(std::string& sConfig);
    STRING_LIST m_Subscriptions;
};

/**
 * FIXME: needs class description
 */
class CQuitKeyAction : public CKeyAction
{
  public:
    CQuitKeyAction();
    virtual bool OnKeystroke(MOOSVARMAP& Vars);
  protected:
    virtual void PrintConfirmMessage();
};

/**
 * FIXME: needs class description
 */
class CHelpKeyAction : public CKeyAction
{
  public:
    CHelpKeyAction(){};
    CHelpKeyAction(KEY_MAP* pKeys);

    virtual bool OnKeystroke(MOOSVARMAP& Vars);
  protected:
    KEY_MAP* m_pKeys;
};
#endif //!defined(KEYAction_H)
