//-----------------------------------------------------------------------------
//  Company:  Basler Vision Technologies
//  Section:  Vision Components
//  Project:  1394 Driver
//  Subproject:  BcamDrv
//  <b> Visual SourceSafe Information:</b>
//  $Archive: /Software/src/bcamdrv/functions.h $
//  $Author: moosauthor $
//  $Revision: 1.1.2.1 $
//  $Date: 2004/07/27 19:56:00 $
//-----------------------------------------------------------------------------
/**
  *  \file     functions.h
  *  \brief   Interface to the kernel-mode driver.
  *
  *  This file defines function codes for the Basler
  *  IEEE-1394 camera driver (BCAM). These codes are
  *  used to invoke the driver's functions.
  */
//-----------------------------------------------------------------------------

#if !defined(FUNCTIONS_H)
#define FUNCTIONS_H

#if defined(BCAM_DRIVER_BUILD)
  #include <wdm.h>
#else
  #include <basetyps.h>
  #include <winnt.h>
  #include <winioctl.h>
#endif

#include "classcode.h"

/**
* Typesafe version of standard macro
*/
#define _CTL_CODE( DeviceType, Function, Method, Access ) (\
    (ULONG)(((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) ) \
)

/*
 * Explicitly specify structure member alignment, to become independent
 * of any alignment that may be in effect when this file is included.
 */
#pragma pack(push, 4)

/* Function ordinal numbers, used to construct driver control codes */
enum
{
  OrdInitCamera = 0x800,
  OrdGetVideoFormatMask,
  OrdGetVideoModeMask,
  OrdGetFrameRateMask,
  OrdSetVideoFormatModeFrameRate,
  OrdGetCameraID,
  OrdGetString,
  OrdFmt7GetModeDescriptor,
  OrdFmt7SetAOI,
  OrdFmt7GetAOI,
  OrdFmt7GetColorMode,
  OrdFmt7SetColorMode,
  OrdGetBandwidthInfo,
  OrdGrabSingleFrame,
  OrdGrabMultipleFrames,
  OrdCameraControl,
  OrdReceiveData,
  OrdGetVersion,
  OrdSetTestImageCSR,
  OrdQueryDCSFeaturePresence,
  OrdQueryDCSFeatureInq,
  OrdSetDCSFeatureCSR,
  OrdGetDCSFeatureCSR,
  OrdGetDCSFeatureAbsValue,
  OrdGetDCSFeatureAbsRange,
  OrdAllocateResources,
  OrdFreeResources,
  OrdSetCameraBytesPerPacket,
  OrdGetVideoFormatModeFrameRate,
  OrdValidateCameraParameters,
  OrdGetDriverVersionString,
  OrdGetTestImageCSR,
  OrdSetShadingCtrl,
  OrdGetShadingCtrl,
  OrdGetShadingInquiry,
  OrdUploadShadingData,
  OrdFlush,
  OrdGetDcamVersion,
  OrdGetBasicFuncInq,
  OrdSetUserThreadPriority,
  OrdSetQueueServerPriority,
  OrdGetQueueServerPriority,
  OrdGetAuthChallenge,
  OrdAuthorize
};



/* Basic type definitions */

/// Camera unique identifier
typedef ULONGLONG CameraUniqueID;

/* Authorization challenge */
typedef UCHAR MD5Sum[16];

/** Rectangle --- used for AOI */
typedef struct
{
  ULONG X0, ///< Left, x-coordinate of top-left corner
    Y0,     ///< Top, y-coordinate of top-left corner
    Xsize,  ///< Width in pixels
    Ysize;  ///< Height in pixels
} BcamRect;

/** Flags denoting rectangle components */
#define BCAM_RECTCOMP_POSITION_X  0x1   ///< X position
#define BCAM_RECTCOMP_POSITION_Y  0x2   ///< Y position
#define BCAM_RECTCOMP_SIZE_X      0x4   ///< X size
#define BCAM_RECTCOMP_SIZE_Y      0x8   ///< Y size
#define BCAM_RECTCOMP_POSITION    (BCAM_RECTCOMP_POSITION_X | BCAM_RECTCOMP_POSITION_Y) ///< both x and y psitions
#define BCAM_RECTCOMP_SIZE        (BCAM_RECTCOMP_SIZE_X | BCAM_RECTCOMP_SIZE_Y)         ///< both x and y sizes
#define BCAM_RECTCOMP_ALL         (BCAM_RECTCOMP_POSITION | BCAM_RECTCOMP_SIZE)         ///< entire rectangle



/** Minimum result buffer size for IOCTL_BCAM_GET_DRIVER_VERSION_STRING */
#define BCAM_MAXIMUM_VERSION_STRING_SIZE 20



