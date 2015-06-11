//-----------------------------------------------------------------------------
//  Company:  Basler Vision Technologies
//  Section:  Vision Components
//  Project:  1394 Driver
//  Subproject:  Bus Access Driver
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/BcamAdapter.h,v 1.1.2.1 2004/07/27 19:55:59 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
  \file     BcamAdapter.h
  \brief   interface to the Bus Access Driver
 *
 * Includes the classes CBcamAdapter, CNodePtr and CNode.
 *
 */
//-----------------------------------------------------------------------------

#if !defined(AFX_BCAMADAPTER_H__34513919_0AE1_4812_B5BB_B6ABFE062D7B__INCLUDED_)
#define AFX_BCAMADAPTER_H__34513919_0AE1_4812_B5BB_B6ABFE062D7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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


#if defined( USE_MFC )
#pragma message ("USE_MFC" )
#include <afxwin.h>         // MFC core and standard components
#endif
#if defined( USE_WTL )
#pragma message ("USE_WTL")
#include <windows.h>
#endif

#pragma warning( push, 3 )
#include <dbt.h>
#include <list>
#include <map>
#include <bitset>
#pragma warning( pop )

#if defined( USE_WTL )
#include <atlapp.h>
#include <atlmisc.h>        // WTL's CSIZE,...
#endif  

#include "bacc.h"
#include "BcamError.h"
#include <bcamutility.h>

struct BcamAdapterOL;

namespace Bcam
{

/// node types
enum NodeType
{
  NT_Bcam,    ///< Basler 1394 Camera
  NT_Dcam,    ///< DCAM compliant camera
  NT_Sbp2,    ///< Sbp2 compliant device
  NT_Pc,      ///< Microsoft PC
  NT_Unknown  ///< not identified node
};

/// physical id
typedef ULONG ID;

/// tranfer rates
enum TransferRate
{
  TR_100,   ///< 100 mbit/s
  TR_200,   ///< 200 mbit/s
  TR_400    ///< 400 mbit/s
};


//------------------------------------------------------------------------------
// class CNode
// Author: HNebelun
// Date: 
//------------------------------------------------------------------------------
/**
 * \brief   Representation of a IEEE 1394 bus node
 *
 * Nodes are created by BcamAdapters only. Their lifetime is controlled by a
 * reference count. As soon as it drops to zero the CNode object destroys itself.
 * The nodes are part of a snapshot of the bus topology. The information contained
 * may be out of date after the next bus reset but is safe to use.
 * 
 * \see   CBcamAdapter::GetTree creates a node tree.
 * 
 * \todo  Get the NodeType
 */
//------------------------------------------------------------------------------
class CNode
{
  friend class CBcamAdapter;
private: // no public creation or copying
  CNode();
  CNode( const CNode& );
  ~CNode();
  CNode& operator=( const CNode& );
public:
  typedef ULARGE_INTEGER NUID;

  /// Returns the child node if present
  CNode* Child( ULONG port ) const;
  /// Gets the node unique ID
  NUID NodeID() const;
  /// Gets the vendor name
  CString VendorName() const;
  /// Gets the model name
  CString ModelName() const;
  /// Gets the number of ports
  ULONG NumPorts() const 
  {
    return m_NumPorts;
  }
  /// Gets the number of child nodes
  ULONG NumChilds() const;
  /// Gets the node type
  NodeType Type() const;
  /// Gets the physical ID
  ID PhysicalId() const;

  /// Gets the Port connected to the parent
  ULONG Parent() const;
public:
  /// Increments the reference count
  ULONG AddRef()
  {
    return InterlockedIncrement( (PLONG)&m_RefCount );
  }
  /// Decrements the reference count
  ULONG Release()
  { 
    const ULONG r = InterlockedDecrement( (PLONG)&m_RefCount );
    if (r == 0)
      delete this;
    return r;
  }
  enum { 
    /// Maximum number of ports
    MaxChilds = 3+3*6 
  };
private:
  /// Number of references
  mutable ULONG m_RefCount;
  /// Physical ID
  ID m_Id;
  /// Pointer to child nodes
  CNode *m_pChild[ MaxChilds ];
  /// Number of ports
  ULONG m_NumPorts;
  /// Vendor name
  CString m_VendorName;
  /// Model name
  CString m_ModelName;
  /// Node unique id
  NUID m_Nuid;
  /// Type of this node
  NodeType m_Type;
  /// Number of Port connect to Parent Node
  ULONG m_ParentPort;
};

//------------------------------------------------------------------------------
// class CNodePtr
// Author: HNebelun
// Date: 
//------------------------------------------------------------------------------
/**
 * \brief   Smart pointer to CNode object
 *
 * Manages a pointer to a CNode and automatically does the reference counting 
 */
//------------------------------------------------------------------------------
class CNodePtr
{
public:
  /// Default constructor
  CNodePtr()
    : m_pNode( NULL )
  {
  }

