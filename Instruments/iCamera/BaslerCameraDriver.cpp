/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Justin Eskesen & others at MIT Sea Grant.
    contact: jge@mit.edu

    This file is part of SGMOOS.

    SGMOOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SGMOOS.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/
// BaslerCameraDriver.cpp: implementation of the CBaslerCameraDriver class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include "BaslerCameraDriver.h"
#include "BvcColorConverter.h"
#include "Bcam.h"

const CColorConverter::PatternOrigin_t BAYER_HOME = CColorConverter::poGB;
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaslerCameraDriver::CBaslerCameraDriver()
{
	m_VideoFormat	= DCS_Format2;
	m_VideoMode		= DCS_Mode2;
	m_FrameRate		= DCS_15fps;
	m_ImageSize		= CSize(1280, 960);
	
	//timing of the strobe is limited to 1 flash every 2 seconds
	m_dfMinPeriod	= 2.0;//sec
	m_dfFrequency	= 1 / m_dfMinPeriod;//Hz
	m_dfCurrentPeriod = m_dfMinPeriod;
	
}


CBaslerCameraDriver::~CBaslerCameraDriver()
{
	try
	{
		m_Bcam.Close();
		m_DibPtr.Release();
	}
	catch( BcamException &e )	
	{
		CString Buffer, B;
		Buffer += (B.Format("Exception 0x%X occurred\n", e.Error() ), B);
		Buffer += (B.Format("Message = %s\n", e.Description() ), B);
		Buffer += (B.Format("Context = %s\n", e.Context()), B);
		
		MOOSTrace( (string)Buffer );
	}

}


bool CBaslerCameraDriver::Initialise()
{
	//invoke base class
	CCameraDriver::Initialise();

	if (CBcam::DeviceNames().size() == 0)
	{
		MOOSTrace("No camera present!\n");
		
		return !this->AddNotification("CAMERA_ERROR", "Camera not loaded");
	}
	else
	{	
		// Get the devicename of the first camera, 
		//we will only have 1 on the vehicle, so don't need to check vendor id, etc
		m_DeviceName = *(CBcam::DeviceNames().begin());
	}

	try
	{
		//open the driver
		m_Bcam.Open(m_DeviceName);
		//read the mode from the file
		string sRGBMode;
		m_pMissionFileReader->GetConfigurationParam("RGBMODE", sRGBMode);
		m_nRGBMode = ParseMode(sRGBMode);
		
		//set the color mode
		SetMode( m_nRGBMode );
		//DCAM cameras have a range of features
		InitFeatureSet();
	}
	catch( BcamException &e )	
	{
		
		CString Buffer, B;
		Buffer += (B.Format("Exception 0x%X occurred\n", e.Error() ), B);
		Buffer += (B.Format("Message = %s\n", e.Description() ), B);
		Buffer += (B.Format("Context = %s\n", e.Context()), B);
		
		MOOSTrace( (string)Buffer );
		return !this->AddNotification("CAMERA_ERROR", Buffer);	
	}

	return true;
}

bool CBaslerCameraDriver::Save()
{
	//call base class
	CCameraDriver::Save();

	//directory info
	char pFileName[256];
	char pDirPath[128];
	
	sprintf(pDirPath, "%s", m_sCameraPath.c_str());

	if( m_nFrames < 100 )
		sprintf(pFileName,"%sframe_0%d.bmp",pDirPath, m_nFrames);
	else
		sprintf(pFileName,"%sframe_%d.bmp",pDirPath, m_nFrames);

	//make the directory
	CreateDirectory(pDirPath, NULL);

	//allocate for a color shot
	CDibPtr pNewDib;

	switch( m_nMode )
	{
	case EYUV:
		//create the space for the destination ptr
		//note 24bit pointer is made, with BottomUp to get proper vertical orientation
		pNewDib.Create(m_ImageSize, 24, CDib::BottomUp, CDib::Color);
		//call utility method to convert
		CColorConverter::ConvertYUV422ToRGB(pNewDib, (BYTE*)m_DibPtr->GetPixels());
		pNewDib->StoreBMP( pFileName );
		break;
	
	case ECOLOR:
		//create the space for the destination ptr
		pNewDib.Create(m_ImageSize, 8, CDib::TopDown, CDib::Color);
		//call utility method to convert
		CColorConverter::ConvertMono8ToRGB(pNewDib, m_DibPtr, BAYER_HOME);
		pNewDib->StoreBMP( pFileName );
		break;
	
	case EMONO:
		//store the 8bit image
		m_DibPtr->StoreBMP( pFileName );
		break;
	}
	return true;
}


