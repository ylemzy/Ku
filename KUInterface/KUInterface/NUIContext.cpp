/************************************************************************
*						   NUI Context Methods							*
*																		*
*						  Author: Andrew DeVine							*
*								  2013									*
************************************************************************/

#include "stdafx.h"
#include "NuiContext.h"
#include <KinectBackgroundRemoval.h>
#include <float.h>

#define PI 3.14159265
#define PLAYER_ONE 254
#define PLAYER_TWO 255

NuiContext::NuiContext()
	: m_pNuiSensor(NULL)
	, m_pBackgroundRemovalStream(NULL)
	, m_bNearMode(TRUE)
	, m_skTackedId(NUI_SKELETON_INVALID_TRACKING_ID)
{
	DWORD width = 0;
	DWORD height = 0;
	NuiImageResolutionToSize(cColorResolution, width, height);

	m_colorWidth = (LONG)width;
	m_colorHeight = (LONG)height;

	NuiImageResolutionToSize(cDepthResolution, width, height);
	m_depthWidth = (LONG)width;
	m_depthHeight = (LONG)height;
	////////////////////////////////
}

// Zeroes members of context object
void NuiContext::Nui_Zero()
{
	m_hNextColorFrameEvent = NULL;
	m_hNextDepthFrameEvent = NULL;
	m_hNextSkeletonFrameEvent = NULL;
	m_hNextBackgroundRemovedFrameEvent = NULL;
	m_pColorStreamHandle = NULL;
	m_pDepthStreamHandle = NULL;
	m_pNuiSensor = NULL;
	angle = 0;
}


// Initializes Sensor for Polling
HRESULT NuiContext::Nui_Init()
{
	
	HRESULT hr;
	int sensorCount;
	INuiSensor* tempSensor;

	NuiGetSensorCount(&sensorCount);

	Nui_Zero();

	for (int i = 0; i < sensorCount; ++i)
	{
		hr = NuiCreateSensorByIndex(i, &tempSensor);
		if (FAILED(hr))
		{
			continue;
		}

		hr = tempSensor->NuiStatus();
		if (hr == S_OK)
		{
			m_pNuiSensor = tempSensor;
			break;
		}

		tempSensor->Release();
	}

	if (m_pNuiSensor == NULL)
		return CREATE_SENSOR_ERROR;


	hr = m_pNuiSensor->NuiInitialize(
		NUI_INITIALIZE_FLAG_USES_COLOR |
		NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX |
		NUI_INITIALIZE_FLAG_USES_SKELETON);

	
	m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hNextSkeletonFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hNextBackgroundRemovedFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (SUCCEEDED(hr) && NULL != m_pNuiSensor)
	{
		// Open a depth image stream to receive depth frames
		hr = m_pNuiSensor->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
			cDepthResolution,
			0,
			2,
			m_hNextDepthFrameEvent,
			&m_pDepthStreamHandle);
		if (SUCCEEDED(hr))
		{
			// Open a color image stream to receive color frames
			hr = m_pNuiSensor->NuiImageStreamOpen(
				NUI_IMAGE_TYPE_COLOR,
				cColorResolution,
				0,
				2,
				m_hNextColorFrameEvent,
				&m_pColorStreamHandle);

			if (SUCCEEDED(hr))
			{
				hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonFrameEvent, NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE);
			}
		}
	}
	else
	{
		ResetEvent(m_hNextDepthFrameEvent);
		ResetEvent(m_hNextColorFrameEvent);
		ResetEvent(m_hNextSkeletonFrameEvent);
	}

	hr = CreateBackgroundRemovedColorStream();

	//m_pNuiSensor->NuiCameraElevationGetAngle(&angle);
	return hr;
}


// Stops Sensor Polling
void NuiContext::Nui_UnInit()
{
	if (m_hNextColorFrameEvent)
		CloseHandle( m_hNextColorFrameEvent );

	if (m_hNextDepthFrameEvent)
		CloseHandle( m_hNextDepthFrameEvent );

	if (m_hNextBackgroundRemovedFrameEvent)
		CloseHandle(m_hNextBackgroundRemovedFrameEvent);

	if (m_pBackgroundRemovalStream)
	{
		m_pBackgroundRemovalStream->Disable();
		m_pBackgroundRemovalStream->Release();
		m_pBackgroundRemovalStream = NULL;
	}

	if (m_pNuiSensor)
	{
		m_pNuiSensor->NuiSkeletonTrackingDisable();
		m_pNuiSensor->NuiShutdown();
		m_pNuiSensor->Release();
	}

	Nui_Zero();
}