/** These are the color modes, as defined by the Digital Camera Specification. */
typedef enum
{
  DCSColor_Mono8 = 0,     ///< Y component has 8bit data
  DCSColor_YUV8_4_1_1,    ///< 4:1:1 YUV 8 format, each component has 8bit data
  DCSColor_YUV8_4_2_2,    ///< 4:2:2 YUV 8 format, each component has 8bit data
  DCSColor_YUV8_4_4_4,    ///< 4:4:4 YUV 8 format, each component has 8bit data
  DCSColor_RGB8,          ///< RGB 8 format, each component has 8bit data
  DCSColor_Mono16,        ///< Y component has 16bit unsigned data
  DCSColor_RGB16,         ///< RGB 16 format, each component has 16bit unsigned data
  DCSColor_SMono16,       ///< Y component has 16bit signed data
  DCSColor_SRGB16,        ///< RGB 16 format, each component has 16bit signed data
  DCSColor_Raw8,          ///< Raw data output of color filter sensor, 8bit data
  DCSColor_Raw16,          ///< Raw data output of color filter sensor, 16bit data
  DCSColor_VendorSpecific0 = 128,  ///< First of 128 vendor specific color codes
  DCSColor_VendorSpecific127 = 255,///< Last of 128 vendor specific color codes
  NumDCSColorCodes
} DCSColorCode;



/** Isochronous speed encoding */
typedef enum
{
  IsoSpeed_Unknown,
  IsoSpeed_100,   ///< 100Mbit/s
  IsoSpeed_200,   ///< 200Mbit/s
  IsoSpeed_400,   ///< 400Mbit/s
  IsoSpeed_800,   ///< 800Mbit/s
  IsoSpeed_1600,  ///< 1600Mbit/s
  IsoSpeed_3200   ///< 3200Mbit/s
} BcamIsoSpeed;


/// These are the test image modes
typedef enum
{
  TestImage_Disabled,     ///< Test image disabled
  TestImage_1,            ///< Test image 1 active
  TestImage_2,            ///< Test image 2 active
  TestImage_3,            ///< Test image 3 active
  TestImage_4,            ///< Test image 4 active
  TestImage_5,            ///< Test image 5 active
  TestImage_6,            ///< Test image 6 active
  TestImage_7             ///< Test image 7 active
} ImageOnMode;


/// Test image feature
typedef struct
{
  ULONG                 : 13;   
  ULONG ImageOn         :  3;   ///< Test image mode
  ULONG                 :  1;   
  ULONG ImageInq7       :  1;   ///< Presence of test image 7
  ULONG ImageInq6       :  1;   ///< Presence of test image 6
  ULONG ImageInq5       :  1;   ///< Presence of test image 5
  ULONG ImageInq4       :  1;   ///< Presence of test image 4
  ULONG ImageInq3       :  1;   ///< Presence of test image 3
  ULONG ImageInq2       :  1;   ///< Presence of test image 2
  ULONG ImageInq1       :  1;   ///< Presence of test image 1
  ULONG                 :  7;
  ULONG Presence_Inq    :  1;   ///< Presence of this feature
} BcamTestImage;     

/// Shading feature
typedef struct
{
  ULONG     ShadingMode;    ///< 0:Shading Correction disabled / !=0:Shading mode enabled
  BOOLEAN   Test;           ///< TRUE:Camera outputs the current shading data as an image / FALSE:Normal camera operation
  BOOLEAN   WriteEnable;    ///< TRUE: Enables write access to shading data memory / FALSE: Disable
  BOOLEAN   Completed;      ///< TRUE: Correct number of expected bytes written or more bytes written than expected / FALSE: Less bytes written than expected 
}BcamShadingCtrl;

/// Shading inquiry
typedef struct
{
  ULONG   ModeFlags;       ///< Presence of shading modes
  BOOLEAN Present;         ///< Presence of this feature
}
BcamShadingInq;

/// A struct describing the image layout in format 7
typedef struct
{
  ULONG nTotalPixX          ///< Width in pixels
    , nTotalPixY;           ///< Height in pixels
  ULONG nSizeIncX,          ///< Increment for the image width
    nSizeIncY;              ///< Increment for the image height
  ULONG nPosIncX,           ///< Increment for the x-position
    nPosIncY;               ///< Increment for the y-position
  ULONG ColorCodingMask[8]; ///< Bitfield describing the supported color modes
} BcamVideoFormat7Descriptor;



/// Bandwidth and buffer related data 
typedef struct
{
  ULONGLONG nBytesTotalFrameSize; ///< Image size in bytes
  ULONG nBytesPerPacketMax;       ///< Maximum packet size in bytes
  ULONG nBytesPerPacketRec;       ///< Recommended packet size in bytes
  ULONG nBytesPerPacketUnit;      ///< Increment for the packet size in bytes
  ULONG nBytesPerPacketCurrent;   ///< Current packet size in bytes
  ULONG nPacketsPerFrame;         ///< Number of packet per image
} BcamBwDesc;



/** Identifiers used to query the camera for string data */
typedef enum
{
  String_VendorName,    ///< Specifies the camara vendor name
  String_ModelName,     ///< Specifies the camera model name
  String_CameraVersion  ///< Specifies the camera version
} BcamStringID;



/** Feature ID codes */
typedef enum
{
  /* Standard DCS features */
  FeatureID_CaptureQuality = 0x0E,  ///< Specifies the image quality for format 6
  FeatureID_CaptureSize,            ///< Specifies the image size for format 6
  FeatureID_OpticalFilter = 0x1C,   ///< Specifies the optical filter control
  FeatureID_Tilt,                   ///< Specifies the tilt control
  FeatureID_Pan,                    ///< Specifies the pan control
  FeatureID_Zoom,                   ///< Specifies the zoom control
  FeatureID_Trigger = 0x33,         ///< Specifies the trigger control
  FeatureID_Temparature,            ///< Specifies the temperature control
  FeatureID_Focus,                  ///< Specifies the focus control
  FeatureID_Iris,                   ///< Specifies the iris control
  FeatureID_Gain,                   ///< Specifies the gain control
  FeatureID_Shutter,                ///< Specifies the shutter control
  FeatureID_Gamma,                  ///< Specifies the gamma control
  FeatureID_Saturation,             ///< Specifies the saturation control
  FeatureID_Hue,                    ///< Specifies the hue control
  FeatureID_WhiteBalance,           ///< Specifies the white balance control
  FeatureID_Sharpness,              ///< Specifies the sharpness control
  FeatureID_AutoExposure,           ///< Specifies the auto exposure control
  FeatureID_Brightness,             ///< Specifies the brightness control
} BcamFeatureID;

