%module mooslib

%{
// Includes the header in the wrapper code 
#include "MOOSGenLib/MOOSGenLibGlobalHelper.h"
#include "MOOSLIB/MOOSGlobalHelper.h"
#include "MOOSLIB/MOOSMsg.h"
#include "MOOSLIB/MOOSCommClient.h"
#include "MOOSLIB/MOOSCommPkt.h"
%}

//%include stl.i
/* instantiate the required template specializations */
//namespace std {
//    %template(MOOSMSG_LIST) list<CMOOSMsg>;
//}


/* Parse the header file to generate wrappers */
%include "MOOSGenLib/MOOSGenLibGlobalHelper.h"
%include "MOOSLIB/MOOSGlobalHelper.h"
%include "MOOSLIB/MOOSMsg.h"
%include "MOOSLIB/MOOSCommObject.h"
%include "MOOSLIB/MOOSCommClient.h"
%include "MOOSLIB/MOOSCommPkt.h"



