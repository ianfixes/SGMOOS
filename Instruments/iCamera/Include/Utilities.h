//-----------------------------------------------------------------------------
//  (c) 2002 by Basler Vision Technologies
//  Section:  Vision Components
//  Project:  BCAM
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/Utilities.h,v 1.1.2.1 2004/07/27 19:56:00 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
\file     Utilities.h
*
\brief   Interface and implementation of some handy helper classes
*/
//-----------------------------------------------------------------------------


#pragma once

#include <vector>
#include <cassert>
using namespace std;
#include <BcamUtility.h>

//! Measures time intervalls
class CStopWatch
{
public:
  //! constructor, starts time measurement
  CStopWatch()
  {
    Start();
  }
  
  //! Start the stop watch 
  void Start()
  {
    QueryPerformanceCounter(&m_StartTime);
  }
  
  //! Stops the stop watch
  /*! The elapsed time is returned. The stop watch may be started again 
  */
  double Stop(
    bool StartAgain = false  //! if true the stop watch is started again immediately
    )
  {
    QueryPerformanceCounter(&m_StopTime);
    double theElapsedTime = ElapsedTime();
    if(StartAgain)
      m_StartTime = m_StopTime; 
    return theElapsedTime;
  }
  
  //! Returns the elapsed time in seconds between start() and stop()
  double ElapsedTime()
  {
    LARGE_INTEGER timerFrequency;
    QueryPerformanceFrequency(&timerFrequency);
    
    __int64 oldTicks = ((__int64)m_StartTime.HighPart << 32) + (__int64)m_StartTime.LowPart;
    __int64 newTicks = ((__int64)m_StopTime.HighPart << 32) + (__int64)m_StopTime.LowPart;
    long double timeDifference = (long double) (newTicks - oldTicks);
    
    long double ticksPerSecond = (long double) (((__int64)timerFrequency.HighPart << 32) 
      + (__int64)timerFrequency.LowPart);
    
    return (double)(timeDifference / ticksPerSecond);
  }
  
protected:
  //! zero-point for time measurment
  LARGE_INTEGER m_StartTime;
  
  //! last time stamp
  LARGE_INTEGER m_StopTime;
};





//! Template class to calculate a moving average
/*! \param T Type of the measurement data (typically double or float)
\param span Number of samples used of computing the mean
The class is thread safe.
*/
template <class T, unsigned int span = 10>
class CMovingAvg
{
  
public:
  //! Default constructor
  CMovingAvg() : m_v(span) 
  {
    Reset();
  }
  
  //! Add a data item to the moving average
  void Add(T sample) 
  { 
    m_CritSect.Lock();
    int idx = m_n % span;
    if ( m_n < span )
    {
      m_v[idx] = sample;
      m_Sum += sample;
    }
    else
    {
      m_Sum -= m_v[idx];
      m_Sum += sample;
      m_v[idx] = sample;
    }
    m_n++;
    
    m_CritSect.Unlock();
  }
  
  //! Get the average
  double Avg() {
    double res = 0;
    m_CritSect.Lock();
    if ( m_n != 0 )
    {
      res = m_n < span ? m_Sum / (double) m_n : m_Sum / (double) span;
    }
    m_CritSect.Unlock();
    return res;
  }
  
  //! Reset the moving average clearing the history
  void Reset() {
    m_CritSect.Lock();
    m_Sum = 0.0;
    m_n = 0;
    m_CritSect.Unlock();
  }
  
protected:
  //! The current sum
  double m_Sum;
  
  //! Number of measurement data
  unsigned int m_n;
  
  //! Vetor holding #span data items
  std::vector<T> m_v;
  
  //! critical section guarding the access to the class
  Bcam::CCriticalSection m_CritSect;
  
};


/*! \def CATCH_MSGBOX( Caption ) 
  \brief Catches exceptions and reports them
  \param Caption Caption of the message box which might be shown. 
  Catches #BcamException and general exceptions and shows appropriate 
  message boxes.
  \par Usage
  \code 
  try
  {
  \\\\ come code...
  }
  CATCH_MSGBOX("CMyClass::MyFunction")
  \endcode
*/
#define CATCH_MSGBOX( Caption ) \
catch( BcamException &e ) \
{ \
  CString Buffer, B; \
  Buffer += (B.Format("Exception 0x%X occurred\n", e.Error() ), B); \
  Buffer += (B.Format("Message = %s\n", e.Description() ), B); \
  Buffer += (B.Format("Context = %s\n", e.Context()), B); \
  MessageBox(Buffer, _T(Caption), MB_OK | MB_ICONEXCLAMATION); \
} \
catch (...) \
{ \
  BcamException e(DISP_E_EXCEPTION, "CMainFrame::OnGrabLive"); \
  CString Buffer, B; \
  Buffer += (B.Format("Exception 0x%X occurred\n", e.Error() ), B); \
  Buffer += (B.Format("Message = %s\n", e.Description() ), B); \
  Buffer += (B.Format("Context = %s\n", e.Context()), B); \
  MessageBox(Buffer, _T(Caption), MB_OK | MB_ICONEXCLAMATION); \
}


