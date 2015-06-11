 /////////////////////////////////////////////////////////////////////////////
 // Driver related error messages
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_BCAM_DRIVER_ERROR       0xFE0
#define FACILITY_BCAM_API_ERROR          0xFF0


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: STATUS_BCAM_UNSUCCESSFUL
//
// MessageText:
//
//  Unexpected driver error in bcam.sys.%0
//
#define STATUS_BCAM_UNSUCCESSFUL         ((ULONG)0xCFE00000L)

//
// MessageId: STATUS_BCAM_ASYNC_BUF_ALLOC
//
// MessageText:
//
//  A buffer for an asynchronous I/O operation could not be allocated.%0
//
#define STATUS_BCAM_ASYNC_BUF_ALLOC      ((ULONG)0xCFE00001L)

//
// MessageId: STATUS_BCAM_ASYNC_IO_ERROR
//
// MessageText:
//
//  An asynchronous I/O operation failed.%0
//
#define STATUS_BCAM_ASYNC_IO_ERROR       ((ULONG)0xCFE00002L)

//
// MessageId: STATUS_BCAM_NO_FEATURE_INQ_ADDRESS
//
// MessageText:
//
//  Kernel mode driver could not map a vailid inquiry address of the specified feature.%0
//
#define STATUS_BCAM_NO_FEATURE_INQ_ADDRESS ((ULONG)0xCFE00003L)

//
// MessageId: STATUS_BCAM_DEVICE_CREATE_FAILED
//
// MessageText:
//
//  Kernel mode driver failed to create a device object.%0
//
#define STATUS_BCAM_DEVICE_CREATE_FAILED ((ULONG)0xCFE00004L)

//
// MessageId: STATUS_BCAM_NO_FEATURE_CSR_ADDRESS
//
// MessageText:
//
//  The feature ID given could not be mapped to a valid CSR address.%0
//
#define STATUS_BCAM_NO_FEATURE_CSR_ADDRESS ((ULONG)0xCFE00005L)

//
// MessageId: STATUS_BCAM_FEATURE_MASK_NULL
//
// MessageText:
//
//  Mask argument does not specify any changes.%0
//
#define STATUS_BCAM_FEATURE_MASK_NULL    ((ULONG)0xCFE00006L)

//
// MessageId: STATUS_BCAM_ILLEGAL_VIDEO_FORMAT
//
// MessageText:
//
//  The selected video format is invalid.%0
//
#define STATUS_BCAM_ILLEGAL_VIDEO_FORMAT ((ULONG)0xCFE00007L)

//
// MessageId: STATUS_BCAM_ILLEGAL_VIDEO_MODE
//
// MessageText:
//
//  The selected video mode is invalid.%0
//
#define STATUS_BCAM_ILLEGAL_VIDEO_MODE   ((ULONG)0xCFE00008L)

//
// MessageId: STATUS_BCAM_ILLEGAL_FRAME_RATE
//
// MessageText:
//
//  The selected video frame rate is invalid.%0
//
#define STATUS_BCAM_ILLEGAL_FRAME_RATE   ((ULONG)0xCFE00009L)

//
// MessageId: STATUS_BCAM_WRONG_MODE
//
// MessageText:
//
//  Device is in Auto_Mode or Manual_mode.
//  Value of the specified feature cannot be written.%0
//
#define STATUS_BCAM_WRONG_MODE           ((ULONG)0xCFE0000AL)

//
// MessageId: STATUS_BCAM_WRITE_FAILED_NO_ABS
//
// MessageText:
//
//  Feature Abs_Control is not supported.
//  Feature cannot be set.%0
//
#define STATUS_BCAM_WRITE_FAILED_NO_ABS  ((ULONG)0xCFE0000BL)

//
// MessageId: STATUS_BCAM_NO_ON_OFF
//
// MessageText:
//
//  Feature On_Off is not supported.
//  Feature cannot be set.%0
//
#define STATUS_BCAM_NO_ON_OFF            ((ULONG)0xCFE0000CL)

//
// MessageId: STATUS_BCAM_NO_MANUAL_MODE
//
// MessageText:
//
//  Feature Manual_Mode is not supported.
//  Feature cannot be set.%0
//
#define STATUS_BCAM_NO_MANUAL_MODE       ((ULONG)0xCFE0000DL)

//
// MessageId: STATUS_BCAM_NO_AUTO_MODE
//
// MessageText:
//
//  Feature Auto_Mode is not supported.
//  Feature cannot be set.%0
//
#define STATUS_BCAM_NO_AUTO_MODE         ((ULONG)0xCFE0000EL)

