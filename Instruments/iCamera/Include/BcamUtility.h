//-----------------------------------------------------------------------------
//  (c) 2003 by Basler Vision Technologies
//  Section:  Vision Components
//  Project:  BCAM
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/BcamUtility.h,v 1.1.2.1 2004/07/27 19:56:00 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
  \file    BcamUtility.h
  \brief   Wrapper classes for the Win32 API

  This file includes following wrappers for win32 objects:
  \li CThread
  \li CEvent
 */
//-----------------------------------------------------------------------------

#ifndef BCAMUTILITY_H
#define BCAMUTILITY_H

#if MSCVER>1000
#pragma once
#endif

#if defined( USE_MFC )
#include <afxwin.h>         // MFC core and standard components
#else
#include <windows.h>
#endif
#include <cassert>

#ifndef TRACE
/// In case we are using WTL replace TRACE calls by AtlTrace
#define TRACE AtlTrace
#endif

namespace Bcam
{

//! Wrapper class for the Win32 thread api
class CThread
{
public:
  
  //! Default constructor 
  CThread()
  {
    m_hThread = NULL;
    m_ID = 0;
    m_bSuspended = false;
  }
  
  //! destructor
  virtual ~CThread() 
  { 
    Release(1000); 
  }
  
  //! Creates a thread
  BOOL Create(
    LPTHREAD_START_ROUTINE pProcess,        //!< Thread function
    LPVOID pParam=NULL,                     //!< Parameter handed over to the thread function
    int Priority=THREAD_PRIORITY_NORMAL,    //!< The thread's priority
    DWORD CreationFlags = 0                 //!< Flags controlling the thread's creation
    )
  {
    assert(pProcess);
    m_hThread = ::CreateThread(NULL, 0, pProcess, pParam, CreationFlags, &m_ID);
    TRACE( _T( "CThread.Create(). Handle = %x, ID = %x\n" ), m_hThread, m_ID);
    m_bSuspended = CreationFlags && CREATE_SUSPENDED;
    if( m_hThread==NULL ) 
      return FALSE;
    if( !::SetThreadPriority(m_hThread, Priority) ) {
      ::CloseHandle(m_hThread);
      return FALSE;
    }
    return TRUE;
  }
  
  //! Close the thread handle and wait for the thread to die
  BOOL Release(DWORD timeout = INFINITE)
  {
    if( m_hThread==NULL ) 
      return TRUE;
    if ( WaitForSingleObject(m_hThread, timeout) != WAIT_OBJECT_0 )
    {
      // The thread didn't die. Terminate it
      TRACE( _T( "Must terminate thread. Handle = %x\n" ), m_hThread);
      Terminate(0);
      WaitForSingleObject(m_hThread, 10000);
    }

    return Detach();
  }

  //! Close the thread handle, but don't wait for the thread to die
  BOOL Detach()
  {
    if ( m_hThread == NULL )
      return TRUE;
    if( ::CloseHandle(m_hThread)==FALSE ) 
      return FALSE;
    m_hThread = NULL;
    m_ID = 0;
    return TRUE;
  }
  
  //! Set the thread's priority
  BOOL SetPriority(int Priority)
  {
    assert(m_hThread);
    return ::SetThreadPriority(m_hThread, Priority);
  }
  
  //! Suspend the thread
  BOOL Suspend()
  {
    assert(m_hThread);
    if( m_bSuspended ) 
      return TRUE;
    if( ::SuspendThread(m_hThread)==-1 ) 
      return FALSE;
    m_bSuspended = true;
    return TRUE;
  }
  
  //! Resume the thread
  BOOL Resume()
  {
    assert(m_hThread);
    if( !m_bSuspended ) return TRUE;
    if( ::ResumeThread(m_hThread)==-1 ) return FALSE;
    m_bSuspended = false;
    return TRUE;
  }
  
  //! Terminate the thread (don't...)
  BOOL Terminate(DWORD dwExitCode)
  {
    assert(m_hThread);
    return ::TerminateThread(m_hThread, dwExitCode);
  }
  
