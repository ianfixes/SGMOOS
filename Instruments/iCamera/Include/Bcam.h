//-----------------------------------------------------------------------------
//  Company:  Basler Vision Technologies
//  Section:  Vision Components
//  Project:  1394 Driver
//  <b> Visual SourceSafe Information:</b>
//  $Archive: /Software/inc/Bcam.h $
//  $Author: moosauthor $
//  $Revision: 1.1.2.1 $
//  $Date: 2004/07/27 19:55:59 $
//-----------------------------------------------------------------------------
/**
  \file     Bcam.h

  \brief   User C++ API for the BCAM 1394 driver

  This interface needs the following classes defined:
    - CString
    - CPoint
    - CSize
  Use either WTL or MFC to supply these classes.

  
  \todo   Things missing in this interface
          - Maybe somemore async functions
          - Ask camera, if OnePush is still active

          Things to implement in the camera
          - Memory_Channel
          - Vmode_Error_Status
          - Feature_Control_Error_Status
          - White_Balance
          - Format7 : mode == binning, ColorCode == Color

 */
//-----------------------------------------------------------------------------

#pragma warning (disable: 4005) // 'DEFINE_GUID' : macro redefinition


#ifndef BCAM_H
#define BCAM_H

#if (WINVER < 0x40A)
   #error Use of Bcam API requires WINVER >= 0x40A
#endif

#if(WINVER<0x0500)
  typedef PVOID HDEVNOTIFY;
  typedef HDEVNOTIFY *PHDEVNOTIFY;
  #define DEVICE_NOTIFY_WINDOW_HANDLE     0x00000000
  #define ERROR_DEVICE_REMOVED 1617L 

WINUSERAPI
HDEVNOTIFY
WINAPI
RegisterDeviceNotificationA(
    IN HANDLE hRecipient,
    IN LPVOID NotificationFilter,
    IN DWORD Flags
    );
WINUSERAPI
HDEVNOTIFY
WINAPI
RegisterDeviceNotificationW(
    IN HANDLE hRecipient,
    IN LPVOID NotificationFilter,
    IN DWORD Flags
    );
#ifdef UNICODE
#define RegisterDeviceNotification  RegisterDeviceNotificationW
#else
#define RegisterDeviceNotification  RegisterDeviceNotificationA
#endif // !UNICODE

WINUSERAPI
BOOL
WINAPI
UnregisterDeviceNotification(
    IN HDEVNOTIFY Handle
);

#endif

// #pragma warning (disable: 4996) // 4996: ... was declared deprecated
// #pragma warning (disable: 4702) // unreachable code

#if defined( USE_MFC )
#include <afxwin.h>         // MFC core and standard components
#else
#include <windows.h>
#endif

#pragma warning( push, 3 )
#include <dbt.h>
#include <list>
#include <map>
#pragma warning( pop )

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

#if _MSC_VER >= 1300
#include <atltypes.h>  // CSize, CPoint, ...
#endif

#include "functions.h"
#include "BcamError.h"
#include "BcamException.h"
#include "BcamPropertyBag.h"
#include <assert.h>

#ifndef TRACE
/// In case we are using WTL replace TRACE calls by AtlTrace
#define TRACE AtlTrace
#endif

/// the bcam namespace
namespace Bcam
{
  using namespace Bvc;

  /*-------------------------*/

  struct BcamOL;
  
