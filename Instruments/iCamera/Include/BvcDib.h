//-----------------------------------------------------------------------------
//  (c) 2002 by Basler Vision Technologies
//  Section:  Vision Components
//  Project:  BVC
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/BvcDib.h,v 1.1.2.1 2004/07/27 19:56:00 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
  \file     BvcDib.h
 *
  \brief   <type brief description here>
 *
 * <type long description here>
 */
//-----------------------------------------------------------------------------


#if !defined(AFX_BVCDIB_H__BFF0F37D_BC2D_4101_9C30_B2ECC740580D__INCLUDED_)
#define AFX_BVCDIB_H__BFF0F37D_BC2D_4101_9C30_B2ECC740580D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined( USE_MFC )
#include <afxwin.h>         // MFC core and standard components
#else
#include <windows.h>
#endif

#if defined( USE_WTL )
#include <atlapp.h>
#include <atlmisc.h>        // WTL's CSIZE,...
#endif  

#if defined( USE_WTL)  && _MSC_VER < 1300 
#include <atlapp.h>
#include <atlmisc.h>        // WTL's CSize, CString, ...
#endif  

#if _MSC_VER >= 1300
#include <atltypes.h>  // CSize, CPoint, ...
#endif


#include "bvcexception.h"

#ifdef IPL_SUPPORT
#include <ipl.h>
#endif

namespace Bvc
{
  class CDibPtr;
  
  //! Encapsulates a Windows DIB section and implements some missing features
  /*! The Windows GDI Bitmap object has two drawbacks:
  - It does not handle the image's orientation (bottom up or top down) properly
  - It does not know about monochrome palettes
  
    This class overcomes these drawbacks. It encapsulates a Windows GDI Bitmap handle
    and adds the missing functionality.
    
      The life time of CDib objects is controlled by a reference counter implementing the same semantics 
      as COM objects.
      
        This class is intended to be used only via the corresponding smart pointer #CDibPtr.
        To ensure this the constructor, the creator functions and the functions to 
        increase and decrease the reference counter are accessible through CDibPtr only.
        
  */
  class CDib 
  {
    friend CDibPtr;
    
    // Public enums
  public:
    //! Orientation of the image
    enum Orientation_t
    {
      BottomUp, //!< The bottom line of the image is located at lower adress than the top line
        TopDown   //!< The top line of the image is located at lower adress than the bottom line
    };
    
    //! The way the colors in the palette are choosen
    enum PaletteType_t
    {
      Color,          //!< The resulting image is a color image
        Monochrome      //!< The resulting image is a monochrome image
    };
    
    // Protected creation functions anly to be accessed by CDibPtr
  protected:
    //! Default constructor
    CDib();
    //! Destroy all BMP related resources 
    void Destroy(); 
    //! Increments the reference counter of this object
    long AddRef();
    //! Decrements the reference counter of this object. Destroys the object if the reference counter becomes 0.
    void Release();
    //! Create a CDib object
    static CDib* Create( CSize Size, unsigned int BitsPerPixel, Orientation_t Orientation, PaletteType_t PaletteType ) ;
    //!  Loads the bitmap from a BMP file
    static CDib* LoadBMP( CString FileName );
  public:
    //! Virtual destructor
    virtual ~CDib();
    //! checks if twobitmaps share the same properties
    bool IsEqualType( const CDib& Dib) const;
    //!  Stores the bitmap to a BMP file
    void StoreBMP( CString FileName ) const;
    //! Gets a HBITMAP handle (no ownership is granted!)
    HBITMAP GetBitmapHandle() const;
    //!  Returns a pointer to the pixel data 
    void* GetPixels() const;
    //!  Returns the bitmap's size 1
    void GetSize(CSize *pSize) const;
    //!  Returns the bitmap's size 2
    CSize GetSize() const;
    //!  Returns the total size of a line in bytes
    unsigned long GetWidthBytes() const;
    //!  Returns the total size of the image in bytes
    unsigned long GetTotalPixelBytes() const;
    //!  Returns the bitmap's bits per pixel
    unsigned short GetBitsPerPixel() const;
    //! Returns the bitmaps orientation (BottomUp or TopDown)
    Orientation_t GetOrientation() const;
    //! Returns the bitmaps palette type (e.g., color or monochrome )
    PaletteType_t GetPaletteType() const;
    //! Returns the bitmaps BITMAPINFORHEADER 
    void GetBitmapInfoHeader(BITMAPINFOHEADER *pBitmapInfoHeader) const;
    //! Returns a bitmaps BITMAPINFO which has to be freed by the user
    void GetBitmapInfo(BITMAPINFO **ppBitmapInfo, unsigned long *pBitmapInfoBytes) const;
    //! Returns the bitmaps BITMAP 
    void GetBitmap(BITMAP *pBitmap) const;
    //!  Topples the bitmap
    void Topple();
    //! Clones the Bitmap 
    CDib* Clone(
      bool Topple = false  //!< If true the clone will be toppled with resprect to the original bitmap
      ) const;
    //! Copies the image to the clipboard
    void CopyToClipboard();
#ifdef IPL_SUPPORT
    //! Copies the image to the clipboard
    IplImage* GetIplImage()
    {
      return m_pIplImage;
    }
    
#endif // #ifdef IPL_SUPPORT
    
    // Helper functions
  protected:
    //!  Returns a DIB section which is a copy of the referenced bitmap
    HBITMAP GetCopy() const;
    //!  Returns a DIB section which is a toppled copy of the referenced bitmap
    HBITMAP GetToppledCopy() const;
    //!  This function is called whenever the bitmap is re-created
    void Refresh();
    // Member Variables
  protected:
    //! Handle of the referenced Bitmap
    HBITMAP m_hBitmap;
    
