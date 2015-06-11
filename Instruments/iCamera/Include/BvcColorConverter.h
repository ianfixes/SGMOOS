//-----------------------------------------------------------------------------
//  (c) 2002 by Basler Vision Technologies
//  Section:  Vision Components
//  Project:  BVC
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/BvcColorConverter.h,v 1.1.2.1 2004/07/27 19:56:00 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
  \file     BvcColorConverter.h
 *
  \brief   <type brief description here>
 *
 * <type long description here>
 */
//-----------------------------------------------------------------------------

#if !defined(AFX_BVCCOLORCONVERTER_H__FE22DC3C_A5BF_4487_A323_84DE8CC72022__INCLUDED_)
#define AFX_BVCCOLORCONVERTER_H__FE22DC3C_A5BF_4487_A323_84DE8CC72022__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <assert.h>

#if defined( USE_MFC )
#include <afxwin.h>         // MFC core and standard components
#else
#include <windows.h>
#endif

#if defined( USE_WTL )
#include <atlapp.h>
#include <atlmisc.h>        // WTL's CSIZE,...
#endif  

#if _MSC_VER >= 1300
#include <atltypes.h>  // CSize, CPoint, ...
#endif


namespace Bvc
{

  //! The length of the look-up tables
  enum{ LUT_LENGTH = 256 };

  class CDibPtr;

  //! Converter for various color formats in RGB8
  class CColorConverter
  {
  public:
    //! Indicates the Bayer pattern of the first row of an image.
    enum  PatternOrigin_t
    {
      poGB = 1, //!< Bayer Image starts with a GBGB row
      poGR,     //!< Bayer Image starts with a GRGR row
      poB,      //!< Bayer Image starts with a BGBG row
      poR       //!< Bayer Image starts with a RGRG row
    };

  //! Converts a Bayer8 to a RGB8 image (works on CDibPtr) 
    static void ConvertMono8ToRGB(CDibPtr &ptrDest, CDibPtr &ptrSource, PatternOrigin_t PatternOrigin);

    //! Converts a Bayer8 to a RGB8 image with LUTs (works on CDibPtr) 
    static void ConvertMono8ToRGB(CDibPtr &ptrDest, CDibPtr &ptrSource, PatternOrigin_t PatternOrigin, 
       BYTE pLutR[LUT_LENGTH],  BYTE pLutG[LUT_LENGTH],  BYTE pLutB[LUT_LENGTH]);

    //! Converts a 422YUV8 to a RGB8 image (the destination image is a CDibPtr)
    static void ConvertYUV422ToRGB(CDibPtr &ptrDest, PBYTE pSource);

    //! Converts a 422YUV8 to a RGB8 image (works on raw pointers)
    static void ConvertYUV422ToRGB(PBYTE pDest,  PBYTE pSource,  CSize& Size, unsigned int lineoffset = 0);

    //! Converts a Bayer8 to a RGB8 image with LUTs (works on raw pointers)
    static void ConvertMono8ToRGB(PBYTE pDest,  PBYTE pSource,  CSize& Size, PatternOrigin_t PatternOrigin, 
       BYTE pLutR[LUT_LENGTH],  BYTE pLutG[LUT_LENGTH],  BYTE pLutB[LUT_LENGTH]);

    protected:
    //! Converts Bayer8 to RGB8 (lines with green/blue color pattern)
    static void ProcessGBLines(RGBTRIPLE* pDest,  PBYTE pSource,  CSize& Size, unsigned int lineoffset,
       BYTE pLutR[LUT_LENGTH],  BYTE pLutG[LUT_LENGTH],  BYTE pLutB[LUT_LENGTH]);

    //! Converts Bayer8 to RGB8 (lines with red/green color pattern)
    static void ProcessRGLines(RGBTRIPLE* pDest,  PBYTE pSource,  CSize& Size, unsigned int lineoffset,
       BYTE pLutR[LUT_LENGTH],  BYTE pLutG[LUT_LENGTH],  BYTE pLutB[LUT_LENGTH]);

    //! Converts Bayer8 to RGB8 (lines with blue/green color pattern)
    static void ProcessBGLines(RGBTRIPLE* pDest,  PBYTE pSource,  CSize& Size, unsigned int lineoffset,
       BYTE pLutR[LUT_LENGTH],  BYTE pLutG[LUT_LENGTH],  BYTE pLutB[LUT_LENGTH]);

    //! Converts Bayer8 to RGB8 (lines with green/red color pattern)
    static void ProcessGRLines(RGBTRIPLE* pDest, PBYTE pSource,  CSize& Size, unsigned int lineoffset,
       BYTE pLutR[LUT_LENGTH],  BYTE pLutG[LUT_LENGTH],  BYTE pLutB[LUT_LENGTH]);


  };

} // namespace Bvc
#endif // !defined(AFX_BVCCOLORCONVERTER_H__FE22DC3C_A5BF_4487_A323_84DE8CC72022__INCLUDED_)