/** Feature flag bit shift values */
typedef enum
{
  FeatBshift_Present = 0,                       ///< Specifies the feature presence bit
  FeatBshift_AbsControl,                        ///< Specifies the absolute value control
  FeatBshift_OnePush = 3,                       ///< Specifies the one-push automatic control
  FeatBshift_ReadOut,                           ///< Specifies the readout capability
  FeatBshift_OnOff,                             ///< Specifies the on/off control
  FeatBshift_Auto,                              ///< Specifies the automatic mode control
  FeatBshift_TriggerPolarity = FeatBshift_Auto, ///< Specifies the trigger input polatity control
  FeatBshift_Manual                             ///< Specifies the manual mode control
};

/*
 * All features are equal, but some are more equal than others, so we have
 * to deal with a few special cases here. That's why there is that union.
 */
/// Descriptor used for inquiring the subfeatures of a DCAM feature
typedef union
{
  struct
  {
    ULONG Max_Value       : 12;   ///< Maximum value for this feature
    ULONG Min_Value       : 12;   ///< Minimum value for this feature
    ULONG Manual_Inq      :  1;   ///< Availability of manual mode
    ULONG Auto_Inq        :  1;   ///< Availability of automatic mode
    ULONG On_Off_Inq      :  1;   ///< Availability of swithing this feature on and off
    ULONG Read_Out_Inq    :  1;   ///< Capability of reading the value of this feature
    ULONG One_Push_Inq    :  1;   ///< Availability of one push automatic mode
    ULONG                 :  1;
    ULONG Abs_Control_Inq :  1;   ///< Availability of absolute control
    ULONG Presence_Inq    :  1;   ///< Presence of this feature
  } Scalar;       ///< a scalar feature

  struct
  {
    ULONG                   : 12;
    ULONG Trigger_Mode3_Inq :  1; ///< Presence of Trigger mode 3
    ULONG Trigger_Mode2_Inq :  1; ///< Presence of Trigger mode 2
    ULONG Trigger_Mode1_Inq :  1; ///< Presence of Trigger mode 1
    ULONG Trigger_Mode0_Inq :  1; ///< Presence of Trigger mode 0
    ULONG                   :  9;
    ULONG Polarity_Inq      :  1; ///< Capability of changing the polarity of the trigger input
    ULONG On_Off_Inq        :  1; ///< Availability of switching this feature on and off
    ULONG Read_Out_Inq      :  1; ///< Capability of reading the value of this feature
    ULONG                   :  2; 
    ULONG Abs_Control_Inq   :  1; ///< Availability of abolute control
    ULONG Presence_Inq      :  1; ///< Presence of this feature
  } Trigger;      ///< trigger feature
} BcamFeatureInqDescriptor;


/** Write mask for setting DCS feature CSR */
#define DCS_FEATURE_CSR_SET_VALUE               0x0001    ///< Specifies the value control
#define DCS_FEATURE_CSR_SET_A_M_MODE            0x0002    ///< Specifies the automatic/manual mode control
#define DCS_FEATURE_CSR_SET_TRIGGER_POLARITY    0x0004    ///< Specifies the trigger input polarity control
#define DCS_FEATURE_CSR_SET_ON_OFF              0x0008    ///< Specifies the on/off control
#define DCS_FEATURE_CSR_SET_ONE_PUSH            0x0010    ///< Specifies the one-push automatic mode control
#define DCS_FEATURE_CSR_SET_ABS_CONTROL         0x0020    ///< Specifies the absolute value control
#define DCS_FEATURE_CSR_SET_UB_VALUE            0x0040    ///< Specifies the white balance U/B value
#define DCS_FEATURE_CSR_SET_VR_VALUE            0x0080    ///< Specifies the white balance V/R value
#define DCS_FEATURE_CSR_SET_TARGET_TEMPERATURE  0x0100    ///< Specifies the target temperature
#define DCS_FEATURE_CSR_SET_TRIGGER_MODE        0x0200    ///< Specifies the trigger mode
#define DCS_FEATURE_CSR_SET_PARAMETER           0x0400    ///< Specifies the trigger parameter
#define DCS_FEATURE_CSR_SET_VALID_FLAGS         0x07ff    ///< Specifies the set of valid flag bits

/** Write mask for setting shading advanced feature CSR */
#define BCAM_SHADING_CTRL_TEST                  0x0001    ///< Specifies the value Test
#define BCAM_SHADING_CTRL_SHADINGMODE           0x0002    ///< Specifies the value ShadingMode
#define BCAM_SHADING_CTRL_WRITEENABLE           0x0004    ///< Specifies the value ShadingMode
#define BCAM_SHADING_CTRL_VALID_FLAGS           0x0007    ///< Specifies the set of valid flag bits