  /// Function codes of all functions which can be called asynchronously
  enum  FunctionCode_t
  {
    NotifyUser            = -3,     ///< a user defined notification, no function
    NotifyQuit            = -2,     ///< notification to quit processing, no function
    NotifyBreak           = -1,     ///< dito
    AsyncShutterRawSet    = 0,      ///< function Bcam.Shutter.Raw.SetAsync was completed
    AsyncShutterAbsSet,             ///< function Bcam.Shutter.Abs.SetAsync was completed
    AsyncGainRawSet,                ///< function Bcam.Gain.Raw.SetAsync was completed
    AsyncGainAbsSet,                ///< function Bcam.Gain.Abs.SetAsync was completed
    AsyncBrightnessRawSet,          ///< function Bcam.Brightness.Raw.SetAsync was completed
    AsyncBrightnessAbsSet,          ///< function Bcam.Brightness.Abs.SetAsync was completed
    AsyncHueRawSet,                 ///< function Bcam.Hue.Raw.SetAsync was completed
    AsyncHueAbsSet,                 ///< function Bcam.Hue.Abs.SetAsync was completed  
    AsyncGammaRawSet,               ///< function Bcam.Gamma.Raw.SetAsync was completed
    AsyncGammaAbsSet,               ///< function Bcam.Gamma.Abs.SetAsync was completed
    AsyncSaturationRawSet,          ///< function Bcam.Saturation.Raw.SetAsync was completed
    AsyncSaturationAbsSet,          ///< function Bcam.Saturation.Abs.SetAsync was completed
    AsyncTriggerRawSet,             ///< function CBcam::CTrigger::CRawRegister::SetAsync was completed
    AsyncTriggerAbsSet,             ///< function CBcam::CTrigger::CAbsRegister::SetAsync was completed
    AsyncWhiteBalanceRawUBSet,      ///< function Bcam.WhiteBalance.Raw.UBValue.SetAsync was completed
    AsyncWhiteBalanceRawVRSet,      ///< function Bcam.WhiteBalance.Raw.VRValue.SetAsync was completed
    AsyncWhiteBalanceAbsUBSet,      ///< function Bcam.WhiteBalance.Abs.UBValue.SetAsync was completed
    AsyncWhiteBalanceAbsVRSet,      ///< function Bcam.WhiteBalance.Abs.VRValue.SetAsync was completed
    AsyncPositionSet,               ///< function CSetPosition::SetAsync was completed
    AsyncSizeSet,                   ///< function CSetSize::SetAsync was completed
    AsyncBytePerPacketSet,          ///< function CBytePerPacket::SetAsync was completed
    AsyncGrabImage,                 ///< An image was grabbed
    AsyncOneShot,                   ///< function CBcam::COneshot::SetAsync was completed
    AsyncContinuousShot             ///< function CBcem::CContiousShot::SetAsync was completed
  };
  
  /// Different types of IsSupported inquiries
  enum Supported_t
  {
    inqPresent,
    inqAbsControl,
    inqOnePush,
    inqReadOut,
    inqOnOff,
    inqAuto,
    inqManual,
    inqPolarity,
    inqTestImage_1 = 1,
    inqTestImage_2 = 2,
    inqTestImage_3 = 3,
    inqTestImage_4 = 4,
    inqTestImage_5 = 5,
    inqTestImage_6 = 6,
    inqTestImage_7 = 7,
    inqShadingCorrection,     
    inqSCM_1_3 = 1,            // shading correction with factor 1:3 supported
    inqSCM_1_5 = 2             // shading correction with factor 1:5 supported
  } ;

  /// Trigger modes defined by DCAM
  enum DCSTriggerMode
  {
    DCS_TriggerMode0 = 0,
    DCS_TriggerMode1,
    DCS_TriggerMode2,
    DCS_TriggerMode3
  };

  /// Shading correction modes
  enum SCMode_t
  {
    SCM_Test     = -1,   // the camera sends the shading correction table instead of real image data
    SCM_Disabled = 0,
    SCM_1_3      = 1,    // shading correction with factor 1:3
    SCM_1_5      = 2     // shading correction with factor 1:5
  }; 

  
  /// Trigger polarity
  enum Polarity_t
  {
    LowActive = 0,
    HighActive = 1,
  };
  
  /// Timespan in msec
  typedef ULONG Timespan;
 
  /// Video formats as defided by DCAM
  enum DCSVideoFormat
  {
    DCS_Format0 = 0,
    DCS_Format1 = 1,
    DCS_Format2 = 2,
    DCS_Format6 = 6,
    DCS_Format7 = 7
  };

  /// Modes of a video format as defined by DCAM
  enum DCSVideoMode
  {
    DCS_IgnoreVideoMode = -1,
    DCS_Mode0 = 0,
    DCS_Mode1 = 1,
    DCS_Mode2 = 2,
    DCS_Mode3 = 3,
    DCS_Mode4 = 4,
    DCS_Mode5 = 5,
    DCS_Mode6 = 6,
    DCS_Mode7 = 7,
  };

  /// Fixed frame rates as defined by DCAM
  enum DCSVideoFrameRate
  {
    DCS_IgnoreFrameRate = -1,
    DCS_1_875fps = 0,
    DCS_3_75fps,
    DCS_7_5fps,
    DCS_15fps,
    DCS_30fps,
    DCS_60fps,
    DCS_120fps,
    DCS_240fps
  };

  /// Values used by switches
  enum OnOff_t
  {
    Off = 0,
    On
  };

 /**
 * \define Used by nested classes to get access to the outer class
 */
#define IMPLEMENT_THIS(OUTER, MEMBER_VARIABLE) \
  OUTER* This() \
    { \
  return (OUTER*)((BYTE*)this - offsetof(OUTER, MEMBER_VARIABLE)); \
}; 



/**------------------------------------------------------------------
 * class:       BcamUtility
 *
 * \brief     Helper class which offers the user some convenience 
 *
 */
//--------------------------------------------------------------------
  class BcamUtility
  {
  public:
    /// Table 1.2.2
    static CString VideoResolutionName(DCSVideoFormat VideoFormat, DCSVideoMode VideoMode);
  