  /// Destructor
  ~CNodePtr()
  {
    InternalRelease();
  }

  /// Copy constructor
  CNodePtr( const CNodePtr& ptr )
    : m_pNode( NULL )
  {
    Attach( ptr.m_pNode, true );
  }

  /// Assignment operator - smart pointer
  CNodePtr& operator=( const CNodePtr& ptr )
  {
    if (ptr.m_pNode != m_pNode)
    {
      InternalRelease();
      Attach( ptr.m_pNode, true );
    }
    return *this;
  }

  /// Wrap up a node
  CNodePtr( CNode * const pNode )
    : m_pNode( NULL )
  {
    Attach( pNode, true );
  }
  /// Release encapsulated pointer
  CNodePtr& operator=(int /*null*/)
  {
    // ASSERT(! null);
    InternalRelease();
    return *this;
  }

  /// Attach a raw pointer
  void Attach( CNode* p, bool fAddRef=false )
  {
    // ASSERT(! m_pNode);
    if (m_pNode)
      m_pNode->Release();
    m_pNode = p;
    if (p && fAddRef)
    {
      p->AddRef();
    }
  }

  /// Detach the raw pointer
  CNode *Detach()
  {
    CNode *p = m_pNode;
    m_pNode = NULL;
    return p;
  }

  /// Address of encapsulated pointer
  CNode** operator &()
  {
    return &m_pNode;
  }
  CNode * const * operator &() const
  {
    return &m_pNode;
  }

  /// Return encapsulated pointer
  CNode* operator->() const
  {
    return m_pNode;
  }
  /// 1 if a pointer is encapsulated pointer, 0 otherwise
  operator int() const
  {
    return m_pNode != NULL;
  }
  /// Return encapsulated pointer
  operator CNode*() const
  {
    return m_pNode;
  }

protected:
  /// Release encapsulated pointer and reset internal pointer 
  void InternalRelease() 
  {
    if (m_pNode) {
      m_pNode->Release();
      m_pNode = NULL;
    }
  }
private:
  /// Pointer to the managed Node
  CNode *m_pNode;
};

/// Utility to help retrieving the topology
class CTopology
{
public:
  /// Constructor
  CTopology( unsigned long initialSize=512 );
  /// Destructor
  ~CTopology();
private:
  CTopology( const CTopology& );
  CTopology& operator =( const CTopology& );
public:
  /// Attach an adapter
  CBcamAdapter* Attach( CBcamAdapter* );
  /// Detach the adapter
  CBcamAdapter* Detach();
  /// Get the event handle
  operator HANDLE();
  /// Cancels the pending calls
  void Cancel();
  /// Get the required size (valid after GetTreeResponse returned!)
  unsigned long RequiredSize() const;
  /// Set the size of the topology result buffer
  void SetSize( unsigned long );
  /// Wait until the device responded.
  unsigned long Wait( unsigned long timeout=INFINITE ) const;
  /// Request a topology tree asynchronously
  void GetTreeRequest( void* pContext=NULL );
  /// Get the response of the device and the context
  CNode* GetTreeResponse( void** ppContext=NULL );
private:
  CBcamAdapter* m_pAdapter;         ///< Pointer to an adapter
  CEvent m_Event;             ///< An event used for the asynchronous call
  BaccResGetTopology* m_pTopology;  ///< Pointer to the result structure
  BcamAdapterOL* m_pOL;             ///< Pointer to the overlapped structure
  bool m_IoPending;
};

//------------------------------------------------------------------------------
// class CBcamAdapter
// Author: HNebelun
// Date: 
//------------------------------------------------------------------------------
/**
 * \brief A IEEE1394 adapter card
 *
 * Gives access to the IEEE 1394 bus, retrieves information on isochronuous 
 * resources and the bus topology.
 * 
 */
//------------------------------------------------------------------------------
class CBcamAdapter  
{
public:
  CBcamAdapter();
  virtual ~CBcamAdapter();
private: // copying prohibited
  CBcamAdapter( const CBcamAdapter& );
  CBcamAdapter& operator=( const CBcamAdapter& );

public:
  /// Callback function type
  typedef void (*BcamCallbackFunc) (CBcamAdapter&, void*);