/*
 * All features are equal, but some are more equal than others, so we have
 * to deal with a few special cases here. That's why there is that union.
 */
/// Descriptor for controlling a DCAM feature
typedef union
{
  struct
  {
    ULONG Value             : 12; ///< Value of this feature
    ULONG                   : 12; 
    ULONG A_M_Mode          :  1; ///< Automatic mode - manual mode switch
    ULONG ON_OFF            :  1; ///< Switch to turn this feature on or off, off is 0, on is 1
    ULONG One_Push          :  1; ///< One push automatic mode, off is 0, on is 1
    ULONG                   :  3;
    ULONG Abs_Control       :  1; ///< Absolute value control
    ULONG Presence_Inq      :  1; ///< Presence of this feature
  } Scalar; ///< a scalar feature
  struct
  {
    ULONG Parameter         : 12; ///< Parameter of the trigger function
    ULONG                   :  4;
    ULONG Trigger_Mode      :  4; ///< Trigger mode 0-15
    ULONG                   :  4;
    ULONG Trigger_Polarity  :  1; ///< Trigger polarity, high active is 1, low active is 0
    ULONG ON_OFF            :  1; ///< Switch to turn this feature on or off, off is 0, on is 1
    ULONG One_Push          :  1; ///< One push automatic mode, off is 0, on is 1
    ULONG                   :  3;
    ULONG Abs_Control       :  1; ///< Absolute value control
    ULONG Presence_Inq      :  1; ///< Presence of this feature
  } Trigger; ///< Trigger feature

  struct
  {
    ULONG V_R               : 12; ///< V value or R value
    ULONG U_B               : 12; ///< U value or B value
    ULONG A_M_Mode          :  1; ///< Automatic mode - manual mode switch
    ULONG ON_OFF            :  1; ///< Switch to turn this feature on or off, off is0, on is 1
    ULONG One_Push          :  1; ///< One push automatic mode, off is 0, on is 1
    ULONG                   :  3;
    ULONG Abs_Control       :  1; ///< Absolute value control
    ULONG Presence_Inq      :  1; ///< Presence of this feature
  } WhiteBalance; ///< White balance feature

  struct
  {
    ULONG Temperature       : 12; ///< Current value
    ULONG Target_Temperature: 12; ///< Aimed value
    ULONG A_M_Mode          :  1; ///< Automatic mode - manual mode switch
    ULONG ON_OFF            :  1; ///< Switch to turn this feature on or off, off is0, on is 1
    ULONG One_Push          :  1; ///< One push automatic mode, off is 0, on is 1
    ULONG                   :  3;
    ULONG Abs_Control       :  1; ///< Absolute value control
    ULONG Presence_Inq      :  1; ///< Presence of this feature
  } Temperature; ///< Temperature feature
} BcamFeatureCSRDescriptor;

/** Macro to test whether a particular feature supports a given operation */
#define BcamFeatureTest(desc, shft) ((desc)->Flags & (0x1UL << shft) != 0)


/** Commands for grabbing \see DCAM, ISO_EN, SINGLESHOT, MULTISHOT */
typedef enum
{
  CamCmd_Stop,        ///< Turns ISO_EN off
  CamCmd_Start,       ///< Turns ISO_EN on
  CamCmd_SingleGrab,  ///< Starts a single grab
  CamCmd_MultiGrab    ///< Starts a sequence of grabs
} BcamCameraCommand;


/**
 * \brief IOCTL_BCAM_INIT_CAMERA
 * Initialize the camera.
 * Reset camera to factory defaults. 
 * This function neither has any arguments nor results.
 *
 */
#define IOCTL_BCAM_INIT_CAMERA \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdInitCamera, METHOD_NEITHER, FILE_ANY_ACCESS)


 
/**
 * \brief IOCTL_BCAM_GET_VIDEO_FORMAT_MASK
 *  Query supported video formats.
 * \return ResGetVideoFormatMask Return a mask word that has a bit set for every video format supported by the camera.
 * 
 */
#define IOCTL_BCAM_GET_VIDEO_FORMAT_MASK \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetVideoFormatMask, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Mask with video mode
typedef struct
{
  ULONG VideoFormatMask; ///< Mask with the supported video formats
} ResGetVideoFormatMask;



/**
 * \brief IOCTL_BCAM_GET_VIDEO_MODE_MASK
 * Query supported video modes.
 * \param arg : ArgGetVideoModeMask; Specifies the video format
 * \return ResGetVideoModeMask A mask word that has a bit set for every video mode supported by the camera
 * for a given video format.
 */
#define IOCTL_BCAM_GET_VIDEO_MODE_MASK \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetVideoModeMask, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the GetVideoModeMask function
typedef struct
{
  ULONG VideoFormat;  ///< Specifies the video format
} ArgGetVideoModeMask;
/// Result of the GetVideoModeMask function
typedef struct
{
  ULONG VideoMode;    ///< Mask with the supported video modes
} ResGetVideoModeMask;



/**
 * \brief IOCTL_BCAM_GET_FRAME_RATE_MASK
 * Query supported frame rates
 * \param arg : ArgGetFrameRateMask, Specifies video format and mode
 * \return ResGetFrameRateMask a mask word that has a bit set for every FPS (frames per second) value supported
 * by the camera for a particular format/mode combination.
 */
