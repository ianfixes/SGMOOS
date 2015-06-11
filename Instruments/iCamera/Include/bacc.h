//-----------------------------------------------------------------------------
//  Company:  Basler Vision Technologies
//  Section:  Vision Components
//  Project:  1394 Driver
//  Subproject:  Bus ACCess Driver
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/bacc.h,v 1.1.2.1 2004/07/27 19:55:59 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
  \file     bacc.h
  \brief   <type brief description here>
 *
 * <type long description here>
 *
 */
//-----------------------------------------------------------------------------

#ifndef BACC_H
#define BACC_H

#if defined(DRIVER)
  #include <wdm.h>
  #include <1394.h>
#else
  #include <basetyps.h>
  #include <winnt.h>
  #include <winioctl.h>
#endif


#define FILE_BACC_DEVICE 0x8080

/*
 * Explicitly specify structure member alignment, to become independent
 * of any alignment that may be in effect when this file is included.
 */
#pragma pack(push, 4)

/**
* Typesafe version of standard macro
*/
#define _CTL_CODE( DeviceType, Function, Method, Access ) (\
    (ULONG)(((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) ) \
)

/* Function ordinal numbers, used to construct driver control codes */
enum BaccOrd
{
  BaccOrdGetTopology = 0x800,
  BaccOrdGetDriverVersionString,
  BaccOrdGetResources,
  BaccOrdReadQuad,
  BaccOrdWriteQuad,
  BaccOrdFireBusReset,
  BaccOrdGetBusResetNotification,
  BaccOrdReadBlock,
  BaccOrdWriteBlock,
};


/**
 * \brief IOCTL_BACC_GET_DRIVER_VERSION_STRING
 * Get a string denoting the current driver version. 
 * \return Version string
 */
#define IOCTL_BACC_GET_DRIVER_VERSION_STRING \
  _CTL_CODE(FILE_BACC_DEVICE, BaccOrdGetDriverVersionString, METHOD_BUFFERED, FILE_ANY_ACCESS)

/**
 * \brief IOCTL_BACC_GET_TOPOLOGY
 * Get the bus topology
 * \return BaccResGetTopology
 */
#define IOCTL_BACC_GET_TOPOLOGY \
  _CTL_CODE(FILE_BACC_DEVICE, BaccOrdGetTopology, METHOD_BUFFERED, FILE_ANY_ACCESS)


/// Flagbits
typedef enum _Flagbits
{
  FB_Dcam1_0 = 0x1,
  FB_Dcam1_2 = 0x2,
  FB_Dcam1_3 = 0x4,
  FB_Bcam    = 0x8,
  FB_Sbp2    = 0x10,
  FB_Pc      = 0x20,
} FLAGBITS;

/// Maximum number of ports per node
#define MAX_PORTS (3+6+6+6)
/// Number of ports stored in a DWORD
#define PORTS_PER_DWORD 16
/// Bits used for a port
#define PORT_MASK 0x3
/// Number of char in names
#define MAX_STRING 64
/// Describes a 1394 bus node
typedef struct _NODE
{
  ULONG Flags;              ///< root, ...
  ULONG ID;                 ///< the device node number
  ULONG LinkActive;         ///< One if device's link and transaction layer are active, zero otherwise
  ULONG Speed;              ///< Transfer rate 100, 200, 400 mbit/s
  ULARGE_INTEGER NUID;           ///< Node Unique ID
  ULONG Ports[ (MAX_PORTS - 1) / PORTS_PER_DWORD + 1 ]; ///< connected type, 2bit per entry
  CHAR  Vendor[ MAX_STRING + 1 ]; ///< Vendor leaf
  CHAR  Model[ MAX_STRING + 1 ]; ///< Model leaf
} NODE;


#if defined(DRIVER)

/// Set connection state of a port
void SetPortState( NODE *pn, size_t _P, ULONG v );

/// Connection state of a port
ULONG GetPortState( NODE *pn, size_t _P );

#endif

/// Result of GetTopology
typedef struct _BaccResGetTopology
{
  ULONG Status;       ///< Zero if ok, non-zero if wrong size
  ULONG Size;         ///< Size of this struct
  ULONG NumNodes;     ///< Number of nodes
  NODE  Node[1];      ///< Array of nodes
} BaccResGetTopology;

typedef ULONG QUADLET;
typedef struct
{
  // ULONG Bus = 0x3ff; //local bus
  USHORT NodeID;
  USHORT Adr_High;
  ULONG  Adr_Low;
} ADDRESS;
/**
* \brief IOCTL_BACC_WRITE_QUAD
* Write a quadlet in IEEE1394 Address Space
*/
#define IOCTL_BACC_WRITE_QUAD \
  _CTL_CODE(FILE_BACC_DEVICE, BaccOrdWriteQuad, METHOD_BUFFERED, FILE_ANY_ACCESS)
typedef struct 
{
  ADDRESS Adr;
  QUADLET Data;
} BaccArgWriteQuad;

/**
* \brief IOCTL_BACC_WRITE_BLOCK
* Write a quadlet in IEEE1394 Address Space
*/
#define IOCTL_BACC_WRITE_BLOCK \
  _CTL_CODE(FILE_BACC_DEVICE, BaccOrdWriteBlock, METHOD_BUFFERED, FILE_ANY_ACCESS)