    /// Table 1.2.3
    static CString VideoFrameRateName(DCSVideoFrameRate VideoFrameRate);

    /// Mapping color ID to name (according to 2.2.2)
    static CString ColorCodeName(DCSColorCode ColorCode);
    
    /// Mapping BcamFeatureID to feature name
    static CString FeatureName(BcamFeatureID id);

    /// Mapping (video format, video mode ) to number of bytes per packet
    static unsigned long BytePerPacket(DCSVideoFormat VideoFormat, DCSVideoMode VideoMode, DCSVideoFrameRate VideoFramerate);
  
    /// Mapping (video format, video mode ) to ImageSize (for formats 0 - 2 only)
    static CSize ImageSize(DCSVideoFormat VideoFormat, DCSVideoMode VideoMode);

    /// Mapping (video format, video mode ) to color code (for formats 0 - 2 only)
    static DCSColorCode ColorCode(DCSVideoFormat VideoFormat, DCSVideoMode VideoMode);

    /// Mapping DCSColorCode to number of bits per pixel
    static unsigned short BitsPerPixel(DCSColorCode ColorCode);

    /// is a given device name a "full" device name, i.e. does it contain the driver interface GUID
    static bool IsFullDeviceName(CString deviceName);

    /// returns the full device name
    static CString FullDeviceName(CString deviceName);

    /// returns the friendly device name (i.e. not containng the driver interface GUID)
    static CString FriendlyDeviceName(CString deviceName);


  };



//------------------------------------------------------------------------------
// class CBcam
// Author: Hartmut Nebelung
// Date: 
//------------------------------------------------------------------------------
/**
 * \brief Encapsulation of the Bcam driver API
 *
 * 
 * \todo   Multishot
 */
//------------------------------------------------------------------------------
class CBcam
{

  typedef std::map<HANDLE, CBcam*> BcamMap_t ;
  
public:
  /// Callback function type
  typedef void (*BcamCallbackFunc) (CBcam&, void*);

  CBcam();

private: // copying prohibited
  CBcam( const CBcam& );
  CBcam& operator=( const CBcam& );

public:
  virtual ~CBcam();

  // List all cameras currently found in the system
  static std::list<CString>  DeviceNames();
  static std::list<CString>& DeviceNames( std::list<CString>& );

  // Register a client window for device notifications
  static void RegisterClient(HWND hwnd);
  // Unregister a client from device notifications
  static void UnregisterClient();
  // CleanUp code.
  static void CleanUp();
  // Get the version of the installed driver
  static DWORD InstalledDriverVersion();

  // Handler for device notifications
  static LRESULT OnDeviceChange(WPARAM wParam, LPARAM lParam);

  // Set a callback function which will be called if a device is to be removed
  void SetOnRemoveRequestCallback( BcamCallbackFunc, void* pv = NULL );

  // Set a callback function which will be called if a device has been removed
  void SetOnRemoveCompleteCallback( BcamCallbackFunc, void* pv = NULL );

  // Is the driver open
  bool IsOpen();

  // Opens the driver
  void Open( CString DeviceName, HWND hWndNotify = NULL );

  bool Authorize(const CString& Name,const CString& Id);

  // Closes the driver and frees all resources (esp. OVERLAPPED structures from cancelled driver calls)
  void Close();

  // Initialize camera
  void Initialize(); 

  // Query whether the video mode is supported
  bool IsVideoModeSupported( DCSVideoFormat VideoFormat, DCSVideoMode VideoMode = DCS_IgnoreVideoMode, DCSVideoFrameRate VideoFramerate = DCS_IgnoreFrameRate );
  // Query the current video mode settings
  void GetVideoMode( DCSVideoFormat *VideoFormat, DCSVideoMode *VideoMode, DCSVideoFrameRate *VideoFramerate );
  // Set new video mode
  void SetVideoMode( DCSVideoFormat VideoFormat, DCSVideoMode VideoMode, DCSVideoFrameRate VideoFramerate = DCS_IgnoreFrameRate );

  // Save current settings to a file
  void SaveSettingsToFile(const CString& FileName);
  // Save current settings to the registry
  void SaveSettingsToRegistry(const CString& KeyName);
  // save the current settings to a PropertyBag
  void SaveSettings(const CPropertyBagPtr ptrBag);

  // Restore settings from a file
  void RestoreSettingsFromFile(const CString& FileName);
  // Restore settings from the registry
  void RestoreSettingsFromRegistry(const CString& KeyName);
  // Restore settings from a property bag
  void RestoreSettings(const CPropertyBagPtr ptrBag);
  // Set the thread priority of the current thread
  void SetCurrentThreadPriority(unsigned long priority);
  // Set the priority of the driver's queue server thread
  unsigned long SetQueueServerPriority(unsigned long priority);
  // Get the priority of the driver's queue server thread
  unsigned long GetQueueServerPriority();