#define IOCTL_BCAM_GET_FRAME_RATE_MASK \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetFrameRateMask, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the GetFrameRateMask function
typedef struct
{
  ULONG VideoFormat,    ///< Specifies the video format
    VideoMode;          ///< Specifies the video mode
} ArgGetFrameRateMask;
/// Result of the GetFrameRateMask function
typedef struct
{
  ULONG FrameRateMask;  ///< Mask with the supported frame rates
} ResGetFrameRateMask;



/**
 * \brief IOCTL_BCAM_SET_VIDEO_FORMAT_MODE_FRAME_RATE
 * Set the camera to use a particular video format/mode combination.
 * If format is 7, FrameRate is ignored.
 * \param arg : ArgSetVideoFormatModeFrameRate
 */
#define IOCTL_BCAM_SET_VIDEO_FORMAT_MODE_FRAME_RATE \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdSetVideoFormatModeFrameRate, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument of the SetVideoFormatModeFrameRate function
typedef struct
{
  ULONG VideoFormat,  ///< Specifies the video format
    VideoMode,        ///< Specifies the video mode
    FrameRate;        ///< Specifies the frame rate
} ArgSetVideoFormatModeFrameRate;



/**
 * \brief IOCTL_BCAM_GET_CAMERA_ID
 * Get the camera's unique ID.
 * This function does not have any arguments. 
 * \return ResGetCameraID It returns a 64-bit camera ID.
 */
#define IOCTL_BCAM_GET_CAMERA_ID \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetCameraID, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Result of the GetCameraID function
typedef struct
{
  CameraUniqueID Id; ///< Specifies the camera unique identifier
} ResGetCameraID;



/**
 * \brief IOCTL_BCAM_GET_STRING
 * Query camera for string data.
 * \param arg : ArgGetString a string identifier
 * \return PCHAR Pointer to the filled buffer
 */
#define IOCTL_BCAM_GET_STRING \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetString, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the GetString function
typedef struct
{
  BcamStringID Id;  ///< Specifies the string to get.
} ArgGetString;



/**
 * \brief IOCTL_BCAM_FMT7_GET_MODE_DESCRIPTOR
 * Get data describing a particular video mode in format 7. 
 *
 * If the camera does not support
 * format 7, these operations will fail. 
 *
 * \param arg : ArgFmt7GetModeDescriptor a mode number
 * \return #ResFmt7GetModeDescriptor a mode descriptor.
 */
#define IOCTL_BCAM_FMT7_GET_MODE_DESCRIPTOR \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdFmt7GetModeDescriptor, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the Fmt7GetModeDescriptor function
typedef struct
{
  ULONG VideoMode;  ///< Specifies the video mode
} ArgFmt7GetModeDescriptor;
/** \brief Result of the Fmt7GetModeDescriptor function */
typedef BcamVideoFormat7Descriptor ResFmt7GetModeDescriptor;



/**
 * \brief IOCTL_BCAM_FMT7_SET_AOI
 * Set AOI as described by arguments.
 * \param arg : ArgFmt7SetAOI the aoi to set
 */
#define IOCTL_BCAM_FMT7_SET_AOI \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdFmt7SetAOI, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the Fmt7SetAOI function
typedef struct
{
  ULONG VideoMode;  ///< Specifies the video mode
  BcamRect AOI;     ///< Specifies the area of interest
  ULONG Mask;       ///< Bitmask for components to set (position, size or both)
} ArgFmt7SetAOI;



/**
 * \brief IOCTL_BCAM_FMT7_GET_AOI
 * Get AOI currently set. 
 * \param arg : ArgFmt7GetAOI is mode number. 
 * \return #ResFmt7GetAOI is a AOI descriptor.
 */
#define IOCTL_BCAM_FMT7_GET_AOI \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdFmt7GetAOI, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the Fmt7GetAOI function
typedef struct
{
  ULONG VideoMode; ///< Specifies the video mode
} ArgFmt7GetAOI;
/// Parameter of the Fmt7GetAOI function
typedef BcamRect ResFmt7GetAOI;



/**
 * \brief IOCTL_BCAM_FMT7_GET_COLOR_MODE
 * Get the color code currently set.
 * \param arg : ArgFmt7GetColorMode the mode number
 * \return ResFmt7GetColorMode the mode currently set
 */
#define IOCTL_BCAM_FMT7_GET_COLOR_MODE \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdFmt7GetColorMode, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the Fmt7GetColorMode function
typedef struct
{
  ULONG VideoMode;        ///< Specifies the video mode
} ArgFmt7GetColorMode;
/// Result of the Fmt7GetColorMode function
typedef struct
{
  DCSColorCode ColorMode; ///< Specifies the image type
} ResFmt7GetColorMode;



/**
 * \brief IOCTL_BCAM_FMT7_SET_COLOR_MODE
 * Set the color mode for a particular video mode (format 7).
 * \param arg : ArgFmt7SetColorMode the image type
 */
#define IOCTL_BCAM_FMT7_SET_COLOR_MODE \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdFmt7SetColorMode, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument of the Fmt7SetColorMode function
typedef struct
{
  ULONG VideoMode;        ///< Specifies the video mode
  DCSColorCode ColorMode; ///< Specifies the image type
} ArgFmt7SetColorMode;



