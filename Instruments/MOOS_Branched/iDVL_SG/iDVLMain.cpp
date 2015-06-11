#include <MOOSLIB/MOOSLib.h>

#include "DVLInstrument.h"

int main(int argc ,char * argv[])
{
	const char * sMissionFile = "Mission.moos";
	const char * sMOOSName = "iDVL";

    switch(argc)
    {
	case 3:
        sMOOSName = argv[2];
	case 2:
		sMissionFile = argv[1];
    }

    CDVLInstrument DVLInstrument;

    DVLInstrument.Run(sMOOSName,sMissionFile);


    return 0;
}