typedef struct 
{
  ADDRESS Adr;
  QUADLET Data[1];
} BaccArgWriteBlock;



/**
* \brief IOCTL_BACC_READ_QUAD
* Read a quadlet from IEEE1394 Address Space
* \return BaccResReadQuad
*/
#define IOCTL_BACC_READ_QUAD \
  _CTL_CODE(FILE_BACC_DEVICE, BaccOrdReadQuad, METHOD_BUFFERED, FILE_ANY_ACCESS)
typedef struct 
{
  ADDRESS Adr;
} BaccArgReadQuad;
typedef struct
{
  QUADLET Data;
} BaccResReadQuad;

/**
* \brief IOCTL_BACC_READ_BLOCK
* Read a block of quadlets from IEEE1394 Address Space
* \return Array of quadlets
*/
#define IOCTL_BACC_READ_BLOCK \
  _CTL_CODE(FILE_BACC_DEVICE, BaccOrdReadBlock, METHOD_BUFFERED, FILE_ANY_ACCESS)
typedef struct 
{
  ADDRESS Adr;
  ULONG NumQuadlets;
} BaccArgReadBlock;

/**
* \brief IOCTL_BACC_GET_RESOURCES
* Get the isochronous resources of a node
* \return BaccResGetResources
*/
#define IOCTL_BACC_GET_RESOURCES \
  _CTL_CODE(FILE_BACC_DEVICE, BaccOrdGetResources, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument of a GetResouces call
typedef struct
{
  ULONG Speed;       ///< Transfer rate: 0=100,1=200,2=400...
} BaccArgGetResources;
/// Result of a GetResources call
typedef struct
{
  ULONG BytesPerFrame;  ///< available bytes per frame
  ULONG Channels;       ///< number of available channels
  ULARGE_INTEGER ChSet;  ///< bitmap of available channels
} BaccResGetResources;

/**
* \brief IOCTL_BACC_FIRE_BUS_RESET
* Fire a bus reset
*/
#define IOCTL_BACC_FIRE_BUS_RESET \
  _CTL_CODE(FILE_BACC_DEVICE, BaccOrdFireBusReset, METHOD_BUFFERED, FILE_ANY_ACCESS)

/**
* \brief IOCTL_BACC_GET_BUS_RESET_NOTIFICATION
* Get notified on a bus reset
*/
#define IOCTL_BACC_GET_BUS_RESET_NOTIFICATION \
  _CTL_CODE(FILE_BACC_DEVICE, BaccOrdGetBusResetNotification, METHOD_BUFFERED, FILE_ANY_ACCESS)
 
#ifdef DRIVER

struct _NODE_INFO;
typedef struct _NODE_INFO *PNODE_INFO;

//
// This is used to keep track of pending irp's for
// notification of bus resets.
//
typedef struct _BUS_RESET_IRP {
    LIST_ENTRY      BusResetIrpList;
    PIRP            Irp;
} BUS_RESET_IRP, *PBUS_RESET_IRP;

typedef struct
{
  PDEVICE_OBJECT StackDeviceObject;    ///< Attached to in AddDevice()
  PDEVICE_OBJECT PortDeviceObject;     ///< 
  PDEVICE_OBJECT PhysicalDeviceObject; ///< Passed in AddDevice()

  UNICODE_STRING SymbolicLinkName;

  BOOLEAN IsShuttingDown;

  ULONG GenerationCount;

  ULONG NumNodes;
  PNODE_INFO NodeList;
  LIST_ENTRY BusResetIrps;

  KSPIN_LOCK ResetSpinLock;

  IO_REMOVE_LOCK RemoveLock;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


NTSTATUS DriverEntry( PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath );
NTSTATUS BaccDispatchPnp( PDEVICE_OBJECT pDevObj, PIRP pIrp );
NTSTATUS BaccDispatchIoControl( PDEVICE_OBJECT pDevObj, PIRP pIrp );
NTSTATUS BaccRetrieveGenerationCount( PDEVICE_OBJECT pDevObj, PIRP pIrp );
NTSTATUS BaccGetBusResetNotification( PDEVICE_OBJECT pDevObj, PIRP pIrp, ULONG flags );
VOID FreeNodeInfo( PNODE_INFO p);
VOID ClearNodeInfoList( PDEVICE_EXTENSION pDevExt );
NTSTATUS SubmitIrpSynch( PDEVICE_OBJECT pDevObj, PIRP pIrp, PIRB pIrb );
NTSTATUS SubmitIrpAsync( PDEVICE_OBJECT pDevObj, PIRP pIrp, PIRB pIrb );
NTSTATUS CopyTextualDesc( PANSI_STRING pAnsiStr, PTEXTUAL_LEAF pTxtDsc );
ULONG MIN( ULONG a, ULONG b );

#endif /* defined (DRIVER) */

#pragma pack(pop)
#endif /* !defined(BACC_H) */

/**
 * Device class GUIDs. These are used to identify device interfaces. GUID definitions must
 * be outside the section protected from multiple inclusion, for reasons explained in the
 * DDK docs (but not easily understood).
 */
#include "bacc_guid.h"

