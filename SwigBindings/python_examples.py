#/usr/bin/env python

import mooslib

def MOOSComms_NoCallback(sHostName="LocalHost", nPort=9000, sAppName="PYTHON_TEST"):

	print "This is an example of MOOS Comm Client in python (without callbacks)"

	client = mooslib.CMOOSCommClient()
	if client.Run(sHostName,nPort,sAppName):

		sVarName = "DB_TIME"

		print "Registering For " + sVarName
		client.Register(sVarName,0.1)
	
		print "Demonstrate Notifications... posting a string & number to the DB"
		client.Notify("STRING_FROM_PYTHON", "Hello")
		client.Notify("NUMBER_FROM_PYTHON", 3.1415926)

		#wait a second for some mail
		mooslib.MOOSPause(1000)

		print "Checking Mail"
		mail = mooslib.MOOSMSG_LIST()
		client.Fetch(mail)
		Msg = mooslib.CMOOSMsg()
		if client.PeekMail(mail,sVarName,Msg):
			print "I got a message: %s at %f from %s" % (Msg.m_sKey, Msg.m_dfTime, Msg.m_sSrc)
		else:
			print "No Messages"

		client.Close()	
	else:
		print "Failed to connect to a MOOS Community"

if __name__ == "__main__":
	MOOSCommNoCallback()