bool CBaslerCameraDriver::Grab()
{
	//call base class
	CCameraDriver::Grab();

	try
	{
		switch( m_nMode )
		{
		case ESPECIAL:
		case ECOLOR:
		case EMONO:
			//m_Bcam.GrabImageAsync((char*)m_DibPtr->GetPixels(), m_ImageSize.cx * m_ImageSize.cy, (char*)m_DibPtr->GetPixels(), false);  
			m_Bcam.GrabImage((char*)m_DibPtr->GetPixels(), m_ImageSize.cx * m_ImageSize.cy, 2000);
			break;
		case EYUV:
			m_Bcam.GrabImage((char*)m_DibPtr->GetPixels(), 2 * m_ImageSize.cx * m_ImageSize.cy, 2000);
			//m_Bcam.GrabImageAsync((char*)m_DibPtr->GetPixels(), 2 * m_ImageSize.cx * m_ImageSize.cy, (char*)m_DibPtr->GetPixels(), false);
			break;
		}
	}
	catch( BcamException &e )	
	{
		
		CString Buffer, B;
		
		Buffer += (B.Format("Exception 0x%X occurred\n", e.Error() ), B);
		
		Buffer += (B.Format("Message = %s\n", e.Description() ), B);
		
		Buffer += (B.Format("Context = %s\n", e.Context()), B);
		
		MOOSTrace( (string)Buffer );
		
		return this->AddNotification("CAMERA_ERROR", Buffer);
	}

	return true;
}

/**
*The supported modes for the A102fc Basler IEEE 1394 camera are:
*Format_2, Mode_0, FrameRate_1 (YUV 4:2:2, 16 bits/pixel ave, 1280x960 @ 7.5fps
*Format_2, Mode_2, FrameRate_3 (Raw, 8 bits/pixel, 1280x960 @ 15fps)
*/
bool CBaslerCameraDriver::SetMode(int nMode)
{
	switch( nMode )
	{
	case ECOLOR:
	case EMONO:
		m_VideoFormat	= DCS_Format2;
		m_VideoMode		= DCS_Mode2;
		m_FrameRate		= DCS_15fps;
		m_ImageSize		= CSize(1280, 960);
		try
		{
			m_Bcam.SetVideoMode(m_VideoFormat, m_VideoMode, m_FrameRate);
			// Allocate Resources (MaxBuffers, MaxBufferSize)
			m_Bcam.AllocateResources(1, m_ImageSize.cx * m_ImageSize.cy);
			//set continuous shot
			m_Bcam.ContinuousShot = false;
			//create the space for a Device Independent Bitmap pointer
			m_DibPtr.Create(m_ImageSize, 8, CDib::TopDown, CDib::Monochrome);
		}
		catch( BcamException &e )	
		{
			
			CString Buffer, B;
			Buffer += (B.Format("Exception 0x%X occurred\n", e.Error() ), B);
			Buffer += (B.Format("Message = %s\n", e.Description() ), B);
			Buffer += (B.Format("Context = %s\n", e.Context()), B);
			
			MOOSTrace( (string)Buffer );
			this->AddNotification("CAMERA_ERROR", Buffer);
		}

		break;
	case EYUV:
		m_VideoFormat	= DCS_Format2;
		m_VideoMode		= DCS_Mode0;
		m_FrameRate		= DCS_7_5fps;
		m_ImageSize		= CSize(1280, 960);//16bits/pixel if YUV
		try
		{
			m_Bcam.SetVideoMode(m_VideoFormat, m_VideoMode, m_FrameRate);
			//note the 2*ImageSize
			m_Bcam.AllocateResources(1, 2 * m_ImageSize.cx * m_ImageSize.cy);
			//set continuous shot
			m_Bcam.ContinuousShot = false;
			//create the space for a Device Independent Bitmap pointer
			m_DibPtr.Create(m_ImageSize, 16, CDib::TopDown, CDib::Color);
		}
		catch( BcamException &e )	
		{
			
			CString Buffer, B;
			Buffer += (B.Format("Exception 0x%X occurred\n", e.Error() ), B);
			Buffer += (B.Format("Message = %s\n", e.Description() ), B);
			Buffer += (B.Format("Context = %s\n", e.Context()), B);
			
			MOOSTrace( (string)Buffer );
			this->AddNotification("CAMERA_ERROR", Buffer);
		}
		

		break;
	case ESPECIAL:
		m_VideoFormat	= DCS_Format7;
		m_VideoMode		= DCS_Mode0;
		m_ColorCode		= DCSColor_Mono8;
		
		m_ImageSize		= m_Bcam.FormatSeven[m_VideoMode].MaxSize();
		m_ImageSize.cx  = m_ImageSize.cx & ~3; // Beware : Windows Bitmaps have to have a DWORD alligned width :-(
		CPoint AoiPosition(0,0);
		CSize AoiSize(m_ImageSize);
		m_Bcam.FormatSeven[m_VideoMode].Position = AoiPosition;
		m_Bcam.FormatSeven[m_VideoMode].Size = AoiSize;
		
		//create the space for a Device Independent Bitmap pointer
		m_DibPtr.Create(m_ImageSize, 8, CDib::TopDown, CDib::Monochrome);

		break;
	}

	//keep track of the mode we're in
	m_nMode = nMode;
	
	return true;
}