  /// Inquire, get and set all scalar properties.
  class CScalarProperty
  {
    const BcamFeatureID m_FunctionId; ///< Identifies the feature
    CBcam * const m_pBcam;            ///< Backpointer for data exchange
    /// Different modes for CSR 
    enum Mode_t
    {
      mOnOff,
      mAutoMode,
      mAbsControl
    };

  public:

    CScalarProperty( FunctionCode_t AsyncFuncCodeRaw, FunctionCode_t AsyncFuncCodeAbs, 
      BcamFeatureID FuncId, CBcam* pBcam );

    bool IsSupported(Supported_t Inquiry = inqPresent);

    /// Get and set boolean controls
    class CBoolControl
    {
      CBcam * const m_pBcam;
      const BcamFeatureID m_FunctionId;
      Mode_t m_Mode;

    public:
      CBoolControl( BcamFeatureID FuncId, Mode_t Mode, CBcam* pBcam ); 

      void operator=(bool Value);
      bool operator()();    
    };  // CBoolControl 

    /// Set feature with absolute value
    CBoolControl AbsControl;

    /// Control feature automatically
    CBoolControl AutoMode;

    /// Switch feature on/off
    CBoolControl OnOff;

    /// Control feauture automatically once
    void OnePush();

    /// Access property via raw register
    class CRawRegister
    {
    private:
      CBcam * const m_pBcam;                ///< Backpointer for data exchange
      const BcamFeatureID m_FunctionId;     ///< Feature identifier
      const FunctionCode_t m_FunctionCode;  ///< Function code used for asynchronous calls
    public:
      CRawRegister( FunctionCode_t FuncCode, BcamFeatureID FuncId, CBcam* pBcam );

      unsigned long Min();
      unsigned long Max();
      void operator=(unsigned long Value);
      unsigned long operator()();   
      void SetAsync(unsigned long Value, void *pContext = NULL);
    } Raw; ///< Raw register of the feature

    /// Access property via absolute register
    class CAbsValue
    {
    private:
      const CBcam* m_pBcam;                ///< Backpointer to the camera object
      const BcamFeatureID m_FunctionId;    ///< Feature identifier
      const FunctionCode_t m_FunctionCode; ///< Function code for asynchronous calls
    public:
      CAbsValue( FunctionCode_t FuncCode, BcamFeatureID FuncId, CBcam* pBcam ); 

      double Min();
      double Max();
      void operator=(double Value);
      double operator()();    
      void SetAsync(double Value, void *pContext = NULL);
    } Abs; ///< Absolute register of a feature

    protected:
      void Save(const CPropertyBagPtr ptrBag);
      void Restore(const CPropertyBagPtr ptrBag);
      friend class CBcam;
 
    friend class CBoolControl;

  };  // CScalarProperty
  friend class CScalarProperty;
  friend class CScalarProperty::CBoolControl;
  friend class CScalarProperty::CRawRegister;

  CScalarProperty Shutter;          ///< Shutter feature
  CScalarProperty Gain;             ///< Gain feature
  CScalarProperty Brightness;       ///< Brightness (offset) feature
  CScalarProperty Hue;              ///< Hue feature
  CScalarProperty Saturation;       ///< Saturation feature
  CScalarProperty Gamma;            ///< Gamma correction feature


  /// Inquire, get and set properties of whitebalance feature
  class CWhiteBalance
  {
    IMPLEMENT_THIS(CBcam, WhiteBalance);

   

    /// Different modes for CSR 
    enum Mode_t
    {
      mOnOff,
      mAutoMode,
      mAbsControl
    };

    enum Value_t
    {
      vUB,    // U / B value
      vVR     // V / R value
    };

  public:

    CWhiteBalance();

    bool IsSupported(Supported_t Inquiry = inqPresent);

    /// Get and set boolean controls
    class CBoolControl
    {
      CBcam * const m_pBcam;
      Mode_t m_Mode;

    public:
      CBoolControl(Mode_t Mode, CBcam* pBcam ); 

      void operator=(bool Value);
      bool operator()();    
    };  // CBoolControl 

    /// Set feature with absolute value
    CBoolControl AbsControl;

    /// Control feature automatically
    CBoolControl AutoMode;

    /// Switch feature on/off
    CBoolControl OnOff;

    /// Control feauture automatically once
    void OnePush();

    /// Access property via raw register
    class CRawRegister
    {
      IMPLEMENT_THIS(CWhiteBalance, Raw);

    public:

      CRawRegister();

      unsigned long Min();
      unsigned long Max();