  //! Get the thread's exit code
  BOOL GetExitCode(DWORD *pExitCode)
  {
    assert(m_hThread);
    assert(pExitCode);
    return ::GetExitCodeThread(m_hThread, pExitCode);
  }
  
  //! Get the thread's handle
  operator HANDLE() 
  { 
    return m_hThread; 
  }

  //! Get the thread's ID
  operator DWORD()
  {
    return m_ID;
  }
  
  //! Check if the thread is currently suspended
  bool IsSuspended() 
  { 
    return m_bSuspended; 
  }
  
private:
  //! The thread's handle
  HANDLE m_hThread;

  //! The thread's ID
  DWORD m_ID;
  
  //! Indicates if the thread is currently suspended
  bool m_bSuspended;
  
};  

//! Wrapper class for the Win32 events
class CEvent
{
  HANDLE m_hEvent;

public:
  CEvent()
  {
    m_hEvent = NULL;
  }

  CEvent( const CEvent & src )
  {
    Duplicate(&m_hEvent, src.m_hEvent);
  }

  CEvent & operator =(const CEvent& src)
  {
    if ( m_hEvent != NULL )
      ::CloseHandle(m_hEvent);
    Duplicate(&m_hEvent, src.m_hEvent);
    return *this;
  }

  BOOL Create(BOOL bManualReset = FALSE, BOOL bInitialState = FALSE, LPSECURITY_ATTRIBUTES lpEventAttributes = NULL, LPCTSTR lpName = NULL)
  {
    m_hEvent = ::CreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName);
    return m_hEvent != NULL;
  }

  BOOL Set()
  {
    return ::SetEvent(m_hEvent);
  }

  BOOL Reset()
  {
    return ::ResetEvent(m_hEvent);
  }

  operator HANDLE() const
  { 

	  
	  return m_hEvent;
  }

  HANDLE* operator &()
  { 
    return &m_hEvent;
  }


  ~CEvent()
  {
    if ( m_hEvent != NULL )
    {
      CloseHandle(m_hEvent);
      m_hEvent = NULL;
    }
  }


private:
  BOOL Duplicate(HANDLE * phDest, HANDLE hSource)
  {
    if (hSource == NULL ) {
      
      *phDest = hSource; 
      
      return TRUE;
      
    } else {
      
      return ::DuplicateHandle(
        GetCurrentProcess(), hSource,
        GetCurrentProcess(), phDest,
        0,                        
        FALSE,                    
        DUPLICATE_SAME_ACCESS);
    }
  }
}; // CEvent

/// Wrapper for Win32 CriticalSection
class CCriticalSection
{
  /// the Win32 critical section
  CRITICAL_SECTION m_CS;
public:
  /// constructor
  CCriticalSection()
  {
    ::InitializeCriticalSection( &m_CS );
  }
  /// destructor
  ~CCriticalSection()
  {
    ::DeleteCriticalSection( &m_CS );
  }

private:
  CCriticalSection( const CCriticalSection & );           // not implemented
  CCriticalSection& operator=( const CCriticalSection& ); // not implemented
public:
  /// wait for ownership
  void Lock()
  {
    ::EnterCriticalSection( &m_CS );
  }
  /// release ownership
  void Unlock()
  {
    ::LeaveCriticalSection( &m_CS );
  }
  /// attempt to gain ownership - true if entering
  bool TryLock()
  {
#if _WIN32_WINNT >= 0x0400
    return ::TryEnterCriticalSection( &m_CS )!= 0;
#else
    assert( FALSE && _T( "TryLock not implemented" ) );
    return false;
#endif
  }

};

/*
* \brief Automatic locking of block statements 
* 
* class T requirements are the functions void Lock() and void Unlock().
*/
template<class T> 
class CAutoLock
{
  T& m_LockingObject;
public:
  CAutoLock( T& obj )
    : m_LockingObject( obj )
  {
    m_LockingObject.Lock();
  }
  ~CAutoLock()
  {
    m_LockingObject.Unlock();
  }
};

} // Bcam

#endif // defined BCAMUTILITY_H