bool CBaslerCameraDriver::GetStatus()
{
	return true;
}

/**default mode is B/W mono*/
int CBaslerCameraDriver::ParseMode(string &sMode)
{
	if( sMode == "COLOR" )
	{
		MOOSTrace("Basler Mode: COLOR\n");
		return this->ERGBMode::ECOLOR;
	}
	else if( sMode == "MONO" )
	{
		MOOSTrace("Basler Mode: MONO\n");
		return this->ERGBMode::EMONO;
	}
	else if ( sMode == "SPECIAL" )
	{
		MOOSTrace("Basler Mode: Format7 SPECIAL\n");
		return this->ERGBMode::ESPECIAL;
	}
	else if ( sMode == "YUV" )
	{
		MOOSTrace("Basler Mode: YUV\n");
		return this->ERGBMode::EYUV;
	}
	else
		return this->ERGBMode::EMONO;
}

bool CBaslerCameraDriver::InitFeatureSet()
{
	//read the feature from the .moos file
	string sFeature;
	if( m_pMissionFileReader->GetConfigurationParam("GAIN", sFeature) )
		SetGain( atoi(sFeature.c_str()) );

	sFeature = "";
	if( m_pMissionFileReader->GetConfigurationParam("SHUTTER", sFeature) )
		SetShutter( atoi(sFeature.c_str()) );

	sFeature = "";
	if( m_pMissionFileReader->GetConfigurationParam("BRIGHTNESS", sFeature) )
		SetBrightness( atoi(sFeature.c_str()) );

	sFeature = "";
	if( m_pMissionFileReader->GetConfigurationParam("WHITEBALANCERED", sFeature) )
		SetWhiteBalanceRed( atoi(sFeature.c_str()) );

	sFeature = "";
	if( m_pMissionFileReader->GetConfigurationParam("WHITEBALANCEBLUE", sFeature) )
		SetWhiteBalanceBlue( atoi(sFeature.c_str()) );


	return true;
}

void CBaslerCameraDriver::SetShutter(unsigned long ulShutter)
{

	if( m_Bcam.Shutter.IsSupported() )
	{
		MOOSTrace("Shutter Supported: ");
		
		unsigned long Min = m_Bcam.Shutter.Raw.Min();
		unsigned long Max = m_Bcam.Shutter.Raw.Max();
		
		if( ulShutter < Min )
		{
			m_Bcam.Shutter.Raw = Min;
		}
		else if( ulShutter > Max )
		{
			m_Bcam.Shutter.Raw = Max;
		}
		else
			m_Bcam.Shutter.Raw = ulShutter;

		this->AddNotification("CAMERA_SHUTTER", GetShutter() );
		MOOSTrace( "%d\n", GetShutter() );
	}
	else
		this->AddNotification("CAMERA_ERROR", "Feature: SHUTTER not supported");
}
	