      /// Access whitebalance property via raw register
      class CWhiteBalanceRawValue
      {
        const Value_t m_vt;                   ///< Do we represent U/B or V/R value?
        CBcam * const m_pBcam;                ///< Backpointer for data exchange
      public: 
        CWhiteBalanceRawValue(Value_t vt, CBcam* pBcam);

        void operator=(unsigned long Value);
        unsigned long operator()();   
        void SetAsync(unsigned long Value, void *pContext = NULL);
      };
      CWhiteBalanceRawValue UBValue; ///< U or B value
      CWhiteBalanceRawValue VRValue; ///< V or R value
    } Raw; ///< Raw register value control

    /// Access property via absolute register
    class CAbsValue
    {
      IMPLEMENT_THIS(CWhiteBalance, Abs);
    public:

      CAbsValue();

      double Min();
      double Max();

      /// Access whitebalance property via absolute register
      class CWhiteBalanceAbsValue
      {
        const Value_t m_vt;                   ///< Do we represent U/B or V/R value?
        CBcam * const m_pBcam;                ///< Backpointer for data exchange
      public: 
        CWhiteBalanceAbsValue(Value_t vt, CBcam* pBcam);

        void operator=(double Value);
        double operator()();   
        void SetAsync(double Value, void *pContext = NULL);
      };
      CWhiteBalanceAbsValue UBValue;          ///< U or B value
      CWhiteBalanceAbsValue VRValue;          ///< V or R value
    } Abs;  ///< Absolute value control

    protected:
      void Save(const CPropertyBagPtr ptrBag);
      void Restore(const CPropertyBagPtr ptrBag);

    friend class CBcam;
    friend class CBoolControl;
    friend class CRawRegister;
    friend class CRawRegister::CWhiteBalanceRawValue;
    friend class CAbsValue;
    friend class CAbsValue::CWhiteBalanceAbsValue;

  } WhiteBalance; ///< WhiteBalance feature 

  friend class CWhiteBalance;
  friend class CWhiteBalance::CBoolControl;
  friend class CWhiteBalance::CRawRegister;
  friend class CWhiteBalance::CRawRegister::CWhiteBalanceRawValue;



  /// Inquire, get and set the test image feature from advanced feature set.
  class CTestImage
  {
    IMPLEMENT_THIS(CBcam, TestImage);
  public:
    
    bool IsSupported(Supported_t Inquiry = inqPresent);
    void operator=(ImageOnMode Value); 
    ImageOnMode operator()();  
  protected:
    void Save(const CPropertyBagPtr ptrBag);
    void Restore(const CPropertyBagPtr ptrBag);
    friend class CBcam;

  } TestImage; ///< Test image feature
  friend class CTestImage;

  /// Pointer to a callback function, called during image upload
  typedef bool (*BcamProgressCallbackFunc) (int, void*);

  /// Inquire, get and set the shading correction feature from advanced feature set
  class CShadingCorrection
  {
    IMPLEMENT_THIS(CBcam, ShadingCorrection);
  public:
    bool IsSupported( Supported_t Inquiry = inqPresent);
    void SetImage( void* pBuffer, size_t BufferSize, Timespan timeout=INFINITE, 
                   BcamProgressCallbackFunc pCallback = NULL , int BlockSize = 0,  void* pContext = NULL ); 
    SCMode_t operator()();
    void operator=(SCMode_t Value);
  } ShadingCorrection; ///< Shading correction feature
  
  friend class CShadingCorrection;

  /// Inquire, get and set the external trigger feature
  class CTrigger
  {
    IMPLEMENT_THIS(CBcam, Trigger); ///< This() retruns a pointer to outer class

  public:
    CTrigger( FunctionCode_t AsyncFuncCodeRaw, FunctionCode_t AsyncFuncCodeAbs, CBcam* pBcam );

    bool IsSupported(Supported_t Inquiry = inqPresent);
    bool IsModeSupported(DCSTriggerMode TriggerMode);

    ///Get and set boolean controls
    class CBoolControl
    {
      virtual CTrigger * This() = 0;

    public:
      
      void operator=(bool Value);
      bool operator()();    
    protected:
      /// Mode of a switch
      enum Mode_t
      {
        mOnOff,      ///< specifies the OnOff control of the feature
        mAbsControl, ///< specifies the absolute control of the feature
      } m_Mode; ///< mode used by this instance


      friend class CBoolControlAbs;
      friend class CBoolControlOnOff;
    };  // CBoolControl 

    friend class CBoolControl;
    /// Get and set absolute control feature
    class CBoolControlAbs: public CBoolControl
    {
      IMPLEMENT_THIS(CTrigger, AbsControl); ///< This() retruns a pointer to outer class
    public:
      /// Constructor
      CBoolControlAbs(){m_Mode = mAbsControl;};
      /// Set the new value
      void operator=(bool Value) { CBoolControl::operator=(Value); }
    } AbsControl; ///< Absolute control feature

