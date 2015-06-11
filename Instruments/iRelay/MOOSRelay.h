// Ex1/MOOSRelay.h: interface for the CMOOSRelay class.

#ifndef RELAY_H
#define RELAY_H

#include <MOOSLIB/MOOSInstrument.h>

class CMOOSRelay : public CMOOSInstrument  
{
public:
    //standard construction and destruction
    CMOOSRelay();
    virtual ~CMOOSRelay();

protected:
    //where we handle new mail
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    //where we do the work
    bool Iterate();
    //called when we connect to the server
    bool OnConnectToServer();
    //called when we are starting up..
    bool OnStartUp();

    bool DoEnable(bool bEnable);

};

#endif 