//
// MessageId: STATUS_BCAM_TRIGGER_POL_READ_ONLY
//
// MessageText:
//
//  Feature Trigger-Polarity is read only.
//  Feature cannot be set.%0
//
#define STATUS_BCAM_TRIGGER_POL_READ_ONLY ((ULONG)0xCFE0000FL)

//
// MessageId: STATUS_BCAM_NO_ONE_PUSH
//
// MessageText:
//
//  Feature One_Push is not supported.
//  Feature cannot be set.%0
//
#define STATUS_BCAM_NO_ONE_PUSH          ((ULONG)0xCFE00010L)

//
// MessageId: STATUS_BCAM_NOT_READY_ONE_PUSH
//
// MessageText:
//
//  Device in Auto_Mode, cannot set One_Push.%0
//
#define STATUS_BCAM_NOT_READY_ONE_PUSH   ((ULONG)0xCFE00011L)

//
// MessageId: STATUS_BCAM_ONE_PUSH_BUSY
//
// MessageText:
//
//  The ONE_PUSH command was rejected because the device has not yet completed the previous command.%0
//
#define STATUS_BCAM_ONE_PUSH_BUSY        ((ULONG)0xCFE00012L)

//
// MessageId: STATUS_BCAM_CSR_NOT_WRITABLE
//
// MessageText:
//
//  On_Off_Mode is not set. CSR register is not writable.
//  CSR register cannot be set.%0
//
#define STATUS_BCAM_CSR_NOT_WRITABLE     ((ULONG)0xCFE00013L)

//
// MessageId: STATUS_BCAM_NO_ONE_SHOT
//
// MessageText:
//
//  Feature One Shot is not supported.%0
//
#define STATUS_BCAM_NO_ONE_SHOT          ((ULONG)0xCFE00014L)

//
// MessageId: STATUS_BCAM_SHADING_DATA_COMPLETED
//
// MessageText:
//
//  No more shading data for current AOI accepted.%0
//
#define STATUS_BCAM_SHADING_DATA_COMPLETED ((ULONG)0xCFE00015L)

//
// MessageId: STATUS_BCAM_SHADING_WRITE_DISABLED
//
// MessageText:
//
//  Shading Write flag is not enabled.%0
//
#define STATUS_BCAM_SHADING_WRITE_DISABLED ((ULONG)0xCFE00016L)

//
// MessageId: STATUS_BCAM_SHADING_FEATURE_NOT_SUPPORTED
//
// MessageText:
//
//  Shading advanced feature is not supported.%0
//
#define STATUS_BCAM_SHADING_FEATURE_NOT_SUPPORTED ((ULONG)0xCFE00017L)

//
// MessageId: STATUS_BCAM_UNLOCK_ERROR
//
// MessageText:
//
//  Advanced feature control register could not be unlocked.%0
//
#define STATUS_BCAM_UNLOCK_ERROR         ((ULONG)0xCFE00018L)

//
// MessageId: STATUS_BCAM_TESTIMAGE_FEATURE_NOT_SUPPORTED
//
// MessageText:
//
//  Test image advanced feature is not supported.%0
//
#define STATUS_BCAM_TESTIMAGE_FEATURE_NOT_SUPPORTED ((ULONG)0xCFE00019L)

//
// MessageId: STATUS_BCAM_TESTIMAGE_MODE_NOT_SUPPORTED
//
// MessageText:
//
//  Test image mode is not supported.%0
//
#define STATUS_BCAM_TESTIMAGE_MODE_NOT_SUPPORTED ((ULONG)0xCFE0001AL)

//
// MessageId: STATUS_BCAM_VERSION_FEATURE_NOT_SUPPORTED
//
// MessageText:
//
//  Camera version advanced feature is not supported.%0
//
#define STATUS_BCAM_VERSION_FEATURE_NOT_SUPPORTED ((ULONG)0xCFE0001BL)

//
// MessageId: STATUS_BCAM_READ_VERSION_FAILED
//
// MessageText:
//
//  An error while reading camera version occured.%0
//
#define STATUS_BCAM_READ_VERSION_FAILED  ((ULONG)0xCFE0001CL)

//
// MessageId: STATUS_BCAM_HOST_ADAPTER_CAPABILITIES
//
// MessageText:
//
//  The IEEE1394 host adapter capabilities are insufficient.%0
//
#define STATUS_BCAM_HOST_ADAPTER_CAPABILITIES ((ULONG)0xCFE0001DL)