/**
 * \brief IOCTL_BCAM_GET_BANDWIDTH_INFO
 * Query camera for bandwidth-related data (format 7).
 * \param arg : ArgGetBandwidthInfo
 * \return #ResGetBandwidthInfo
 */
#define IOCTL_BCAM_GET_BANDWIDTH_INFO \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetBandwidthInfo, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the GetBandwidthInfo function
typedef struct
{
  ULONG VideoMode;    ///< Specifies the video mode
} ArgGetBandwidthInfo; 
/// Result of the GetBandwidthInfo function
typedef BcamBwDesc ResGetBandwidthInfo; 



/**
 * \brief IOCTL_BCAM_CAMERA_CONTROL
 * Start/stop image aquisition.
 * \param arg : ArgCameraControl
 */
#define IOCTL_BCAM_CAMERA_CONTROL \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdCameraControl, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the CameraControl function.
typedef struct
{
  BcamCameraCommand Cmd;  ///< Specifies the camera command
  ULONG nFrames;          ///< Specifies the number of images to grab in case of CamCmd_MultiGrab
} ArgCameraControl;



/**
 * \brief IOCTL_BCAM_RECEIVE_DATA
 * Receive data sent by camera and place it in the buffer passed with the request.
 * \param arg : PVOID; Pointer to the image buffer
 */
#define IOCTL_BCAM_RECEIVE_DATA \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdReceiveData, METHOD_IN_DIRECT, FILE_ANY_ACCESS)



/**
 * \brief IOCTL_BCAM_QUERY_DCS_FEATURE_PRESENCE
 * Obtain information about the feature presence for current format/mode combination.
 * \return ResQueryDCSFeaturePresence the feature descriptor
 */
#define IOCTL_BCAM_QUERY_DCS_FEATURE_PRESENCE \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdQueryDCSFeaturePresence, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Result of the QueryDCSFeaturePresence
typedef struct
{
  ULONGLONG FeatureFlags; ///< Mask with the supported features.
} ResQueryDCSFeaturePresence; 


/**
 * \brief IOCTL_BCAM_QUERY_DCS_FEATURE_INQ
 * Obtain information about a feature.
 * \param arg : ArgQueryDCSFeatureInq; the feature
 * \return #ResQueryDCSFeatureInq the feature descriptor
 */
#define IOCTL_BCAM_QUERY_DCS_FEATURE_INQ \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdQueryDCSFeatureInq, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument to the QueryDCSFeatureInq function
typedef struct
{
  BcamFeatureID FeatId; ///< The feature identifier
} ArgQueryDCSFeatureInq; 
/// Result for the QueryDCSFeatureInq function
typedef BcamFeatureInqDescriptor ResQueryDCSFeatureInq; 


/**
 * \brief IOCTL_BCAM_SET_DCS_FEATURE_CSR
 * Set a feature's value.
 * \param arg : ArgSetDCSFeatureCSR the feature descriptor
 */
#define IOCTL_BCAM_SET_DCS_FEATURE_CSR \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdSetDCSFeatureCSR, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument to the SetDCSFeatureCSR function
typedef struct
{
  BcamFeatureID FeatId;                       ///< Specifies the feature
  BcamFeatureCSRDescriptor FeatCSRDescriptor; ///< The feature descriptor
  ULONG Mask;                                 ///< Bitmask specifying changes to apply
} ArgSetDCSFeatureCSR;


/**
 * \brief IOCTL_BCAM_GET_DCS_FEATURE_CSR
 * Get a feature's value.
 * \param arg : ArgGetDCSFeatureCSR
 * \return #ResGetDCSFeatureCSR
 */
#define IOCTL_BCAM_GET_DCS_FEATURE_CSR \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetDCSFeatureCSR, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument to the GetDCSFeatureCSR function
typedef struct
{
  BcamFeatureID FeatId;  ///< Specifies the feature
  BOOLEAN Reserved;      ///< This argument is currently unused. 
} ArgGetDCSFeatureCSR; 
/// Result of the GetDCSFeatureCSR function
typedef BcamFeatureCSRDescriptor ResGetDCSFeatureCSR; 


/**
 * \brief IOCTL_BCAM_ALLOCATE_RESOURCES
 * Allocate isochronous resources (channel, bandwidth...)
 * \param arg : ArgAllocateResources;
 * \return ResAllocateResources
 */
#define IOCTL_BCAM_ALLOCATE_RESOURCES \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdAllocateResources, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument to the AllocateResources function
typedef struct
{
  ULONG nBytesPerPacket;    ///< Packet size in bytes
  ULONG nNumberOfBuffers;   ///< Number of Buffers
  ULONG nMaxBufferSize;     ///< Maximum buffer size in bytes
} ArgAllocateResources; 
/// Result of the AllocateResource function
typedef struct
{ 
  ULONG Channel;                ///< Isochronous channnel allocated
  ULONG nBytesPerPacketRemain;  ///< Unused bandwidth remaining after allocation
  BcamIsoSpeed Speed;           ///< Isochronous transmission speed for device
} ResAllocateResources;



/**
 * \brief IOCTL_BCAM_FREE_RESOURCES
 * Free isochronous resources
 */
#define IOCTL_BCAM_FREE_RESOURCES \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdFreeResources, METHOD_NEITHER, FILE_ANY_ACCESS)



