#if 0  // makefile definitions
!if 0  #nmake skips the rest of this file
#endif // end of makefile definitions
//{{NO_DEPENDENCIES}}
/*-------------------------------------------------------------------
 * Project:	Bcam
 * Purpose:	Declares the Version of the whole BCAM-Library
 *
 * This is the versioning file used by C/C++ code and make files.
 * Modifying this file to set the version is obsolete. Use 
 * BuildBcam.ver instead.
 *
 *-------------------------------------------------------------------
 * $Revision: 1.1.2.1 $
 * $Author: moosauthor $
 * 
 * Copyright (c) Basler Vision Technologies AG - All rights reserved.
 */

// Set to 1/0 the following lines as appropriate.
#define BCAM_VERSION_PRERELEASE    0
#define BCAM_VERSION_PRIVATEBUILD  0

/* Nothing below this point has to be modified to adjust version data.
 *-------------------------------------------------------------------*/

#if 0  // Ignore by C++ preprocessor
!endif // Activate nmake processor

BCAM_VERSION_MAJOR=1
BCAM_VERSION_MINOR=7
BCAM_VERSION_BUILD=0015

!if 0  #nmake skips the rest of this file
#endif

#define	BCAM_VERSION_MAJOR 1
#define	BCAM_VERSION_MINOR 7
#define BCAM_VERSION_BUILD 0015

#define	BCAM_VERSIONSTRING_MAJOR "1"
#define	BCAM_VERSIONSTRING_MINOR "7"
#define	BCAM_VERSIONSTRING_BUILD "0015"

#ifdef _DEBUG
  #define BCAM_DEBUGSTRING " (debug)"
#else
  #define BCAM_DEBUGSTRING ""
#endif

#ifndef BCAM_FILE_DESCRIPTION
  #if defined(BCAM_DRV)
    #define BCAM_FILE_DESCRIPTION(name) #name " Device Driver\0"
  #elif defined(BCAM_CPL)
    #define BCAM_FILE_DESCRIPTION(name) #name " Control Panel\0"
  #elif defined(BCAM_EXE)
    #define BCAM_FILE_DESCRIPTION(name) #name " Application\0"
  #else
    #define BCAM_FILE_DESCRIPTION(name) #name " Module\0"
  #endif
#endif

#if BCAM_VERSION_PRIVATEBUILD
  #define BCAM_FILE_FLAGS2 VS_FF_PRIVATEBUILD
#else
  #define BCAM_FILE_FLAGS2 0
#endif

#ifdef _DEBUG
  #define BCAM_FILE_FLAGS BCAM_FILE_FLAGS2|VS_FF_DEBUG
#elif BCAM_VERSION_PRERELEASE
  #define BCAM_FILE_FLAGS BCAM_FILE_FLAGS2|VS_FF_PRERELEASE
#else
  #define BCAM_FILE_FLAGS 0
#endif


#if defined(BCAM_DRV)
  #define BCAM_FILETYPE VFT_DRV
  #define BCAM_FILESUBTYPE VFT2_DRV_INSTALLABLE
  #define BCAM_ORIGINAL_FILENAME(name) #name ".sys\0"
#elif defined(BCAM_CPL)
  #define BCAM_FILETYPE VFT_DLL
  #define BCAM_FILESUBTYPE VFT2_UNKNOWN
  #define BCAM_ORIGINAL_FILENAME(name) #name ".cpl\0"
#elif defined(BCAM_EXE)
  #define BCAM_FILETYPE VFT_APP
  #define BCAM_FILESUBTYPE VFT2_UNKNOWN
  #define BCAM_ORIGINAL_FILENAME(name) #name ".exe\0"
#else
  #define BCAM_FILETYPE VFT_DLL
  #define BCAM_FILESUBTYPE VFT2_UNKNOWN
  #define BCAM_ORIGINAL_FILENAME(name) #name ".dll\0"
#endif


#define BCAM_VERSION(name, priv) \
  LANGUAGE LANG_NEUTRAL, SUBLANG_DEFAULT \
    VS_VERSION_INFO VERSIONINFO \
     FILEVERSION BCAM_VERSION_MAJOR, BCAM_VERSION_MINOR, BCAM_VERSION_BUILD \
     PRODUCTVERSION BCAM_VERSION_MAJOR, BCAM_VERSION_MINOR, BCAM_VERSION_BUILD \
     FILEFLAGSMASK VS_FFI_FILEFLAGSMASK \
     FILEFLAGS BCAM_FILE_FLAGS \
     FILEOS VOS_NT_WINDOWS32 \
     FILETYPE BCAM_FILETYPE \
     FILESUBTYPE BCAM_FILESUBTYPE \
    BEGIN \
	BLOCK "StringFileInfo" \
	BEGIN \
	    BLOCK "000004b0" \
	    BEGIN \
		VALUE "Comments", "\0" \
		VALUE "CompanyName", "Basler Vision Technologies AG\0" \
		VALUE "FileDescription", BCAM_FILE_DESCRIPTION(name) "\0" \
		VALUE "FileVersion", BCAM_VERSIONSTRING_MAJOR "." BCAM_VERSIONSTRING_MINOR "." BCAM_VERSIONSTRING_BUILD BCAM_DEBUGSTRING "\0" \
		VALUE "InternalName", #name "\0" \
		VALUE "LegalCopyright", "Copyright (c) 2001 Basler AG - All rights reserved.\0" \
		VALUE "LegalTrademarks", "\0" \
		VALUE "OLESelfRegister", "\0" \
		VALUE "OriginalFilename", BCAM_ORIGINAL_FILENAME(name) \
		VALUE "PrivateBuild", "\0" \
		VALUE "ProductName", "BCAM - The Basler Vision Toolbox\0" \
		VALUE "ProductVersion", BCAM_VERSIONSTRING_MAJOR "." BCAM_VERSIONSTRING_MINOR "." BCAM_VERSIONSTRING_BUILD BCAM_DEBUGSTRING "\0" \
		VALUE "SpecialBuild", #priv \
	    END \
	END \
	BLOCK "VarFileInfo" \
	BEGIN \
	    VALUE "Translation", 0x0, 1200 \
	END \
    END


#if 0  // required at end of source file, to hide makefile terminator
!endif // makefile terminator
#endif