    //! Indicates the orientation of the image (topdown or bottomup)
    /*! In Windows a Bitmap object can be created topdown by setting Height < 0.
    For some strange reason the BITMAP structure returned from a topdown
    bitmap will have Height > 0 so the information has to be cached here.
    */
    Orientation_t m_Orientation;
    
    //! Indicates if the bitmap is monochrome or color
    /*! In Windows a monochrome Bitmap is a 8 bit palletized image with a special
    palette : RGB(i) = {i,i,i}. This information is cached.
    */
    PaletteType_t m_PaletteType;
    
    //! Reference counter controlling the life of the CDib object 
    //! Same semantic as in COM objects
    mutable long m_RefCount;
    
#ifdef IPL_SUPPORT
    //! Pointer to a header describing the bitmap as IPL image
    IplImage* m_pIplImage;
    
#endif 
    
};
//! Smart pointer to CDib objects
/*! This class implements the same semantics as a COM smart pointer.
In addition it provides creator functions to create CDib objects.
*/
class CDibPtr
{
public:
  //! Default constructor
  CDibPtr()
  {
    m_pDib = NULL;
  }
  
  explicit CDibPtr( CDib* pDib)
    : m_pDib( pDib )
  {
    if (m_pDib)
      pDib->AddRef();
  }

  //! Copy constructor 1
  CDibPtr( const CDibPtr * const pDibPtr )
  {
    if(pDibPtr != NULL)
    {
      if ((m_pDib = pDibPtr->m_pDib) != NULL)
        m_pDib->AddRef();
    }
    else
      m_pDib = NULL;
  }
  
  //! Copy constructor 2
  CDibPtr( const CDibPtr& DibPtr)
  {
    if ((m_pDib = DibPtr.m_pDib) != NULL)
      m_pDib->AddRef();
  }
  
  //! Creates a bismap object
  void Create( CSize Size, 
    unsigned int BitsPerPixel, 
    CDib::Orientation_t Orientation = CDib::BottomUp, 
    CDib::PaletteType_t PaletteType = CDib::Color) 
  {
    Release();
    
    m_pDib = CDib::Create( Size, BitsPerPixel, Orientation, PaletteType );
  }
  
  //!  Loads a bitmap from a BMP file
  void LoadBMP( CString FileName )
  {
    Release();
    
    m_pDib = CDib::LoadBMP( FileName );
  }
  
  //! virtual Destructor
  virtual ~CDibPtr()
  {
    if(m_pDib)
      m_pDib->Release();
  }
  
  //! releases the referenced CDib object
  void Release()
  {
    CDib* pTemp = m_pDib;
    if (pTemp)
    {
      m_pDib = NULL;
      pTemp->Release();
    }
  }
  
  //! Gets access to the referenced CDib object
  CDib& operator*() const
  {
    if(!m_pDib)
      throw  BvcException( Bvc::ErrorNumber( E_POINTER ), _T( "CDibPtr::operator*" ) );
    
    return *m_pDib;
  }
  
  //! Gets access to the referenced CDib object
  CDib* operator->()
  {
    if(!m_pDib)
      throw  BvcException( Bvc::ErrorNumber( E_POINTER ), _T( "CDibPtr::operator->" ) );
    
    return m_pDib;
  }
  const CDib* const operator->() const
  {
    if(!m_pDib)
      throw  BvcException( Bvc::ErrorNumber( E_POINTER ), _T( "CDibPtr::operator->" ) );
    
    return m_pDib;
  }
  
  //! Assigns a CDib pointer 
  CDibPtr& operator=( CDib * const pDib)
  {
    Release();
    
    if(pDib)
    {
      m_pDib = pDib;
      if(m_pDib)
        m_pDib->AddRef();
    }
    else
      m_pDib = NULL;
    
    return *this;
  }
  
  //! Assigns a smart pointer 1
  CDibPtr& operator=( const CDibPtr * const pDibPtr)
  {
    Release();
    
    if(pDibPtr)
    {
      m_pDib = pDibPtr->m_pDib;
      if(m_pDib)
        m_pDib->AddRef();
    }
    else
      m_pDib = NULL;
    
    return *this;
  }
  
  //! Assigns a smart pointer 2
  CDibPtr& operator=( const CDibPtr& DibPtr)
  {
    Release();
    
    m_pDib = DibPtr.m_pDib;
    if(m_pDib)
      m_pDib->AddRef();
    
    return *this;
  }
  
  //! True, if this smart pointer does not reference a CDib object
  bool IsNull() const
  {
    return (m_pDib == NULL);
  }
  
  //! True, if this smart pointer references a CDib object
  operator bool() const
  {
    return !IsNull();
  }
  
  //! True, if this smart pointer does not reference a CDib object
  bool operator!() const
  {
    return IsNull();
  }
  
  //! Compares if two smart pointers are equal
  bool operator==( const CDibPtr& DibPtr) const
  {
    return m_pDib == DibPtr.m_pDib;
  }
  bool operator==( int null ) const
  {
    if (NULL != null) 
      throw BvcException( Bvc::ErrorNumber( Bvc::ErrorNumber( E_POINTER ) ), _T( "Argument must be NULL!" ) );
    return NULL == m_pDib;
  }

  //! Pointer to the referenced CDib object
  CDib* m_pDib;
};
} // namespace Bvc
#endif // !defined(AFX_BVCDIB_H__BFF0F37D_BC2D_4101_9C30_B2ECC740580D__INCLUDED_)
