/************************************************************************
*						   NUI Context Methods							*
*																		*
*						  Author: Andrew DeVine							*
*								  2013									*
************************************************************************/

#include "stdafx.h"
#include "NuiContext.h"

#define PI 3.14159265
#define PLAYER_ONE 254
#define PLAYER_TWO 255

// Zeroes members of context object
void NuiContext::Nui_Zero()
{
	m_hNextColorFrameEvent = NULL;
	m_hNextDepthFrameEvent = NULL;
	m_hVideoStream = NULL;
	m_hDepthStream = NULL;
	m_pSensor = NULL;
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
			m_pSensor = tempSensor;
			break;
		}

		tempSensor->Release();
	}
    
	if (m_pSensor != NULL)
	{
		hr = m_pSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON|NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_DEPTH);
		if (FAILED(hr))
		{
			return hr;
		}

		m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
		m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

		hr = m_pSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR,NUI_IMAGE_RESOLUTION_640x480,0,2,m_hNextColorFrameEvent,&m_hVideoStream);
		if (FAILED(hr))
		{
			return hr;
		}

		hr = m_pSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH,NUI_IMAGE_RESOLUTION_640x480,0,2,m_hNextDepthFrameEvent,&m_hDepthStream);
		if (FAILED(hr))
		{
			return hr;
		}

		m_pSensor->NuiCameraElevationGetAngle(&angle);
	}

	m_pSensor->NuiCameraElevationGetAngle(&angle);

	return S_OK;
}


// Stops Sensor Polling
void NuiContext::Nui_UnInit()
{
    CloseHandle( m_hNextColorFrameEvent );
    m_hNextColorFrameEvent = NULL;

    CloseHandle( m_hNextDepthFrameEvent );
    m_hNextDepthFrameEvent = NULL;

	m_pSensor->NuiSkeletonTrackingDisable();
	m_pSensor->NuiShutdown();
	m_pSensor->Release();

	Nui_Zero();
}


// Polls for new Skeleton Frame
HRESULT NuiContext::PollNextSkeletonFrame() {

	HRESULT hr;
	NUI_SKELETON_FRAME skeletonFrame;

	hr = m_pSensor->NuiSkeletonGetNextFrame(100, &skeletonFrame);
	if (FAILED(hr))
	{
		return hr;
	}

	m_pSensor->NuiTransformSmooth(&skeletonFrame, NULL);

	for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
    {
		//Player 1
		if(skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED && (int)skeletonFrame.SkeletonData[i].dwUserIndex == PLAYER_ONE)
        {
			skData = skeletonFrame.SkeletonData[i];

			if(!twoPlayer) {
				return S_OK;
			}
        }

		//Player 2
		if(twoPlayer) {
			if(skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED && (int)skeletonFrame.SkeletonData[i].dwUserIndex == PLAYER_TWO)
			{
				skData2 = skeletonFrame.SkeletonData[i];
				return S_OK;
			}
		}
    }

	return E_FAIL;
}


// Polls for new Image Frame
HRESULT NuiContext::PollNextImageFrame()
{
	HRESULT hr;
	NUI_IMAGE_FRAME ImageFrame;
	NUI_LOCKED_RECT LockedRect;

	hr = m_pSensor->NuiImageStreamGetNextFrame(m_hVideoStream, 100, &ImageFrame);

	if ( !FAILED(hr) ) {
		INuiFrameTexture* pTexture = ImageFrame.pFrameTexture;
		pTexture->LockRect(0, &LockedRect, NULL, 0);
		
		texture = LockedRect.pBits;
		tsize = LockedRect.size;
		
		pTexture->UnlockRect(0);
		m_pSensor->NuiImageStreamReleaseFrame(m_hVideoStream, &ImageFrame);
	}

	return hr;
}


// Polls for new Depth Frame
HRESULT NuiContext::PollNextDepthFrame()
{
	HRESULT hr;
	NUI_IMAGE_FRAME DepthFrame;
	NUI_LOCKED_RECT LockedRect;

	hr = m_pSensor->NuiImageStreamGetNextFrame(m_hDepthStream, 100, &DepthFrame);

	if ( !FAILED(hr) ) {
		INuiFrameTexture* pTexture = DepthFrame.pFrameTexture;
		pTexture->LockRect(0, &LockedRect, NULL, 0);
		
		depthframe = LockedRect.pBits;
		dsize = LockedRect.size;
		
		pTexture->UnlockRect(0);
		m_pSensor->NuiImageStreamReleaseFrame(m_hDepthStream, &DepthFrame);
	}

	return hr;
}


// Set camera angle
HRESULT NuiContext::SetCameraAngle(long angle) {

	return m_pSensor->NuiCameraElevationSetAngle(angle);
}


// Transform coordinates from camera view space to world space
void NuiContext::TransformCoordinates(OUT KUVector4* skTrans) {

	KUVector4 &in = *skTrans;

	in.x = in.x;
	in.y = (in.y * cos((float)angle * (PI / 180))) + (in.z * sin((float)angle * (PI / 180)));
	in.z = (-in.y * sin((float)angle * (PI / 180))) + (in.z * cos((float)angle * (PI / 180)));
}