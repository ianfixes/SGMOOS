//-----------------------------------------------------------------------------
//  (c) 2002 by Basler Vision Technologies
//  Section:  Vision Components
//  Project:  BVC
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/BvcException.h,v 1.1.2.1 2004/07/27 19:56:00 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
  \file     BvcException.h
 *
  \brief   <type brief description here>
 *
 * <type long description here>
 */
//-----------------------------------------------------------------------------


#if !defined(AFX_BVCEXCEPTION_H__1AFF92A3_4C70_4645_AA91_B114721AB77F__INCLUDED_)
#define AFX_BVCEXCEPTION_H__1AFF92A3_4C70_4645_AA91_B114721AB77F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#include "Exception.h"
namespace Bvc
{
  class BvcException : public Exception  
  {
  public:
    BvcException( Bvc::ErrorNumber e, CString context = _T( "" ), va_list *pArgs = NULL );
    BvcException( Bvc::ErrorNumber e, CString context , PVOID pArg  );
  private:
    static const CString m_LibraryFile;
   
  };
}
#endif // !defined(AFX_BVCEXCEPTION_H__1AFF92A3_4C70_4645_AA91_B114721AB77F__INCLUDED_)