  // List all adapter currently found in the system
  static std::list<CString> DeviceNames();

  // Fill a list with all adapters currently found in the system
  static std::list<CString>& DeviceNames( std::list<CString>& );


  // Set a callback function which will called if a busrest occurrs
  void SetOnBusResetCallback( BcamCallbackFunc pfunc, void* pv = NULL );


  // Opens the driver
  void Open( CString DeviceName );

  // Closes the driver and frees all resources (esp. OVERLAPPED structures from cancelled driver calls)
  void Close();
  
  // Is the driver open
  bool IsOpen();

  // Request bus topology
  BcamAdapterOL* GetTreeAsync( HANDLE hEvent, BaccResGetTopology*, void* pv );
  // Get requested tree
  CNode* GetTree( BcamAdapterOL *pOL, void**pv );

  // Create tree with the bus topology
  CNode* GetTree() const;
  
  // Initiate a bus reset
  void ResetBus();

  // Number of available isochronous channels
  typedef std::bitset<64>ISOCHANNELSET;
  ULONG NumChannelAvailable( ISOCHANNELSET &s );
  // Available bandwidth in Packet per Frame
  ULONG ResourcesAvailable( ULONG Speed = TR_400 );

  // Read a Quadlet from the bus
  DWORD ReadQuad( ID, LARGE_INTEGER );
  // Write a Quadlet to the bus
  void WriteQuad( ID, LARGE_INTEGER, DWORD );
  // Read Quadlets blockwise
  DWORD ReadBlock( ID, LARGE_INTEGER, DWORD data[], DWORD numQuads );
  // Write quadlets blockwise
  void WriteBlock( ID, LARGE_INTEGER, const DWORD data[], DWORD numQuads );

  // Retrieve DeviceName
  CString GetDeviceName() const;

  // Retrieve Device

  HANDLE GetDevice() const
  {
	  return m_hDevice;
  }

protected:
  static CNode* BuildTree( BaccResGetTopology* pRes );
  static DWORD WINAPI BusResetThread( PVOID pv );
  void ClearBusResetContext();
private: //members

  HANDLE            m_hDevice;             ///< handle to the opened device 

  /// Reset context is used receiving bus reset notifications
  class CResetContext
  {
  public:
    BcamCallbackFunc  m_pResetCallback;      ///< pointer to a callback function
    void             *m_pResetContext;       ///< pointer to user defined context information
    bool              m_ResetThreadRunning;  ///< flag indicating whether the thread is running or not
    bool              m_ResetThreadStop;     ///< flag indicating whether to stop the tread or not
    HANDLE            m_hResetThread;        ///< handle to reset thread;
    HANDLE            m_hDevice;
    OVERLAPPED        m_Overlapped;
    CBcamAdapter     *m_pAdapter;
  } *m_pResetContext;   ///< null if notification is not registered

  CString           m_DeviceName;          ///< name of the device
  static HWND       s_hWndInterfaceNotify; ///< window handle for which we register device interface related notifications
  static HDEVNOTIFY s_hDevInterfaceNotify; ///< device notification handle for interface related notifications
};

inline CString CBcamAdapter::GetDeviceName() const
{
  return m_DeviceName;
}

} // namespace Bcam
#endif // !defined(AFX_BCAMADAPTER_H__34513919_0AE1_4812_B5BB_B6ABFE062D7B__INCLUDED_)

/**
 * Device class GUIDs. These are used to identify device interfaces. GUID definitions must
 * be outside the section protected from multiple inclusion, for reasons explained in the
 * DDK docs (but not easily understood).
 */
#include <bacc_guid.h>