    /// Get and set absolute control feature
    class CBoolControlOnOff: public CBoolControl
    {
      IMPLEMENT_THIS(CTrigger, OnOff); ///< This() retruns a pointer to outer class
    public:
      /// Constructor
      CBoolControlOnOff(){m_Mode = mOnOff;};
      /// Set the new Value
      void operator=(bool Value) { CBoolControl::operator=(Value); }
    } OnOff; ///< On/Off feature

    /// Get and set trigger polarity
    class CPolarity
    {
      IMPLEMENT_THIS( CTrigger, Polarity ); ///< This() returns pointer to outer class
    public:
      /// Set polarity
      void operator=(Polarity_t Value);
      /// Get polarity
      Polarity_t operator()();    
    } Polarity; ///< Trigger polarity feature
    friend class CPolarity;

 
    /// Get and set mode
    class CMode
    {
    IMPLEMENT_THIS(CTrigger, Mode); ///< This() retruns a pointer to outer class
    public:
      void operator=(DCSTriggerMode Value);
      DCSTriggerMode operator()();   
    } Mode; ///< Mode feature
    friend class CMode;


    /// Get and set the trigger parameter
    class CParameter
    {
    IMPLEMENT_THIS(CTrigger, Parameter); ///< This() retruns a pointer to outer class
    public:
      void operator=(unsigned long Value);
      unsigned long operator()();   
    } Parameter; ///< Trigger parameter feature
    friend class CParameter;

    friend class CBoolControl;

    friend class CBcam;

    protected:
    void Save(const CPropertyBagPtr ptrBag);
    void Restore(const CPropertyBagPtr ptrBag);


  } Trigger; ///< Trigger feature
  friend class CTrigger; 
  friend class CTrigger::CMode;
  friend class CTrigger::CParameter;
  friend class CTrigger::CBoolControl;
  friend class CTrigger::CPolarity;

  /// Inquire and set OneShot
  class COneShot
  {
    IMPLEMENT_THIS(CBcam, OneShot); ///< This() returns a pointer to outer class
  public:
    bool IsSupported();
    void Set();
    void SetAsync(void *pContext = NULL);
  } OneShot; ///< One shot feature
  friend class COneShot; 

  /// Inquire, set and reset ContinuousShot
  class CContinuousShot
  {
    IMPLEMENT_THIS(CBcam, ContinuousShot); ///< This() returns a pointer to the outer class
  public:
    bool IsSupported();
    void operator=(bool Value);
    bool operator()();
    void SetAsync(bool Value, void *pContext = NULL );
  } ContinuousShot; ///< Continous shot (ISO_EN) feature
  friend class CContinuousShot;

  // Inquire, set and reset MultiShot
  /*
  class CMultiShot
  {
  public:
    bool IsSupported();
    void operator=(bool Value);
    bool operator()();
    class CCountNumber
    {
    public:
      void operator=(unsigned long Value);
      unsigned long operator()();
    } CountNumber;
  } MultiShot;
  */
  // Allocate isochronous channel, resource handle and bandwidth
  ULONG AllocateResources(unsigned long MaxBuffers, unsigned long MaxBufferSize, unsigned long BytePerPacket = 0);
  // Release isochronous channel, resource handle and bandwidth
  void FreeResources();
  // Grab image with or without asserting OneShot
  void GrabImage( void *pBuffer, size_t BufferSize, Timespan Timeout = INFINITE, bool OneShot = true );
  // Grab image asynchronously
  void GrabImageAsync( void *pBuffer, size_t BufferSize, void *pContext = NULL, bool OneShot = true);
  // Grab n buffers (Multishot)
  void GrabImages( void *pBuffer[], size_t numBuffers, size_t BufferSize, Timespan Timeout = INFINITE );


  /// Inquire, get and set Format7 parameters
  class CFormatSeven 
  {
    IMPLEMENT_THIS( CBcam, FormatSeven ); ///< This() returns pointer to outer class
  public:
    /// Inquire, get and set a specific Format7 mode
    class _FormatSeven
    {
      const size_t m_Mode; ///< video mode of format 7
    public:
      CSize MaxSize();
      /// Inquire, get and set the image position
      class CImagePosition
      {
        IMPLEMENT_THIS( _FormatSeven, Position ); ///< This() returns pointer to outer class
      public:
        void operator=(CPoint Value);
        CPoint operator()();
        CSize Inc();
        void SetAsync(CPoint Value, void *pContext = NULL);
      } Position; ///< Image position of mode
      friend class CImagePosition;
      /// Inquire, get and set the image size
      class CImageSize
      {
        IMPLEMENT_THIS( _FormatSeven, Size ); ///< This() returns pointer to outer class
      public:
        void operator=(CSize Value);
        CSize operator()();
        CSize Inc();
        void SetAsync(CSize Value, void *pContext = NULL);
      } Size; ///< Image size of mode
      friend class CImageSize;
      /// Inquire, get and set the image type
      class CColorCoding
      {
        IMPLEMENT_THIS( _FormatSeven, ColorCoding ); ///< This() returns pointer to outer class
      public:
        bool IsSupported(DCSColorCode ColorCode);
        void operator=(DCSColorCode Value);
        DCSColorCode operator()();
      } ColorCoding; ///< image type of mode
      friend class CColorCoding;