void CBaslerCameraDriver::SetGain(unsigned long ulGain)
{

	if( m_Bcam.Gain.IsSupported() )
	{
		MOOSTrace("Gain Supported: ");
		
		unsigned long Min = m_Bcam.Gain.Raw.Min();
		unsigned long Max = m_Bcam.Gain.Raw.Max();
		
		if( ulGain < Min )
		{
			m_Bcam.Gain.Raw = Min;
		}
		else if( ulGain > Max )
		{
			m_Bcam.Gain.Raw = Max;
		}
		else
			m_Bcam.Gain.Raw = ulGain;

		this->AddNotification("CAMERA_GAIN", GetGain() );
		MOOSTrace( "%d\n", GetGain() );
	}
	else
		this->AddNotification("CAMERA_ERROR", "Feature: GAIN not supported");
}

void CBaslerCameraDriver::SetBrightness(unsigned long ulBrightness)
{

	if( m_Bcam.Brightness.IsSupported() )
	{
		MOOSTrace("Brightness Supported: ");
		
		unsigned long Min = m_Bcam.Brightness.Raw.Min();
		unsigned long Max = m_Bcam.Brightness.Raw.Max();
		
		if( ulBrightness < Min )
		{
			m_Bcam.Brightness.Raw = Min;
		}
		else if( ulBrightness > Max )
		{
			m_Bcam.Brightness.Raw = Max;
		}
		else
			m_Bcam.Brightness.Raw = ulBrightness;

		this->AddNotification("CAMERA_BRIGHTNESS", GetBrightness() );
		MOOSTrace( "%d\n", GetBrightness() );
	}
	else
		this->AddNotification("CAMERA_ERROR", "Feature: Brightness not supported");
}

void CBaslerCameraDriver::SetWhiteBalanceRed(unsigned long ulWhiteBalanceRed)
{

	if( m_Bcam.WhiteBalance.IsSupported() )
	{
		MOOSTrace("WhiteBalance Supported: ");
		
		unsigned long Min = m_Bcam.WhiteBalance.Raw.Min();
		unsigned long Max = m_Bcam.WhiteBalance.Raw.Max();
		
		if( ulWhiteBalanceRed < Min )
		{
			m_Bcam.WhiteBalance.Raw.VRValue = Min;
		}
		else if( ulWhiteBalanceRed > Max )
		{
			m_Bcam.WhiteBalance.Raw.VRValue = Max;
		}
		else
			m_Bcam.WhiteBalance.Raw.VRValue = ulWhiteBalanceRed;

		this->AddNotification("CAMERA_WBRED", GetWhiteBalanceRed() );
		MOOSTrace( "[RED] %d\n", GetWhiteBalanceRed() );
	}
	else
		this->AddNotification("CAMERA_ERROR", "Feature: WhiteBalance not supported");
}

void CBaslerCameraDriver::SetWhiteBalanceBlue(unsigned long ulWhiteBalanceBlue)
{

	if( m_Bcam.WhiteBalance.IsSupported() )
	{
		MOOSTrace("WhiteBalance Supported: ");
		
		unsigned long Min = m_Bcam.WhiteBalance.Raw.Min();
		unsigned long Max = m_Bcam.WhiteBalance.Raw.Max();
		
		if( ulWhiteBalanceBlue < Min )
		{
			m_Bcam.WhiteBalance.Raw.UBValue = Min;
		}
		else if( ulWhiteBalanceBlue > Max )
		{
			m_Bcam.WhiteBalance.Raw.UBValue = Max;
		}
		else
			m_Bcam.WhiteBalance.Raw.UBValue = ulWhiteBalanceBlue;

		this->AddNotification("CAMERA_WBBLUE", GetWhiteBalanceBlue() );
		MOOSTrace( "[BLUE] %d\n", GetWhiteBalanceBlue() );
	}
	else
		this->AddNotification("CAMERA_ERROR", "Feature: WhiteBalance not supported");
}

unsigned long CBaslerCameraDriver::GetGain()
{
	return m_Bcam.Gain.Raw();
}

unsigned long CBaslerCameraDriver::GetShutter()
{
	return m_Bcam.Shutter.Raw();
}

unsigned long CBaslerCameraDriver::GetBrightness()
{
	return m_Bcam.Brightness.Raw();
}

unsigned long CBaslerCameraDriver::GetWhiteBalanceBlue()
{
	return m_Bcam.WhiteBalance.Raw.UBValue();
}

unsigned long CBaslerCameraDriver::GetWhiteBalanceRed()
{
	return m_Bcam.WhiteBalance.Raw.VRValue();
}

