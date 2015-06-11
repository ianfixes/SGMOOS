//-----------------------------------------------------------------------------
//  (c) 2002 by Basler Vision Technologies
//  Section:  Vision Components
//  Project:  BCAM
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/md5.h,v 1.1.2.1 2004/07/27 19:56:00 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
  \file     BcamPropertyBag.h
 *
  \brief   MD5 routines.
 *
 * Original version written by Colin Plumb. Adapted for use
 * in BCAM driver context by Thomas Koeller.
 */
//-----------------------------------------------------------------------------


#ifndef MD5_H
#define MD5_H

typedef unsigned long uint32;

struct MD5Context
{
  uint32 buf[4];
  uint32 bits[2];
  unsigned char in[64];
};

#if defined(__cplusplus)
# define externC extern "C"
#else
# define externC
#endif

externC void MD5Init(struct MD5Context *context);
externC void MD5Update(struct MD5Context *context, unsigned char const *buf, unsigned len);
externC void MD5Final(unsigned char digest[16], struct MD5Context *context);

#endif /* !MD5_H */
