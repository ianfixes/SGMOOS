//-----------------------------------------------------------------------------
//  Company:  Basler Vision Technologies
//  Section:  Vision Components
//  Project:  1394 Driver
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/BcamException.h,v 1.1.2.1 2004/07/27 19:56:00 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
  \file     BcamException.h
  \brief    Interface  of the BcamException class

  This interface needs the following classes defined:
    - CString
  Use either WTL or MFC to supply these classes.

*/

#ifndef BCAMEXCEPTION_H
#define BCAMEXCEPTION_H

#if defined( USE_MFC )
#include <afxwin.h>         // MFC core and standard components
#else
#include <windows.h>
#endif

#if defined( USE_WTL)  && _MSC_VER < 1300 
#include <atlapp.h>
#include <atlmisc.h>        // WTL's CSize, CString, ...
#endif  

#if defined( USE_MFC ) && _MSC_VER >= 1300 
#include <afxstr.h>  // MFC7 CString
#endif

#if ! defined( USE_MFC ) && _MSC_VER >= 1300
#include <atlstr.h>  // ATL CString
#endif

#include "exception.h"

namespace Bcam
{

//------------------------------------------------------------------------------
// class BcamException
// Author: A.Happe
// Date: 28.07.01
//------------------------------------------------------------------------------
/**
 * \brief   Exception class thrown by the Bcam API
 *
 * 
 * 
 * 
 * \todo      
 */
//------------------------------------------------------------------------------
  class BcamException : public Bvc::Exception 
  {
  public:
    // constructors and destructors
    BcamException( Bvc::ErrorNumber e, CString context = _T(""), va_list *pArgs = NULL );
    BcamException( Bvc::ErrorNumber e, CString context , PVOID pArg  );
  protected:
    void FormatMessage( const CString& LibraryFile, char** ppArgs );
  private:
    static const CString m_LibraryFile;
    static const unsigned int m_FacilityCode;
  };

}

#endif //__BCAMEXCEPTION_H__