//
// MessageId: STATUS_BCAM_MS_BUG_WORKAROUND
//
// MessageText:
//
//  Frame buffer sizes less than or equal to one isochronous packet could cause a system crash.%0
//
#define STATUS_BCAM_MS_BUG_WORKAROUND    ((ULONG)0xCFE0001EL)

//
// MessageId: STATUS_BCAM_FEATURE_VALUE_NOT_READABLE
//
// MessageText:
//
//  The feature value cannot be read.%0
//
#define STATUS_BCAM_FEATURE_VALUE_NOT_READABLE ((ULONG)0xCFE0001FL)

//
// MessageId: STATUS_BCAM_CAMERA_BPP_UNUSABLE
//
// MessageText:
//
//  The 'bytes per packet' value reported by the camera cannot be used.%0
//
#define STATUS_BCAM_CAMERA_BPP_UNUSABLE  ((ULONG)0x8FE00020L)

//
// MessageId: STATUS_BCAM_BUS_RESET
//
// MessageText:
//
//  Image contents may be damaged by a bus reset.%0
//
#define STATUS_BCAM_BUS_RESET            ((ULONG)0xCFE00021L)

//
// MessageId: STATUS_TOPOLOGY_INVALIDATED
//
// MessageText:
//
//  Topology information was invalidated by a bus reset.%0
//
#define STATUS_TOPOLOGY_INVALIDATED      ((ULONG)0xCFE00022L)

//
// MessageId: STATUS_BCAM_PRIVILEGE_DENIED
//
// MessageText:
//
//  Access to restricted features has been denied.%0
//
#define STATUS_BCAM_PRIVILEGE_DENIED     ((ULONG)0xCFE00023L)

 /////////////////////////////////////////////////////////////////////////////
 // User API related error messages
//
// MessageId: BCAM_E_REGISTER_DEVICE_NOTIFICATION_FAILED
//
// MessageText:
//
//  RegisterDeviceNotification() failed. GetLastError = %1!d!%0
//
#define BCAM_E_REGISTER_DEVICE_NOTIFICATION_FAILED ((ULONG)0xCFF00001L)

//
// MessageId: BCAM_E_REGISTER_CLIENT_NOT_ALLOWED
//
// MessageText:
//
//  There must not be any open devices when calling RegisterClient()%0
//
#define BCAM_E_REGISTER_CLIENT_NOT_ALLOWED ((ULONG)0xCFF00002L)

//
// MessageId: BCAM_E_UNREGISTER_CLIENT_NOT_ALLOWED
//
// MessageText:
//
//  Please, close all open devices before calling UnregisterClient()%0
//
#define BCAM_E_UNREGISTER_CLIENT_NOT_ALLOWED ((ULONG)0xCFF00003L)

//
// MessageId: BCAM_E_CAMERA_NOT_OPENED
//
// MessageText:
//
//  Camera object is not opened%0
//
#define BCAM_E_CAMERA_NOT_OPENED         ((ULONG)0xCFF00004L)

//
// MessageId: BCAM_E_INVALID_INQUIRY
//
// MessageText:
//
//  This type of camera feature supports no inquiriy for '%1!ld!'%0
//
#define BCAM_E_INVALID_INQUIRY           ((ULONG)0xCFF00005L)

//
// MessageId: BCAM_E_UNSUPPORTED_VIDEO_FORMAT
//
// MessageText:
//
//  This function doesn't support video format '%1!ld!'%0
//
#define BCAM_E_UNSUPPORTED_VIDEO_FORMAT  ((ULONG)0xCFF00006L)

//
// MessageId: BCAM_E_UNSUPPORTED_VIDEO_MODE
//
// MessageText:
//
//  This function doesn't support video mode '%1!ld!'%0
//
#define BCAM_E_UNSUPPORTED_VIDEO_MODE    ((ULONG)0xCFF00007L)

//
// MessageId: BCAM_E_UNSUPPORTED_COLOR_CODE
//
// MessageText:
//
//  This function doesn't support color code '%1!ld!'%0
//
#define BCAM_E_UNSUPPORTED_COLOR_CODE    ((ULONG)0xCFF00008L)

//
// MessageId: BCAM_E_TIMEOUT_GRAB
//
// MessageText:
//
//  A timeout while grabbing an image occured.%0
//
#define BCAM_E_TIMEOUT_GRAB              ((ULONG)0xCFF00009L)

