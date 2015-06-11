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
// StateMachine.h - for controlling a program's state
//
/////////////////////////////////////


#if !defined(AFX_STATEMACHINE_H_INCLUDED_)
#define AFX_STATEMACHINE_H_INCLUDED_

#include <stdio.h>
#include <string>

using namespace std;

typedef list<string> STRING_LIST;

class CState
{
public:

    typedef struct {
        bool (*action)(void);
        string nextstate;
    } outcome;
    
    typedef map<string, outcome> TRANSITION_MAP;

    //states have a name, a default action, and a default destination state
    CState(string name, 
           bool (*defaultAction)(void), 
           string defaultTransitionState)
    {
        m_sName = name;
        m_sDefTransName = defaultTransitionState;
        m_defaultAction = defaultAction;
    }
    
    ~CState() {}
    
    string GetName() const { return m_sName; }
    string GetDefaultTransition() const { return m_sDefTransName; }
    int NumTransitions() const { return m_transitionMap.size(); }

    void AddTransition(string symbol, bool (*action)(void), string nextstate)
    {
        m_transitionMap[symbol] = (outcome){action, nextstate};
    }
    
    bool Handle(string event)
    {
        bool (*f)(void);
        //try event action if it's in the transition map
        if (m_transitionMap.end() != m_transitionMap.find(event))
        {
            //if action is not null, try it (else true)
            bool ret = (f = m_transitionMap[event].action) ? (f)() : true;
            return ret;
        }

        //fprintf(stderr, "StateMachine: %s doesn't understand event %s\n",
        //       m_sName.c_str(), event.c_str());

        //try default
        if (m_defaultAction)
            return (m_defaultAction)();
       
        //if no default action, probly means we want to just stay on this state
        return true;
    }

    string Next(string event)
    {
        //if no matching transition exists, choose default
        if (m_transitionMap.end() != m_transitionMap.find(event))
            return m_transitionMap[event].nextstate;
        return m_sDefTransName;
    }

    STRING_LIST AllTransitionStates()
    {
        STRING_LIST sl;
        for (TRANSITION_MAP::iterator it = m_transitionMap.begin(); it != m_transitionMap.end(); it++)
        {
            sl.push_back(it->second.nextstate);
        }
        sl.push_back(m_sDefTransName);
        return sl;
    }

    STRING_LIST AllEvents()
    {
        STRING_LIST sl;
        for (TRANSITION_MAP::iterator it = m_transitionMap.begin(); it != m_transitionMap.end(); it++)
        {
            sl.push_back(it->first);
        }
        return sl;
    }

protected:
    TRANSITION_MAP m_transitionMap;
    string m_sName;
    string m_sDefTransName;
    bool (*m_defaultAction)(void);

};



class CStateMachine
{
public:
    CStateMachine() { m_bAutoAdvance = false; };
    virtual ~CStateMachine()
    {
        for (STATE_MAP::iterator it = m_states.begin(); it != m_states.end(); it++)
            delete it->second;
    }
    
    typedef map<string, CState*> STATE_MAP;

    void AddState(CState* s)
    {
        m_states[s->GetName()] = s;
    }

    void SetAutoAdvance(bool yes) { m_bAutoAdvance = yes; }        
    void Start(string name) { m_sCurrentState = name; }
    string CurrentState() const{ return m_sCurrentState; }
    STRING_LIST ValidActions() { return m_states[m_sCurrentState]->AllEvents(); }

    bool Iterate(string event) 
    { 
        CState* s;
        //advance until we hit a state with a transition list
        do
        {
            //fprintf(stderr, "Statemachine: iterating event '%s' on state '%s'\n",
            //        event.c_str(), m_sCurrentState.c_str());
            //if there is no way to handle event, ignore
            if (false == ((s = m_states[m_sCurrentState])->Handle(event)))
            {
                //big problem if there is self-reference but no transitions
                if (0 == s->NumTransitions() && s->GetName() == s->GetDefaultTransition())
                {
                    fprintf(stderr, "Statemachine: '%s' has no transitions; inf. loop\n",
                            s->GetName().c_str());
                    return false;
                }

                break;
            }

            //advance pointer
            m_sCurrentState = s->Next(event);

        } 
        while (m_bAutoAdvance && 0 == m_states[m_sCurrentState]->NumTransitions());

        return true;
    }

    //check for transitions to nonexistent states
    bool IsValid()
    {
        STATE_MAP::iterator    it_s;
        STRING_LIST::iterator  it_t;
        STRING_LIST sl;

        bool ret = true;

        for (it_s = m_states.begin(); it_s != m_states.end(); it_s++)
        {
            sl = it_s->second->AllTransitionStates();
            for (it_t = sl.begin(); it_t != sl.end(); it_t++)
            {
                if (m_states.end() == m_states.find(*it_t)) 
                {
                    fprintf(stderr, "Statemachine: '%s' transitioned to nonexistent '%s'",
                            it_s->first.c_str(), it_t->c_str());
                    ret = false;
                }
            }
        }

        return ret;
        
    }



protected:
    string m_sCurrentState;
    STATE_MAP m_states;
    bool m_bAutoAdvance;


};

#endif // !defined(AFX_STATEMACHINE_H_INCLUDED_)

