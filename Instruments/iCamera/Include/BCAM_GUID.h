//-----------------------------------------------------------------------------
//  Company:  Basler Vision Technologies
//  Section:  Vision Components
//  Project:  1394 Driver
//  Subproject:  Bus Access Driver
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/BCAM_GUID.h,v 1.1.2.1 2004/07/27 19:55:59 moosauthor Exp $
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


// {A046BA53-4E42-490c-B6E4-F64167010693}
DEFINE_GUID( GUID_FILE_DEVICE_BCAM_1394, 0xa046ba53, 0x4e42, 0x490c, 0xb6, 0xe4, 0xf6, 0x41, 0x67, 0x1, 0x6, 0x93);
