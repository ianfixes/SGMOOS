//*********************************************************
//
//  (c) 2001 by Basler Vision Technologies
//
//  Section : VC
//  Project : BcamDrv
//
//  Description:
//  This file defines device class identifier codes for
//  Basler drivers. These codes are used in construction
//  Driver function codes.
//
//  File    : $Archive: $
//  Author  : $Author: moosauthor $
//  Revision: $Revision: 1.1.2.1 $ / $Date: 2004/07/27 19:56:00 $
//
//*********************************************************

#if !defined(CLASSCODE_H)
#define CLASSCODE_H

/*
 * Valid device class IDs range from 0x8000 to 0xffff. We start at 0x8010
 * because the lower codes have already been used for frame grabber drivers.
 */
enum
{
  FILE_DEVICE_BCAM_1394 = 0x8010
};

#endif /* !defined(CLASSCODE_H) */