// Polls for new Skeleton Frame
HRESULT NuiContext::ProcessSkeleton() {

	HRESULT hr;

	NUI_SKELETON_FRAME skeletonFrame;
	hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
	if (FAILED(hr))
	{
		return hr;
	}

	NUI_SKELETON_DATA* pSkeletonData = skeletonFrame.SkeletonData;

	float closestSkeletonDistance = FLT_MAX;
	int candidateId = -1;
	bool isTrackedSkVisible = false;
	for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
	{
		NUI_SKELETON_DATA skeleton = pSkeletonData[i];
		if(skeleton.eTrackingState == NUI_SKELETON_TRACKED)
		{
			if (m_skTackedId == skeleton.dwTrackingID)
			{
				candidateId = i;
				break;
			}
			else if (skeleton.Position.z < closestSkeletonDistance)
			{
				candidateId = i;
				closestSkeletonDistance = skeleton.Position.z;
			}
		}
	}

	if (0 <= candidateId && candidateId < NUI_SKELETON_COUNT)
	{
		skData = pSkeletonData[candidateId];
		if (m_skTackedId != skData.dwTrackingID && skData.dwTrackingID != NUI_SKELETON_INVALID_TRACKING_ID)
		{
			m_skTackedId = skData.dwTrackingID;

			hr = m_pBackgroundRemovalStream->SetTrackedPlayer(skData.dwTrackingID);
			if (FAILED(hr))
			{
				CHECK_ERROR(SET_TRACKEDPLAYER_ERROR);
			}
		}
	}
	else
	{
		m_skTackedId = NUI_SKELETON_INVALID_TRACKING_ID;
	}

	hr = m_pBackgroundRemovalStream->ProcessSkeleton(NUI_SKELETON_COUNT, pSkeletonData, skeletonFrame.liTimeStamp);
	if (FAILED(hr))
	{
		CHECK_ERROR(BACKGROUND_REMOVED_PROCESS_SKELETON_ERROR);
	}

	return SUCCEEDED_OK;
}


// Polls for new Image Frame
HRESULT NuiContext::ProcessColor()
{
	HRESULT hr;
	HRESULT bghr = S_OK;

	NUI_IMAGE_FRAME imageFrame;

	// Attempt to get the depth frame
	LARGE_INTEGER colorTimeStamp;
	hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pColorStreamHandle, 0, &imageFrame);
	if (FAILED(hr))
	{
		return hr;
	}
	colorTimeStamp = imageFrame.liTimeStamp;

	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data. Then save a copy of color frame.
	if (LockedRect.Pitch != 0)
	{
		bghr = m_pBackgroundRemovalStream->ProcessColor(m_colorWidth * m_colorHeight * cBytesPerPixel, LockedRect.pBits, colorTimeStamp);
	}

	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);

	// Release the frame
	hr = m_pNuiSensor->NuiImageStreamReleaseFrame(m_pColorStreamHandle, &imageFrame);

	if (FAILED(bghr))
	{
		return bghr;
	}

	return hr;
	return SUCCEEDED_OK;
}


// Polls for new Depth Frame
HRESULT NuiContext::ProcessDepth()
{
	HRESULT hr;
	HRESULT bghr = S_OK;
	NUI_IMAGE_FRAME imageFrame;

	// Attempt to get the depth frame
	LARGE_INTEGER depthTimeStamp;
	hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
	if (FAILED(hr))
	{
		return hr;
	}
	depthTimeStamp = imageFrame.liTimeStamp;
	INuiFrameTexture* pTexture;

	// Attempt to get the extended depth texture
	hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(m_pDepthStreamHandle, &imageFrame, &m_bNearMode, &pTexture);
	if (FAILED(hr))
	{
		return hr;
	}
	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data, and then present it to the background removed color stream. 
	USHORT* uData = reinterpret_cast<USHORT*>(LockedRect.pBits);
	for (int i = 0; i < LockedRect.size / 2; i += 2)
	{
		USHORT tmp = uData[i];
		if (uData[i])
		{
			int a = 0;
			a++;
		}
	}
	if (LockedRect.Pitch != 0)
	{
		bghr = m_pBackgroundRemovalStream->ProcessDepth(m_depthWidth * m_depthHeight * cBytesPerPixel, LockedRect.pBits, depthTimeStamp);
	}

	// We're done with the texture so unlock it. Even if above process failed, we still need to unlock and release.
	pTexture->UnlockRect(0);
	pTexture->Release();

	// Release the frame
	hr = m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);

	if (FAILED(bghr))
	{
		return bghr;
	}

	return hr;
}


// Set camera angle
HRESULT NuiContext::SetCameraAngle(long angle) {

	return m_pNuiSensor->NuiCameraElevationSetAngle(angle);
}


// Transform coordinates from camera view space to world space
void NuiContext::TransformCoordinates(OUT KUVector4* skTrans) {

	KUVector4 &in = *skTrans;

	in.x = in.x;
	in.y = (in.y * cos((float)angle * (PI / 180))) + (in.z * sin((float)angle * (PI / 180)));
	in.z = (-in.y * sin((float)angle * (PI / 180))) + (in.z * cos((float)angle * (PI / 180)));
}

HRESULT NuiContext::ProcessBackgroundRemoved()
{

	HRESULT hr = S_FALSE;
	NUI_BACKGROUND_REMOVED_COLOR_FRAME bgRemovedFrame = {0};
	hr = m_pBackgroundRemovalStream->GetNextFrame(0, &bgRemovedFrame);
	if (FAILED(hr))
		return SUCCEEDED_FALSE;

	backgroundRemovedTexture = bgRemovedFrame.pBackgroundRemovedColorData;
	backgroundRemovedSize = bgRemovedFrame.backgroundRemovedColorDataLength;

	hr = m_pBackgroundRemovalStream->ReleaseFrame(&bgRemovedFrame);
	return hr;
}

BOOL NuiContext::IsBackgroundRemovedEnabled() const
{
	return m_pBackgroundRemovalStream != NULL;
}

NuiContext::~NuiContext()
{
	Nui_UnInit();
}

HRESULT NuiContext::CreateBackgroundRemovedColorStream()
{
	HRESULT hr;
	if (NULL == m_pNuiSensor)
	{
		return E_FAIL;
	}

	hr = NuiCreateBackgroundRemovedColorStream(m_pNuiSensor, &m_pBackgroundRemovalStream);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pBackgroundRemovalStream->Enable(cColorResolution, cDepthResolution, m_hNextBackgroundRemovedFrameEvent);

	return hr;
}