/**
 * \brief IOCTL_BCAM_SET_CAMERA_BYTES_PER_PACKET
 * Set bytes-per-packet value for camera (video format 7)
 * \param arg ArgSetCameraBytesPerPacket;
 */
#define IOCTL_BCAM_SET_CAMERA_BYTES_PER_PACKET \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdSetCameraBytesPerPacket, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the SetCameraBytesPerPacket function
typedef struct
{
  ULONG VideoMode,      ///< The video mode of format 7
    nBytesPerPacket;    ///< The packet size in bytes.
} ArgSetCameraBytesPerPacket;



/**
 * \brief IOCTL_BCAM_GET_VIDEO_FORMAT_MODE_FRAME_RATE
 * Get current video format, mode and frame rate. 
 * \return ResGetVideoFormatModeFrameRate If video format is 7, * returns ~0 for frame rate.
 */
#define IOCTL_BCAM_GET_VIDEO_FORMAT_MODE_FRAME_RATE \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetVideoFormatModeFrameRate, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Result of the GetVideoFormatModeFrameRate function.
typedef struct
{
  ULONG VideoFormat,  ///< The video format
    VideoMode,        ///< The video mode
    FrameRate;        ///< The frame rate
} ResGetVideoFormatModeFrameRate;



/**
 * \brief IOCTL_BCAM_VALIDATE_CAMERA_PARAMETERS
 * Check parameter set currently programmed into camera for consistency. 
 * \return None
 */
#define IOCTL_BCAM_VALIDATE_CAMERA_PARAMETERS \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdValidateCameraParameters, METHOD_NEITHER, FILE_ANY_ACCESS)



/**
 * \brief IOCTL_BCAM_GET_DRIVER_VERSION_STRING
 * Get a string denoting the current driver version. 
 * \return Version string
 */
#define IOCTL_BCAM_GET_DRIVER_VERSION_STRING \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetDriverVersionString, METHOD_BUFFERED, FILE_ANY_ACCESS)


/**
 * \brief IOCTL_BCAM_SET_TEST_IMAGE_CSR
 * Set the test image advances feature CSR. 
 * \param arg ArgSetTestImageCSR
 * \return None
 */
#define IOCTL_BCAM_SET_TEST_IMAGE_CSR \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdSetTestImageCSR, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the SetTestImageCSR function
typedef struct
{
  BcamTestImage TestImageCSR; ///< Specifies the test image mode
} ArgSetTestImageCSR;

/**
 * \brief IOCTL_BCAM_GET_TEST_IMAGE_CSR
 * Get the test image advanced feature CSR. 
 * \return ResGetTestImageCSR
 */
#define IOCTL_BCAM_GET_TEST_IMAGE_CSR \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetTestImageCSR, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Result of the GetTestImageCSR function
typedef struct
{
  BcamTestImage TestImageCSR; ///< Specifies the test image advanced feature control register
} ResGetTestImageCSR;


/**
 * \brief IOCTL_BCAM_SET_SHADING_CTRL
 * Set the shading advanced feature control register. 
 * \param arg ArgSetShadingCtrl
 * \return None
 */
#define IOCTL_BCAM_SET_SHADING_CTRL \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdSetShadingCtrl, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Argument for the SetShadingCtrl function
typedef struct 
{
  BcamShadingCtrl ShadingCtrl;  ///< Specifies the shading mode
  ULONG Mask;                   ///< Bitmask for components to set (ShadingMode or Test)
}ArgSetShadingCtrl; 


/**
 * \brief IOCTL_BCAM_GET_SHADING_CTRL
 * Get the shading advanced feature control register. 
 * \return #ResGetShadingCtrl
 */
#define IOCTL_BCAM_GET_SHADING_CTRL \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetShadingCtrl, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Result of the GetShadingCtrl function
typedef BcamShadingCtrl ResGetShadingCtrl; 

/**
 * \brief IOCTL_BCAM_GET_SHADING_INQUIRY
 *  Query shading modes.
 * \return #ResGetShadingMode
 */
#define IOCTL_BCAM_GET_SHADING_INQUIRY \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetShadingInquiry, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Result of the GetShadingMode function
typedef BcamShadingInq ResGetShadingInq; ///< Specifies the shading mode

/**
 * \brief IOCTL_BCAM_UPLOAD_SHADING_DATA
 *  Upload the shading data to the BCAM device.
 * \param arg : PVOID; Pointer to the shading data buffer
 * \return None
 */
#define IOCTL_BCAM_UPLOAD_SHADING_DATA \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdUploadShadingData, METHOD_BUFFERED, FILE_ANY_ACCESS)


/**
 * \brief IOCTL_BCAM_FLUSH
 *  Abort all operations that are not yet completed.
 * \return None
 */
#define IOCTL_BCAM_FLUSH \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdFlush, METHOD_NEITHER, FILE_ANY_ACCESS)

 
/**
 * \brief IOCTL_BCAM_GET_DCAM_VERSION
 *  Get the implemented DCAM version of the camera.
 * \return ResGetDcamVersion
 */
#define IOCTL_BCAM_GET_DCAM_VERSION \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetDcamVersion, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Result of the GetDcamVersion function
typedef ULONG ResGetDcamVersion;


/**
 * \brief IOCTL_BCAM_GET_BASIC_FUNC_INQ
 *  Return the contents of the camera's BASIC_FUNC_INQ register.
 * \return ResGetBasicFuncInq
 */