//
// MessageId: BCAM_E_TIMEOUT_CANCEL
//
// MessageText:
//
//  A timeout while cancelling image acquisition occured.%0
//
#define BCAM_E_TIMEOUT_CANCEL            ((ULONG)0xCFF0000AL)

//
// MessageId: BCAM_E_ONESHOT_NOT_SUPPORTED
//
// MessageText:
//
//  The camera doesn't support the One Shot feature.%0
//
#define BCAM_E_ONESHOT_NOT_SUPPORTED     ((ULONG)0xCFF0000BL)

//
// MessageId: BCAM_E_UNEXPECTED_BUFFER
//
// MessageText:
//
//  GrabImage() received an unexpected image buffer, which was not queued in by GrabImage().%0
//
#define BCAM_E_UNEXPECTED_BUFFER         ((ULONG)0xCFF0000CL)

//
// MessageId: BCAM_E_UNSUPPORTED_COLOR_ID
//
// MessageText:
//
//  The camera reports an unsupported color coding ID ( ID = %1!ld! )%0
//
#define BCAM_E_UNSUPPORTED_COLOR_ID      ((ULONG)0xCFF0000DL)

//
// MessageId: BCAM_E_TESTIMAGE_NOT_SUPPORTED
//
// MessageText:
//
//  The camera doesn't support test image %1!d! %0
//
#define BCAM_E_TESTIMAGE_NOT_SUPPORTED   ((ULONG)0xCFF0000EL)

//
// MessageId: BCAM_E_INVALID_OR_UNKNOWN_FEATURE_ID
//
// MessageText:
//
//  Invalid or unknown feature ID%0
//
#define BCAM_E_INVALID_OR_UNKNOWN_FEATURE_ID ((ULONG)0xCFF0000FL)

//
// MessageId: BCAM_E_INVALID_TRANSMISSION_RATE
//
// MessageText:
//
//  Invalid or unknown transmission rate%0
//
#define BCAM_E_INVALID_TRANSMISSION_RATE ((ULONG)0xCFF00010L)

//
// MessageId: BCAM_E_BAG_ALREADY_EXISTS
//
// MessageText:
//
//  The specified property bag already exists%0
//
#define BCAM_E_BAG_ALREADY_EXISTS        ((ULONG)0xCFF00011L)

//
// MessageId: BCAM_E_BAG_DOES_NOT_EXIST
//
// MessageText:
//
//  The specified property bag does not exist%0
//
#define BCAM_E_BAG_DOES_NOT_EXIST        ((ULONG)0xCFF00012L)

//
// MessageId: BCAM_E_VALUE_NOT_FOUND
//
// MessageText:
//
//  The property bag doesn't contain the specified value%0
//
#define BCAM_E_VALUE_NOT_FOUND           ((ULONG)0xCFF00013L)

//
// MessageId: BCAM_E_INVALID_VALUE
//
// MessageText:
//
//  Invalid value found in the property bag%0
//
#define BCAM_E_INVALID_VALUE             ((ULONG)0xCFF00014L)

//
// MessageId: BCAM_E_CONFIGURATION_MISMATCH
//
// MessageText:
//
//  The configuration file doesn't match the current device%0
//
#define BCAM_E_CONFIGURATION_MISMATCH    ((ULONG)0xCFF00015L)

//
// MessageId: BCAM_E_CAMERA_ALREADY_OPEN
//
// MessageText:
//
//  The camera is already open%0
//
#define BCAM_E_CAMERA_ALREADY_OPEN       ((ULONG)0xCFF00016L)

//
// MessageId: BCAM_E_NOT_ALLOWED_FOR_REALTIME_CLASS
//
// MessageText:
//
//  This function call is not allowed for REALTIME_PRIORITY_CLASS processes.%0
//
#define BCAM_E_NOT_ALLOWED_FOR_REALTIME_CLASS ((ULONG)0xCFF00017L)

//
// MessageId: BCAM_E_DRIVER_NOT_INSTALLED
//
// MessageText:
//
//  The Bcam Driver is not installed.%0
//
#define BCAM_E_DRIVER_NOT_INSTALLED      ((ULONG)0x4FF00018L)

//
// MessageId: BCAM_E_DRIVER_VERSION
//
// MessageText:
//
//  The Versions of the BCAM API(%1!s!) and BCAM Driver(%2!s!) don't match.%0
//
#define BCAM_E_DRIVER_VERSION            ((ULONG)0x4FF00019L)