      unsigned long PixelPerFrame();
      unsigned long BytePerFrame();
      unsigned long PacketsPerFrame();
      /// Inquire, get and set the packet parameters
      class CBytePerPacket
      {
        IMPLEMENT_THIS(_FormatSeven, BytePerPacket); ///< This() returns pointer to outer class.
      public:
        unsigned long Inc();
        unsigned long Max();
        unsigned long Rec();
        void operator=(unsigned long Value);
        unsigned long operator()();   
        void SetAsync(unsigned long Value, void *pContext = NULL);
      } BytePerPacket; ///< Packet parameters of mode
      friend class CBytePerPacket;
      
      
      const size_t baseoffset; ///< offset of first video mode
      /// Access to outer class
      CFormatSeven* This(){ return (CFormatSeven*)((BYTE*)this - baseoffset); }
      /// Constructor
      _FormatSeven( size_t offset,size_t mode ):baseoffset(offset), m_Mode(mode){}

      protected:
        void Save(const CPropertyBagPtr ptrBag);
        void Restore(const CPropertyBagPtr ptrBag);

      friend class CBcam;
    };
    friend class _FormatSeven;
    friend class _FormatSeven::CImagePosition;
    friend class _FormatSeven::CImageSize;
    friend class _FormatSeven::CColorCoding;
    friend class _FormatSeven::CBytePerPacket;
    _FormatSeven Format7_0; ///< Mode 0 of format 7
    _FormatSeven Format7_1; ///< Mode 1 of format 7
    _FormatSeven Format7_2; ///< Mode 2 of format 7
    _FormatSeven Format7_3; ///< Mode 3 of format 7
    _FormatSeven Format7_4; ///< Mode 4 of format 7
    _FormatSeven Format7_5; ///< Mode 5 of format 7
    _FormatSeven Format7_6; ///< Mode 6 of format 7
    _FormatSeven Format7_7; ///< Mode 7 of format 7
    /// Access to the modes of format 7
    _FormatSeven& operator[]( int i ){
      switch (i)
      {
      case 0: return Format7_0;
      case 1: return Format7_1;
      case 2: return Format7_2;
      case 3: return Format7_3;
      case 4: return Format7_4;
      case 5: return Format7_5;
      case 6: return Format7_6;
      case 7: return Format7_7;
      default:
        throw BcamException( BCAM_E_UNSUPPORTED_VIDEO_MODE, _T( "Mode not defined" ), &i );
      }
    }
    /// Constructor
    CFormatSeven() : 
      Format7_0( offsetof( CFormatSeven, Format7_0 ), 0 ),
      Format7_1( offsetof( CFormatSeven, Format7_1 ), 1 ),
      Format7_2( offsetof( CFormatSeven, Format7_2 ), 2 ),
      Format7_3( offsetof( CFormatSeven, Format7_3 ), 3 ),
      Format7_4( offsetof( CFormatSeven, Format7_4 ), 4 ),
      Format7_5( offsetof( CFormatSeven, Format7_5 ), 5 ),
      Format7_6( offsetof( CFormatSeven, Format7_6 ), 6 ),
      Format7_7( offsetof( CFormatSeven, Format7_7 ), 7 )
    {
    }
  } FormatSeven; ///< Format 7 feature 
  friend class CFormatSeven;
  friend class CFormatSeven::_FormatSeven;
  friend class CFormatSeven::_FormatSeven::CImagePosition;
  friend class CFormatSeven::_FormatSeven::CImageSize;
  friend class CFormatSeven::_FormatSeven::CBytePerPacket;
  friend class CFormatSeven::_FormatSeven::CColorCoding;

  // Wait for completion of asynchronous function calls
  void WaitForCompletion(FunctionCode_t *pFunktionCode, unsigned long *pErrorCode, 
    void**ppContext = NULL, Timespan Timeout = INFINITE);
  // Post an I/O completion packet to the completion port 
  void Notify(FunctionCode_t FunctionCode, void* pContext = NULL);
  