#define IOCTL_BCAM_GET_BASIC_FUNC_INQ \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetBasicFuncInq, METHOD_BUFFERED, FILE_ANY_ACCESS)
/// Result of the GetDcamVersion function
typedef ULONG ResGetBasicFuncInq;


/**
 * \brief IOCTL_BCAM_SET_USER_THREAD_PRIORITY
 *  Set the priority of the calling thread.
 * \param arg ArgSetUserThreadPriority
 * \return None
 */
#define IOCTL_BCAM_SET_USER_THREAD_PRIORITY \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdSetUserThreadPriority, METHOD_BUFFERED, FILE_ANY_ACCESS)
typedef ULONG ArgSetUserThreadPriority;


/**
 * \brief IOCTL_BCAM_SET_QUEUE_SERVER_PRIORITY
 *  Set the priority of the driver's internal queue server thread.
 * \param arg ArgSetQueueServerPriority
 * \return ResSetQueueServerPriority
 */
#define IOCTL_BCAM_SET_QUEUE_SERVER_PRIORITY \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdSetQueueServerPriority, METHOD_BUFFERED, FILE_ANY_ACCESS)
typedef ULONG ArgSetQueueServerPriority;
typedef ULONG ResSetQueueServerPriority;


/**
 * \brief IOCTL_BCAM_GET_QUEUE_SERVER_PRIORITY
 *  Get the priority of the driver's internal queue server thread.
 * \param arg None
 * \return ResSetQueueServerPriority
 */
#define IOCTL_BCAM_GET_QUEUE_SERVER_PRIORITY \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetQueueServerPriority, METHOD_BUFFERED, FILE_ANY_ACCESS)
typedef ULONG ResGetQueueServerPriority;



/**
 * \brief IOCTL_BCAM_GET_AUTH_CHALLENGE
 * Get a challenge value for the challenge/response authetication protocol
 * \param arg String
 * \return ResGetAuthChallenge
 */
#define IOCTL_BCAM_GET_AUTH_CHALLENGE \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdGetAuthChallenge, METHOD_BUFFERED, FILE_ANY_ACCESS)
typedef MD5Sum ResGetAuthChallenge;


/**
 * \brief IOCTL_BCAM_AUTHORIZE
 * \param arg ArgAuthorize
 * \return None
 */
#define IOCTL_BCAM_AUTHORIZE \
  _CTL_CODE(FILE_DEVICE_BCAM_1394, OrdAuthorize, METHOD_BUFFERED, FILE_ANY_ACCESS)
typedef MD5Sum ArgAuthorize;


/* Combined argument types for all driver functions */
typedef union
{
  ArgGetVideoModeMask GetVideoModeMask;
  ArgGetFrameRateMask GetFrameRateMask;
  ArgSetVideoFormatModeFrameRate SetVideoFormatModeFrameRate;
  ArgGetString GetString;
  ArgFmt7GetModeDescriptor GetFmt7Mode;
  ArgFmt7SetAOI SetAOI;
  ArgGetBandwidthInfo GetBwInfo;
  ArgFmt7GetColorMode GetColorMode;
  ArgFmt7SetColorMode SetColorMode;
  ArgCameraControl CameraControl;
  ArgQueryDCSFeatureInq QueryDCSFeature;
  ArgSetDCSFeatureCSR SetDCSFeature;
  ArgGetDCSFeatureCSR GetDCSFeature;
  ArgAllocateResources AllocateResources;
  ArgSetCameraBytesPerPacket SetBytesPerPacket;
  ArgSetTestImageCSR SetTestImageCSR;
  ArgSetShadingCtrl SetShadingCtrl;
  ArgSetUserThreadPriority SetUserThreadPriority;
  ArgSetQueueServerPriority SetQueueServerPriority;
  ArgAuthorize Authorize;
} BcamArg;

/* Combined result types for all driver functions */
typedef union
{
  ResGetVideoFormatMask VideFormatMask;
  ResGetVideoModeMask VideoModeMask;
  ResGetFrameRateMask FrameRateMask;
  ResGetCameraID CameraID;
  ResFmt7GetModeDescriptor Fmt7Mode;
  ResGetBandwidthInfo BwInfo;
  ResAllocateResources Res;
  ResFmt7GetColorMode ColorMode;
  ResQueryDCSFeatureInq DCSFeature;
  ResQueryDCSFeaturePresence DCSFeaturePresence;
  ResGetDCSFeatureCSR DCSFeatureCSR;
  ResGetVideoFormatModeFrameRate GetVideoFormatModeFrameRate;
  ResGetTestImageCSR GetTestImageCSR;
  ResGetShadingInq GetShadingInq;
  ResGetShadingCtrl GetShadingCtrl;
  ResGetDcamVersion GetDcamVersion;
  ResGetBasicFuncInq GetBasicFuncInq;
  ResSetQueueServerPriority SetQueueServerPriority;
  ResGetQueueServerPriority GetQueueServerPriority;
  ResGetAuthChallenge AuthChallenge;
} BcamResult;

#pragma pack(pop)
#endif /* !defined(FUNCTIONS_H) */

/**
 * Device class GUIDs. These are used to identify device interfaces. GUID definitions must
 * be outside the section protected from multiple inclusion, for reasons explained in the
 * DDK docs (but not easily understood).
 */
#include <bcam_guid.h>

