//-----------------------------------------------------------------------------
//  Company:  Basler Vision Technologies
//  Section:  Vision Components
//  Project:  1394 Driver
//  <b> Visual SourceSafe Information:</b>
//  $Archive: /Software/src/BCAMViewer/CreateCorrectionTable.h $
//  $Author: moosauthor $
//  $Revision: 1.1.2.1 $
//  $Date: 2004/07/27 19:56:00 $
//-----------------------------------------------------------------------------
/**
  \file     CreateCorrectionTable.h
 *
 * 
 *
  \brief  Calculation of a shading correction table
 */
//----------------------------------------------------------------------------

#include "Bcam.h"
using namespace Bcam;

//------------------------------------------------------------------------------
// SCMode_t CreateShadingCorrectionTable(int nImages, CBcam& Bcam, PBYTE pTable, CRect ROI = CRect(0,0,0,0) )
// Author: 
//------------------------------------------------------------------------------
/**
 * <type function description here>
 *
 * \param     nImages   Number of images used to calculate average image
 * \param     Bcam      Bcam device
 * \param     pTable    Pointer to shading table. The user has to provide the memory (according to image size)
 * \param     ROI       (optional) Specify a region in which darkest and lightest pxiel will be searched. 
 *                      If no or an empty region is specified, the whole image area will be taken into account

 * \return    Shading mode to apply in combination with calculated correction table. If shading correction isn't 
              supported or the camera is in color image mode, SCM_Disabled will be returned
 *
 */
//------------------------------------------------------------------------------

SCMode_t CreateShadingCorrectionTable(int nImages, CBcam& Bcam, PBYTE pTable, CRect ROI = CRect(0,0,0,0) )
{
  // precondition: the Bcam device is already opened and 1394 resources are allocated

  DCSVideoFormat format;
  DCSVideoMode   mode;
  DCSColorCode   code;
  CSize          imageSize;
  double*        pAvg = NULL;
  int            x, y;
  unsigned char  min = 255;
  unsigned char  max = 0;
  unsigned char  pixel;
  SCMode_t       shadingMode;

 
  try
  {
    
    // check if camera supportes shading correction
    if ( ! Bcam.ShadingCorrection.IsSupported() )
      return SCM_Disabled;
    
    Bcam.GetVideoMode(&format, &mode, NULL);
    
    // determine current color coding. Calculating a shading correction table is only supported for
    // monochrome video formats
    code = format != DCS_Format7 ? BcamUtility::ColorCode(format, mode) : Bcam.FormatSeven[mode].ColorCoding();
    if ( code != DCSColor_Mono8 )
      return SCM_Disabled;
    
    // determine image sizes
    imageSize = format != DCS_Format7 ? BcamUtility::ImageSize(format, mode) : Bcam.FormatSeven[mode].Size();
    if ( ! ROI.IsRectEmpty() ) 
      ROI.IntersectRect(ROI, CRect(0, 0, imageSize.cx, imageSize.cy) ); // ensure that ROI isn't greater than image
    
    // create buffer to hold the average image
    pAvg = new double[imageSize.cx * imageSize.cy];
    if ( pAvg == NULL )
      throw BcamException(E_OUTOFMEMORY);
    ZeroMemory(pAvg, sizeof(double) * imageSize.cx * imageSize.cy);
    
    // calculate sum of nImages images ( pTable is used as temporary image buffer )
    for ( int i = 0; i < nImages; ++i )
    {
      Bcam.GrabImage(pTable, imageSize.cx * imageSize.cy, 5000);
      for ( y = 0; y < imageSize.cy; ++y )
        for ( x = 0; x < imageSize.cx; ++x )
          pAvg[x + y * imageSize.cx ] += pTable[x + y * imageSize.cx];
    }
    
    // Calculate average image and determine lightest and darkest pixel values in area specified by ROI.
    // If the ROI is empty, the search will be extended to the whole image area.
    for ( y = 0; y < imageSize.cy; ++y )
    {
      for ( x = 0; x < imageSize.cx; ++x )
      {
        unsigned char avg = ( pAvg[x + y * imageSize.cx] /= nImages ); 
        if ( ROI.IsRectEmpty() || ROI.PtInRect(CPoint(x, y)) ) 
        {
          if ( avg < min ) 
            min = avg;
          else if ( avg > max )
            max = avg;
        }
      }
    }
    
    // Determine the suited shading mode
    shadingMode = min > 0.3 * max ? SCM_1_3 : SCM_1_5;
    
    // Calculate the shading correction table
    for ( y = 0; y < imageSize.cy; ++y )
    {
      for ( x = 0; x < imageSize.cx; ++x )
      {
        pixel = pAvg[x + y * imageSize.cx ];
        pTable[ x + y * imageSize.cx ] = pixel == 0 ? 0 : shadingMode == SCM_1_3 ?
          ( max * 128.0 ) / pixel - 128 : 
        ( max * 64.0 ) / pixel - 64;
      }
    }
    if ( pAvg != NULL )
      delete[] pAvg;

  }
  catch ( BcamException& e )
  {
    if ( pAvg != NULL )
      delete[] pAvg;
    throw e;
  }

  return shadingMode;
}





 