  // Cancel all function calls 
  void Cancel();

  /// Supply information about the camera and the driver
  class CInfo
  {
    IMPLEMENT_THIS( CBcam, Info ); ///< This() returns pointer to outer class.
  public:

    CString DeviceName();
    CString NodeId();
    CString ModelName();
    CString VendorName();
    CString CameraFirmwareVersion();
    unsigned long CameraDcamVersion();
    CString DriverSoftwareVersion();
    
    // For information purposes only. This value is set by the driver
    long IsoChannel();
    
    // For information purposes only. This value is set by the driver
    BcamIsoSpeed IsoSpeed();

  } Info; ///< Camera information object
  friend class CInfo;


  /// get and set feature
  class CBoolControl
  {
  public:
    void operator=(bool Value);
    bool operator()();    
  } ErrorChecking, ///< Error checking feature of the driver
    ValueCaching;  ///< Value caching feature of the driver


protected:

  // close the file handle
  void CloseFileHandle();

  // create completion port
  void CreateCompletionPort();

  // send command to camera synchronously 
  bool Transmit( DWORD IoCtl, void *pArg, size_t argSize, void *pRes=NULL, size_t resSize=0, Timespan timeout = INFINITE );
  // send command to camera asynchronously 
  bool Transmit( DWORD IoCtl, void *pArg, size_t argSize , BcamOL* pOL );
  // send command to and retrieve information from camera asynchronously
  bool Transmit( DWORD IoCtl, void *pArg, size_t argSize, void* pRes, size_t resSize, BcamOL* pOL );
  // try to send command to camera synchronously, throw on failure 
  void TryTransmit( LPCTSTR context, 
    DWORD IoCtl, void *pArg, size_t argSize, void *pRes = NULL, size_t resSize = 0,
    Timespan timeout= INFINITE  );

  // is the camera object valid
  bool IsValid();
  // Registers a client window for handle related device notifications
  HDEVNOTIFY RegisterDeviceNotifcation(HWND hWnd);
  // unregister client window from device notifications
  void UnregisterDeviceNotification(HDEVNOTIFY hNotify);
  // Unregister the client window from device interface related notifications
  static void InternalUnregisterClient();
  // reopens the camera driver
  void Reopen();
  // The thread procedure of the message loop thread
  static DWORD WINAPI MessageLoop(LPVOID lpThreadParameter);
  // When a windows gets registered for device notifications, it will be subclassed by this window procedure
  static LRESULT CALLBACK BcamWindowProc(HWND, UINT, WPARAM, LPARAM);
  // The window procedure of the hidden message-only window
  static LRESULT CALLBACK HiddenWindowProc(HWND, UINT, WPARAM, LPARAM);
  // Close the hidden message-only window
  static void CloseHiddenWindow();

protected:
  
  HANDLE            m_hCamera;             ///< handle to the opened device 
  CString           m_DeviceName;          ///< friendly devcie name of device we have opened
  HDEVNOTIFY        m_hDevHandleNotify;    ///< device notification handle for file handle related notivations
  HDEVNOTIFY        m_hDevHandleNotifyOpt; ///< device notification handle for an additional registered window
  HWND              m_hWndHandleNotify;    ///< window handle for which we register handle related notifications
  static HDEVNOTIFY s_hDevInterfaceNotify; ///< device notification handle for interface related notifications
  static BcamMap_t  s_BcamMap;             ///< map the open devices
  static HWND       s_hWndMsgOnly;         ///< message-only window used for device notifications instead of a user supplied window
  static HWND       s_hWndNotify;          ///< window handle for which we register device notifications
  static WNDPROC    s_lpPrevWndFunc;       ///< Pointer to the window function we subclass
  BcamCallbackFunc  m_OnRemoveRequestCallback; ///< pointer to OnRemoveRequest callback function
  BcamCallbackFunc  m_OnRemoveCompleteCallback;///< pointer to OnRemoveComplete callback function
  void*             m_pvOnRemoveRequest;   ///< void pointer which will passed to the OnRemoveRequest callback function 
  void*             m_pvOnRemoveComplete;  ///< void pointer which will passed to the OnRemoveComplete callback function 
  HANDLE            m_hCompletionPort;     ///< handle to the completion port
  const ULONG       m_CompletionKey;       ///< key used with the completion port
  static ULONG      s_CompletionKey;       ///< unique key for each instance
  long              m_IsoChannel;          ///< allocated isochronous channel. 
  BcamIsoSpeed      m_IsoSpeed;            ///< isochronous speed 

  /// Does some clean up
  static class CCleanUp {
  public:
     ~CCleanUp();
  } s_CleanUp;
  friend class CBcam::CCleanUp;
  
  
};  // CBcam

} // namespace Bcam


#endif // BCAM_H

