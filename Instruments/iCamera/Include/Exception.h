// Exception.h: interface for the Exception class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXCEPTION_H__5AEB605F_A0BA_4A42_B47D_149DB9F9FC54__INCLUDED_)
#define AFX_EXCEPTION_H__5AEB605F_A0BA_4A42_B47D_149DB9F9FC54__INCLUDED_

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

#include <assert.h>

namespace Bvc
{

/// The Error Numbers
typedef unsigned long ErrorNumber;

class Exception  
{
public:
  Exception( ErrorNumber e, CString context = _T(""), va_list *pArgs = NULL );
  Exception( ErrorNumber e, CString context , PVOID pArg  );
  Exception( ErrorNumber e, CString context, bool initonly );
  virtual ~Exception();

  /// retrieve the error code
  ErrorNumber Error() const
  {
    return m_Error;
  }
  /// retrieve the error message
  CString Description() const
  {
    return m_Description;
  }
  /// retrieve additional context information
  CString Context() const
  {
    return m_Context;
  }
protected:
  /// creates a new message
  void FormatMessage( char** ppArgs );
  virtual void FormatMessage( const CString& LibraryFile, char** ppArgs );
protected:

  ErrorNumber m_Error;
  CString m_Description;
  CString m_Context;
};

inline Exception::Exception( ErrorNumber e, CString context, va_list *pArgs )
: m_Error( e ), m_Context( context )
{
  FormatMessage( (char**) pArgs );
}

inline Exception::Exception( ErrorNumber e, CString context , PVOID pArg  )
: m_Error( e ), m_Context( context )
{
  FormatMessage( (char**) pArg );
}

inline Exception::Exception( ErrorNumber e, CString context, bool /*initonly*/ )
: m_Error( e ), m_Context( context )
{
}

inline void Exception::FormatMessage( char** ppArgs )
{
  const CString NoResourceFile;
  FormatMessage( NoResourceFile, ppArgs );
}

inline void Exception::FormatMessage( const CString& LibraryFile, char** ppArgs )
{
  // Try to get an error message by using FormatMessage
  // Search sequence: 1. m_LibraryFile
  //                  2. System
  //                  3. Application program
  
  LPVOID lpMsgBuf = NULL;
  int cCopied = 0;
  
  m_Description = _T( "<no message found>" );
  
  if (! LibraryFile.IsEmpty())
  {
    HINSTANCE hLib;  // handle to the messagetable DLL 
    hLib = LoadLibraryEx( LibraryFile, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if ( hLib )
    {
      cCopied = ::FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_HMODULE | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        hLib,
        m_Error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL );
      FreeLibrary(hLib);
    }
  }
  if ( cCopied == 0 )
  {
    // first try to get the message from the system
    cCopied = ::FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
      FORMAT_MESSAGE_FROM_SYSTEM | 
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      m_Error,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &lpMsgBuf,
      0,
      NULL );
    
    if ( cCopied == 0 )
    {
      // Finally try to get a message from the application image file.
      cCopied = ::FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_HMODULE |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // -> try to retrieve message from the application image file
        m_Error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,  
        NULL );
    }
  }

  if ( cCopied != 0 )
  {
    // we got an error message
    m_Description = (LPCTSTR)lpMsgBuf;
    
    if ( ppArgs != NULL )
    {
      cCopied = 0;
      // The user has specified arguments. Process the inserts in lpMsgBuf
      LPVOID lpFormattedMsg = NULL;
      __try
      {
        cCopied = ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_STRING |
          FORMAT_MESSAGE_ARGUMENT_ARRAY,
          lpMsgBuf,
          0,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
          (LPTSTR) &lpFormattedMsg,
          0,
          ppArgs );
      }
      __except( EXCEPTION_EXECUTE_HANDLER)
      {
        // Format Message may cause an access violation, if the user hasn't specified
        // the expected arguments. We will catch such errors here and the user will get 
        // the raw message string with format specifier included;
        m_Description += _T( "<unable to format message>" );
        assert( false && _T( "<unable to format message>" ) );
      }
      if (cCopied != 0 && lpFormattedMsg != NULL)
      {
        m_Description = (LPCTSTR) lpFormattedMsg;
        LocalFree ( lpFormattedMsg );
      }
    }
    // m_Description.AnsiToOem();
    LocalFree (lpMsgBuf);
  }
}

inline Exception::~Exception()
{

}

}
#endif // !defined(AFX_EXCEPTION_H__5AEB605F_A0BA_4A42_B47D_149DB9F9FC54__INCLUDED_)
