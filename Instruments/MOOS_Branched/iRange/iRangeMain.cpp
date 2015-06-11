#include <MOOSLIB/MOOSLib.h>

#include "RangeInstrument.h"
int main(int argc ,char * argv[])
{
	const char * sMissionFile = "Mission.moos";
	const char * sMOOSName = "iRange";

    switch(argc)
    {
	case 3:
        sMOOSName = argv[2];
	case 2:
		sMissionFile = argv[1];
    }

    CRangeInstrument RangeInstrument;

    RangeInstrument.Run(sMOOSName,sMissionFile);


    return 0;
}
