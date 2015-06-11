//-----------------------------------------------------------------------------
//  Company:  Basler Vision Technologies
//  Section:  Vision Components
//  Project:  1394 Driver
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/BACC_GUID.h,v 1.1.2.1 2004/07/27 19:55:59 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
  \file    Bcam_GUID..h
  \brief   Device class GUIDs
 *
 * 
 */
//-----------------------------------------------------------------------------

/**
 * Device class GUIDs. These are used to identify device interfaces. GUID definitions must
 * be outside the section protected from multiple inclusion, for reasons explained in the
 * DDK docs (but not easily understood).
 */

#ifdef INITGUID
#define BCAM_GUID_INITIALIZED
#endif


// {6E0B1690-10F4-11d6-91AD-005004384DF1}
DEFINE_GUID(GUID_BACC_DEVICE, 0x6e0b1690, 0x10f4, 0x11d6, 0x91, 0xad, 0x0, 0x50, 0x4, 0x38, 0x4d, 0xf1);
#define GUID_BACC_DEVICE_STR "6E0B1690-10F4-11d6-91AD-005004384DF1"

