#!/usr/bin/env python

import pyMOOS

class TestApp(pyMOOS.CMOOSApp):
    def __init__(self):
        pyMOOS.CMOOSApp.__init__(self)
         
    def Iterate(self):
        #print "Iterate:",pyMOOS.MOOSTime()
        self.m_Comms.Notify("PYTHON_NUMBER", 3.14159)
        self.m_Comms.Notify("PYTHON_STRING", "spamSpamSPAM")
        return True

    def OnNewMail(self, mail):

        newMsg = pyMOOS.CMOOSMsg()
        if self.m_Comms.PeekMail(mail,"DB_UPTIME",newMsg):
            print "MOOSDB Uptime:", newMsg.m_dfVal

#        for m in mail:
#            if m.IsString():
#                print m.m_sVal 
#            else:
#                print m.m_dfVal

        return True

    def OnStartUp(self):
        print "Starting up now."
        return True 

    def OnConnectToServer(self):
        self.m_Comms.Register("DB_UPTIME",0.1)
        return True

if __name__ == "__main__":

    app = TestApp()
    try:
        app.Run("pyApp","Mission.moos")
    except KeyboardInterrupt:
        print "Goodbye"

    del(app)
        
