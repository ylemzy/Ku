
#include "stdafx.h"
#include "NuiContext.h"
#include <float.h>

#define PI 3.14159265
#define PLAYER_ONE 254
#define PLAYER_TWO 255
#define WAIT_FRAME_TIME 0	//经过测试，最好不超过10ms，否则导致background_removed 取不到帧

SensorContext::SensorContext()
	: m_pNuiSensor(NULL)
	, m_pBackgroundRemovalStream(NULL)
	, m_pNuiInteractionClient(NULL)
	, m_pNuiInteractionStream(NULL)
	, m_hColorStream(INVALID_HANDLE_VALUE)
	, m_hDepthStream(INVALID_HANDLE_VALUE)
	, m_hNextColorFrameEvent(INVALID_HANDLE_VALUE)
	, m_hNextDepthFrameEvent(INVALID_HANDLE_VALUE)
	, m_hNextSkeletonFrameEvent(INVALID_HANDLE_VALUE)
	, m_hNextBackgroundRemovedFrameEvent(INVALID_HANDLE_VALUE)
	, m_hNextInteractionFrameEvent(INVALID_HANDLE_VALUE)
	, m_bImageValid(FALSE)
	, m_bDepthValid(FALSE)
	, m_bSkeletonValid(FALSE)
	, m_bBackgroundRemovedValid(FALSE)
	, m_bInteractionValid(FALSE)
	, m_bNearMode(FALSE)
	, m_skTackedId(NUI_SKELETON_INVALID_TRACKING_ID)
	, m_nImageSize(0)
	, m_nDepthSize(0)
	, m_nBackgroundRemovedSize(0)
	, m_pImageData(NULL)
	, m_pDepthData(NULL)
	, m_pBackgroundRemovedData(NULL)
{
	memset(&m_skData, 0, sizeof(NUI_SKELETON_DATA));
	memset(&m_skData2, 0, sizeof(NUI_SKELETON_DATA));
	memset(&m_mainUserInfo, 0, sizeof(NUI_USER_INFO));
	DWORD width = 0;
	DWORD height = 0;
	NuiImageResolutionToSize(cColorResolution, width, height);

	m_colorWidth = (LONG)width;
	m_colorHeight = (LONG)height;

	NuiImageResolutionToSize(cDepthResolution, width, height);
	m_depthWidth = (LONG)width;
	m_depthHeight = (LONG)height;
}

//初始化，多次调用不会有产生多次重复的flag初始化
HRESULT SensorContext::InitSensor(DWORD flag, RUNTIME_RESULT* rtHr /*= 0*/)
{
	HRESULT hr = S_FALSE;
	if (!m_pNuiSensor)
	{
		UnInitSensor();//先清空数据
		if (FAILED(hr = FindSensor(rtHr)))
			return hr;
	}

	//注意保证flag中已初始化的不再初始化
	DWORD oldFlag = m_pNuiSensor->NuiInitializationFlags();
	HRESULT hrColor = S_FALSE, hrDepth = S_FALSE, hrSkeleton = S_FALSE;
	if (oldFlag != flag)
	{
		hr = m_pNuiSensor->NuiInitialize(flag);
		if (EXIST_FLAG(flag, NUI_INITIALIZE_FLAG_USES_COLOR)
			&& NO_EXIST_FLAG(oldFlag, NUI_INITIALIZE_FLAG_USES_COLOR))
		{
			hrColor = SetEnableColor(TRUE, rtHr);
			if (FAILED(hrColor))
				return hrColor;
		}

		if (EXIST_FLAG(flag, NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX)
			&& NO_EXIST_FLAG(oldFlag, NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX))
		{
			hrDepth = SetEnableDepth(TRUE, rtHr);
			if (FAILED(hrDepth))
				return hrDepth;
			hrDepth = m_pNuiSensor->NuiImageStreamSetImageFrameFlags(m_hDepthStream, NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE);
			if (FAILED(hrDepth))
				return hrDepth;


		}

		if (EXIST_FLAG(flag, NUI_INITIALIZE_FLAG_USES_SKELETON)
			&& NO_EXIST_FLAG(oldFlag, NUI_INITIALIZE_FLAG_USES_SKELETON))
		{
			hrSkeleton = SetEnableSkeleton(TRUE, rtHr);
			if (FAILED(hrSkeleton))
				return hrSkeleton;
		}
		//m_pNuiSensor->NuiCameraElevationGetAngle(&angle);
		
	}
	return hr;
}


HRESULT SensorContext::FindSensor(RUNTIME_RESULT* rtHr /*= 0*/)
{
	HRESULT hr = S_FALSE;
	int sensorCount = 0;
	hr = NuiGetSensorCount(&sensorCount);
	INuiSensor* tempSensor = NULL;
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

	if (!tempSensor)
	{
		MAKE_RUNTIMERESULT(rtHr, CREATE_SENSOR_ERROR);
	}
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);
	return hr;
}

HRESULT SensorContext::SetEnableColor(BOOL bEnabled, RUNTIME_RESULT* rtHr)
{
	if (!m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}

	DWORD flag = m_pNuiSensor->NuiInitializationFlags();
	if (IS_VALID_HANDLE(m_hColorStream) && EXIST_FLAG(NUI_INITIALIZE_FLAG_USES_COLOR, flag))
		return S_OK;

	ResetColorData();
	m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	HRESULT hr = S_FALSE;
	hr = m_pNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		cColorResolution,
		0,
		2,
		m_hNextColorFrameEvent,
		&m_hColorStream);

	if (FAILED(hr))
	{
		ResetColorData();
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, OPEN_IMAGE_COLOR_ERROR);
	}

	m_pImageData = new byte[m_colorWidth * m_colorHeight * cBytesPerPixel];
	return hr;
}

HRESULT SensorContext::SetEnableDepth(BOOL bEnabled, RUNTIME_RESULT* rtHr)
{
	if (!m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}

	DWORD flag = m_pNuiSensor->NuiInitializationFlags();
	if (IS_VALID_HANDLE(m_hDepthStream) && EXIST_FLAG(flag, NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX))
		return S_OK;
	
	ResetDepthData();
	m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	HRESULT hr = S_FALSE;
	hr = m_pNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
		cDepthResolution,
		0,
		2,
		m_hNextDepthFrameEvent,
		&m_hDepthStream);

	if (FAILED(hr))
	{
		ResetDepthData();
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, OPEN_DEPTH_AND_PLAYER_INDEX_ERROR);
	}

	m_pDepthData = new byte[m_depthWidth * m_depthHeight * cBytesPerPixel];
	return hr;
}

HRESULT SensorContext::SetEnableSkeleton(BOOL bEnabled, RUNTIME_RESULT* rtHr)
{
	if (!m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}

	DWORD flag = m_pNuiSensor->NuiInitializationFlags();
	if (IS_VALID_HANDLE(m_hNextSkeletonFrameEvent) && EXIST_FLAG(flag, NUI_INITIALIZE_FLAG_USES_SKELETON))
		return S_OK;

	//清空遗留数据
	ResetSkeletonData();
	m_hNextSkeletonFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	HRESULT hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonFrameEvent, NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE);
	if (FAILED(hr))
	{
		ResetSkeletonData();
		MAKE_RUNTIMERESULT(rtHr, OPEN_SKELETON_ERROR);
		return hr;
	}
	return hr;
}

HRESULT SensorContext::SetEnableInteractioin(BOOL bEnabled, RUNTIME_RESULT* rtHr)
{
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);
	if (!bEnabled)
	{
		ResetInteractionData();
		return S_OK;
	}

	if (NULL == m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, CREATE_SENSOR_ERROR);
	}
	
	if (m_pNuiInteractionClient && m_pNuiInteractionStream)
		return S_OK;

	ResetInteractionData();
	m_pNuiInteractionClient = new KinectAdapter();
	HRESULT hr = S_OK;
	hr = NuiCreateInteractionStream(m_pNuiSensor, m_pNuiInteractionClient, &m_pNuiInteractionStream);
	if (FAILED(hr))
	{
		ResetInteractionData();
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, CREATE_INTERACTION_ERROR);
	}

	m_hNextInteractionFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (FAILED(hr = m_pNuiInteractionStream->Enable(m_hNextInteractionFrameEvent)))
	{
		ResetInteractionData();
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, ENABLE_INTERACTION_ERROR);
	}
	return S_OK;
}

HRESULT SensorContext::SetEnableBackgroundRemovedColorStream(BOOL bEnabled, RUNTIME_RESULT* rtHr)
{
	if (!bEnabled)
	{
		ResetBackgroundRemovedColorData();
		MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);
		return S_OK;
	}

	HRESULT hr;
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);
	if (NULL == m_pNuiSensor)
	{
		MAKE_RUNTIMERESULT(rtHr, CREATE_SENSOR_ERROR);
		return E_FAIL;
	}

	if (m_pBackgroundRemovalStream && m_pBackgroundRemovedData && IS_VALID_HANDLE(m_hNextBackgroundRemovedFrameEvent))
		return S_OK;

	ResetBackgroundRemovedColorData();
	hr = NuiCreateBackgroundRemovedColorStream(m_pNuiSensor, &m_pBackgroundRemovalStream);
	if (FAILED(hr))
	{
		MAKE_RUNTIMERESULT(rtHr, CREATE_BACKGROUND_REMOVED_ERROR);
		return hr;
	}


	m_hNextBackgroundRemovedFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hr = m_pBackgroundRemovalStream->Enable(cColorResolution, cDepthResolution, m_hNextBackgroundRemovedFrameEvent);
	if (FAILED(hr))
	{
		ResetBackgroundRemovedColorData();
		MAKE_RUNTIMERESULT(rtHr, ENABLE_BACKGROUND_REMOVED_ERROR);
	}

	m_pBackgroundRemovedData = new byte[m_colorWidth * m_colorHeight * cBytesPerPixel];
	return hr;
}

// Stops Sensor Polling
void SensorContext::UnInitSensor()
{
	ResetColorData();
	ResetDepthData();
	ResetBackgroundRemovedColorData();
	ResetInteractionData();

	if (m_pNuiSensor)
	{
		m_pNuiSensor->NuiSkeletonTrackingDisable();
		m_pNuiSensor->NuiShutdown();
		m_pNuiSensor->Release();
		m_pNuiSensor = NULL;
	}

	m_pNuiSensor = NULL;
	angle = 0;
}


// Polls for new Skeleton Frame
HRESULT SensorContext::ProcessSkeleton(RUNTIME_RESULT* rtHr) {

	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);
	if (!m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, CREATE_SENSOR_ERROR);
	}

	if (NO_EXIST_FLAG(m_pNuiSensor->NuiInitializationFlags(), NUI_INITIALIZE_FLAG_USES_SKELETON))
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}

	HRESULT hr = S_OK;
	NUI_SKELETON_FRAME skeletonFrame;
	hr = m_pNuiSensor->NuiSkeletonGetNextFrame(WAIT_FRAME_TIME, &skeletonFrame);
	if (FAILED(hr))
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_SKELETON_FRAME_ERROR);
	}
	NUI_SKELETON_DATA* pSkeletonData = skeletonFrame.SkeletonData;

	HRESULT iaHr = S_OK;
	if (IsInteractionEnabled())
	{
		Vector4 vTmp = {0};
		iaHr = m_pNuiSensor->NuiAccelerometerGetCurrentReading(&vTmp);
		CHECK_WITH_RUNTIMERESULT(iaHr, rtHr, ACCELEROMETER_GET_CURRENT_READING_ERROR)
		CHECK_KN_EXIT(iaHr, KN_IA_EXIT)
		iaHr = m_pNuiInteractionStream->ProcessSkeleton(NUI_SKELETON_COUNT, pSkeletonData, &vTmp, skeletonFrame.liTimeStamp);
		CHECK_WITH_RUNTIMERESULT(iaHr, rtHr, INTERACTION_PROCESS_SKELETON_ERROR)
		CHECK_KN_EXIT(iaHr, KN_IA_EXIT)
	}
KN_IA_EXIT:


	

	float closestSkeletonDistance = FLT_MAX;
	int candidateId = -1;
	
	for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
	{
		NUI_SKELETON_DATA skeleton = pSkeletonData[i];
		if(skeleton.eTrackingState == NUI_SKELETON_TRACKED)
		{
			if (m_skTackedId == skeleton.dwTrackingID)
			{
				candidateId = i;//已检测的骨架有效，则继续检测
				break;
			}
			else if (skeleton.Position.z < closestSkeletonDistance)
			{
				candidateId = i;//最近距离的骨架
				closestSkeletonDistance = skeleton.Position.z;
			}
		}
	}

	int oldId = m_skTackedId;
	if (0 <= candidateId && candidateId < NUI_SKELETON_COUNT)
	{
		m_skData = pSkeletonData[candidateId];
		m_bSkeletonValid = true;
		//检测新的骨架，更新"背景移除"需要跟踪的骨架
		if (m_skTackedId != m_skData.dwTrackingID && m_skData.dwTrackingID != NUI_SKELETON_INVALID_TRACKING_ID)
		{
			m_skTackedId = m_skData.dwTrackingID;
			printf("Curent Skeleton tracked: %d\n", m_skTackedId);
		}
	}
	else
	{
		if (m_skTackedId != NUI_SKELETON_INVALID_TRACKING_ID)
		{
			printf("warning:Lose skeleton\n");
		}
		m_bSkeletonValid = false;
		m_skTackedId = NUI_SKELETON_INVALID_TRACKING_ID;
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, GET_SKELETON_FRAME_ERROR);
	}

	HRESULT bgHr = S_OK;
	
	if (IsBackgroundRemovedEnabled())
	{
		if (oldId != m_skTackedId && m_skTackedId != NUI_SKELETON_INVALID_TRACKING_ID)
		{
			bgHr = m_pBackgroundRemovalStream->SetTrackedPlayer(m_skTackedId);
			CHECK_WITH_RUNTIMERESULT(bgHr, rtHr, SET_TRACKEDPLAYER_ERROR)
			CHECK_KN_EXIT(bgHr, KN_BG_EXIT);//如果有问题，还得注意处理interaction
		}
		bgHr = m_pBackgroundRemovalStream->ProcessSkeleton(NUI_SKELETON_COUNT, pSkeletonData, skeletonFrame.liTimeStamp);
		CHECK_WITH_RUNTIMERESULT(hr, rtHr, BACKGROUND_REMOVED_PROCESS_SKELETON_ERROR)
		CHECK_KN_EXIT(bgHr, KN_BG_EXIT);
	}
KN_BG_EXIT:

	if (FAILED(hr))
		return hr;
	else if (FAILED(iaHr))
		return iaHr;
	else if (FAILED(bgHr))
		return bgHr;
	return hr;
}


// Polls for new Image Frame
HRESULT SensorContext::ProcessColor(RUNTIME_RESULT *rtHr)
{
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);
	if (!m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, CREATE_SENSOR_ERROR);
	}

	if (NO_EXIST_FLAG(m_pNuiSensor->NuiInitializationFlags(), NUI_INITIALIZE_FLAG_USES_COLOR)
		|| !m_pImageData)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}

	HRESULT hr;
	NUI_IMAGE_FRAME imageFrame;

	// Attempt to get the depth frame
	LARGE_INTEGER colorTimeStamp;
	hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_hColorStream, WAIT_FRAME_TIME, &imageFrame);
	CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_IMAGE_FRAME_ERROR);

	colorTimeStamp = imageFrame.liTimeStamp;

	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data. Then save a copy of color frame.
	HRESULT bghr = S_OK;
	if (LockedRect.Pitch != 0 && LockedRect.size > 0)
	{
		/*
		4byte为一个rgba,不过指定的flag,a is not used
		*/
		m_nImageSize = LockedRect.size;
		memcpy(m_pImageData, LockedRect.pBits, LockedRect.size);
		m_bImageValid = true;
		if (IsBackgroundRemovedEnabled())
		{
			bghr = m_pBackgroundRemovalStream->ProcessColor(m_colorWidth * m_colorHeight * cBytesPerPixel, LockedRect.pBits, colorTimeStamp);
			CHECK_KN_EXIT(bghr, KN_EXIT);
		}
	}
	else
	{
		MAKE_RUNTIMERESULT(rtHr, GET_IMAGE_FRAME_ERROR);
		m_bImageValid = false;
	}

KN_EXIT:
	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);

	// Release the frame
	hr = m_pNuiSensor->NuiImageStreamReleaseFrame(m_hColorStream, &imageFrame);

	CHECK_RUTURN_WITH_RUNTIMERESULT(bghr, rtHr, BACKGROUND_REMOVED_PROCESS_COLOR_ERROR);
	return S_OK;
}


// Polls for new Depth Frame
HRESULT SensorContext::ProcessDepth(RUNTIME_RESULT *rtHr)
{
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);
	if (!m_pNuiSensor)
	{
		MAKE_RUNTIMERESULT(rtHr, CREATE_SENSOR_ERROR);
		return E_FAIL;
	}

	if (NO_EXIST_FLAG(m_pNuiSensor->NuiInitializationFlags(), NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX)
		|| !m_pDepthData)
	{
		MAKE_RUNTIMERESULT(rtHr, SENSOR_INITIALIZE_ERROR);
		return E_FAIL;
	}
	HRESULT hr;
	
	NUI_IMAGE_FRAME depthFrame;
	// Attempt to get the depth frame
	hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_hDepthStream, WAIT_FRAME_TIME, &depthFrame);
	CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_DEPTH_FRAME_ERROR);

	LARGE_INTEGER depthTimeStamp = depthFrame.liTimeStamp;
	INuiFrameTexture* pTexture;
	// Attempt to get the extended depth texture
	hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(m_hDepthStream, &depthFrame, &m_bNearMode, &pTexture);
	CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_DEPTH_IMAGE_PIXEL_FRAME_TEXTURE_ERROR);

	NUI_LOCKED_RECT LockedRect = { 0 };

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data, and then present it to the background removed color stream. 
	HRESULT bghr = S_OK;
	HRESULT iaHr = S_OK;
	if (LockedRect.Pitch != 0 && LockedRect.size)
	{
		/*数据结构
		4个byte为一组：
		前两个byte组合playerID
		后两个byte表示depthValue
		*/
		m_nDepthSize = LockedRect.size;
		memcpy(m_pDepthData, LockedRect.pBits, LockedRect.size);
		m_bDepthValid = true;
		if (IsBackgroundRemovedEnabled())
		{
			bghr = m_pBackgroundRemovalStream->ProcessDepth(m_depthWidth * m_depthHeight * cBytesPerPixel, LockedRect.pBits, depthTimeStamp);
			CHECK_KN_EXIT(bghr, KN_EXIT)
		}

		if (IsInteractionEnabled())
		{
			iaHr = m_pNuiInteractionStream->ProcessDepth(LockedRect.size, (PBYTE)(LockedRect.pBits), depthTimeStamp);
			CHECK_KN_EXIT(iaHr, KN_EXIT);
		}
	}
	else
	{
		MAKE_RUNTIMERESULT(rtHr, GET_DEPTH_IMAGE_PIXEL_FRAME_TEXTURE_ERROR);
		m_bDepthValid = false;
	}

KN_EXIT:
	// We're done with the texture so unlock it. Even if above process failed, we still need to unlock and release.
	pTexture->UnlockRect(0);
	pTexture->Release();

	// Release the frame
	m_pNuiSensor->NuiImageStreamReleaseFrame(m_hDepthStream, &depthFrame);
	CHECK_RUTURN_WITH_RUNTIMERESULT(bghr, rtHr, BACKGROUND_REMOVED_PROCESS_DEPTH_ERROR);
	CHECK_RUTURN_WITH_RUNTIMERESULT(iaHr, rtHr, INTERACTION_PROCESS_DEPTH_ERROR);
	return hr;
}

HRESULT SensorContext::ProcessBackgroundRemoved(RUNTIME_RESULT *rtHr)
{
	/*注意需要提前处理color，depth，skeleton的数据*/
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);

	if (!m_pBackgroundRemovalStream || !m_pBackgroundRemovedData)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}
		
	HRESULT hr = S_FALSE;
	NUI_BACKGROUND_REMOVED_COLOR_FRAME bgRemovedFrame = {0};
	hr = m_pBackgroundRemovalStream->GetNextFrame(WAIT_FRAME_TIME, &bgRemovedFrame);
	if (FAILED(hr))
	{
		m_bBackgroundRemovedValid = false;
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_BACKGROUND_REMOVED_FRAME_ERROR);
	}
	/*数据结构
	4个byte组成一组rgba
	如果a为0，说明是背景；否则，是人像
	*/
	m_nBackgroundRemovedSize = bgRemovedFrame.backgroundRemovedColorDataLength;
	memcpy(m_pBackgroundRemovedData, bgRemovedFrame.pBackgroundRemovedColorData, m_nBackgroundRemovedSize);
	m_bBackgroundRemovedValid = true;
	hr = m_pBackgroundRemovalStream->ReleaseFrame(&bgRemovedFrame);
	return hr;
}

HRESULT SensorContext::PickHandEventType(
	const NUI_HANDPOINTER_INFO* pHandPointerInfo, 
	NUI_HAND_EVENT_TYPE lastEventType /*= NUI_HAND_EVENT_TYPE_NONE*/, 
	NUI_HAND_EVENT_TYPE* pEventType /*= 0*/) const
{
	/*
	NUI_HAND_EVENT_TYPE_NONE // No change from last event, or an undefined change.
	所以需要对考虑last event状态
	*/
	if (!pHandPointerInfo)
		return E_INVALIDARG;

	if (!pEventType)
		return S_OK;

	DWORD state = pHandPointerInfo->State;
	if (EXIST_FLAG(state, NUI_HANDPOINTER_STATE_NOT_TRACKED)
		|| NO_EXIST_FLAG(state, NUI_HANDPOINTER_STATE_ACTIVE))
	{
		*pEventType = NUI_HAND_EVENT_TYPE_NONE;
	}
	else
	{
		//如果手移除区域，就不能保证上次状态。必须再加区域判断
		*pEventType = pHandPointerInfo->HandEventType;
		if (NUI_HAND_EVENT_TYPE_NONE == *pEventType)
			*pEventType = lastEventType;
	}
	
	DebugHandStateMsg(pHandPointerInfo->HandType, state);
	return S_OK;
}

HRESULT SensorContext::ProcessInteraction(RUNTIME_RESULT* rtHr /*= 0*/)
{
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);

	if (!m_pNuiInteractionClient || !m_pNuiInteractionStream)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}

	HRESULT hr = S_FALSE;
	NUI_INTERACTION_FRAME iaFrame = {0};
	hr = m_pNuiInteractionStream->GetNextFrame(WAIT_FRAME_TIME, &iaFrame);
	if (FAILED(hr))
	{
		m_bInteractionValid = FALSE;
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_INTERACTION_FRAME_ERROR);
	}

	
	printf("new frame data--------------------------------------------\n");
	int candidateId = NUI_SKELETON_INVALID_TRACKING_ID;
	bool bMainUserUpdated = false;
	for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		const NUI_USER_INFO* pUserInfo = iaFrame.UserInfos + i;
		if (NUI_SKELETON_INVALID_TRACKING_ID == pUserInfo->SkeletonTrackingId)
			continue;

		candidateId = i;
		printf("-----interaction skeleton %d with id %d------------\n", i, pUserInfo->SkeletonTrackingId);
		if (pUserInfo->SkeletonTrackingId == m_mainUserInfo.SkeletonTrackingId)
		{
			UpdateLastHandType(pUserInfo, &m_mainUserInfo);
			bMainUserUpdated = true;
		}
	}

	if (!bMainUserUpdated && candidateId != NUI_SKELETON_INVALID_TRACKING_ID)
	{
		m_mainUserInfo = iaFrame.UserInfos[candidateId];
	}
	//system("pause");
	system("CLS");
	return hr;
}

HRESULT SensorContext::UpdateLastHandType(IN const NUI_USER_INFO* pNewUserInfo, OUT NUI_USER_INFO* pLastUserInfo) const
{
	/*
	结合上次和最新的手势，更新上次的手势
	*/
	if (!pNewUserInfo || !pLastUserInfo)
		return E_INVALIDARG;

	//没跟踪的骨架
	if (NUI_SKELETON_INVALID_TRACKING_ID == pNewUserInfo->SkeletonTrackingId)
		return S_FALSE;

	//必须结合之前的eventType状态
	for (int j = 0; j < NUI_USER_HANDPOINTER_COUNT; ++j)
	{
		const NUI_HANDPOINTER_INFO* pHandPointerInfo = pNewUserInfo->HandPointerInfos + j;
		NUI_HANDPOINTER_INFO* pLastHandPointerInfo = pLastUserInfo->HandPointerInfos + j;
		if (j == 0)
		{
			assert(NUI_HAND_TYPE_LEFT == pHandPointerInfo->HandType);
		}
		else
		{
			assert(NUI_HAND_TYPE_RIGHT == pHandPointerInfo->HandType);
		}

		NUI_HAND_EVENT_TYPE eventType = NUI_HAND_EVENT_TYPE_NONE;
		if (NUI_HAND_TYPE_NONE != pHandPointerInfo->HandType)
		{
			PickHandEventType(pHandPointerInfo, pLastHandPointerInfo->HandEventType, &eventType);
			pLastHandPointerInfo->HandEventType = eventType;//更新手势
		}

		DebugHandEventType(eventType);
	}
	return S_OK;
}


// Set camera angle
HRESULT SensorContext::SetCameraAngle(long angle) {

	return m_pNuiSensor->NuiCameraElevationSetAngle(angle);
}

// Transform coordinates from camera view space to world space
void SensorContext::TransformCoordinates(OUT KUVector4* skTrans) {

	KUVector4 &in = *skTrans;

	in.x = in.x;
	in.y = (in.y * cos((float)angle * (PI / 180))) + (in.z * sin((float)angle * (PI / 180)));
	in.z = (-in.y * sin((float)angle * (PI / 180))) + (in.z * cos((float)angle * (PI / 180)));
}

SensorContext::~SensorContext()
{
	UnInitSensor();
}

BOOL SensorContext::IsInteractionEnabled() const
{
	return m_pNuiInteractionClient != NULL && m_pNuiInteractionStream != NULL;
}

BOOL SensorContext::IsBackgroundRemovedEnabled() const
{
	return m_pBackgroundRemovalStream != NULL;
}

BOOL SensorContext::IsColorEnabled() const
{
	return m_pNuiSensor && EXIST_FLAG(m_pNuiSensor->NuiInitializationFlags(), NUI_INITIALIZE_FLAG_USES_COLOR);
}

BOOL SensorContext::IsDepthEnabled() const
{
	return m_pNuiSensor && EXIST_FLAG(m_pNuiSensor->NuiInitializationFlags(), NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX);
}

BOOL SensorContext::IsSkeletonEnabled() const
{
	return m_pNuiSensor && EXIST_FLAG(m_pNuiSensor->NuiInitializationFlags(), NUI_INITIALIZE_FLAG_USES_SKELETON);
}

void SensorContext::ResetColorData()
{
	m_hColorStream = INVALID_HANDLE_VALUE;
	CLOSE_HANDLE(m_hNextColorFrameEvent);

	if (m_pImageData)
	{
		delete[] m_pImageData;
		m_pImageData = NULL;
	}

	m_nImageSize = 0;
	m_bImageValid = false;
}

void SensorContext::ResetDepthData()
{
	m_hDepthStream = INVALID_HANDLE_VALUE;
	CLOSE_HANDLE(m_hNextDepthFrameEvent);

	if (m_pDepthData)
	{
		delete[] m_pDepthData;
		m_pDepthData = NULL;
	}

	m_nDepthSize = 0;
	m_bDepthValid = false;
}

void SensorContext::ResetSkeletonData()
{
	CLOSE_HANDLE(m_hNextSkeletonFrameEvent);
	memset(&m_skData, 0, sizeof(m_skData));
	m_bSkeletonValid = false;
	m_skTackedId = NUI_SKELETON_INVALID_TRACKING_ID;
}

void SensorContext::ResetBackgroundRemovedColorData()
{
	if (m_pBackgroundRemovalStream)
	{
		m_pBackgroundRemovalStream->Disable();
		m_pBackgroundRemovalStream->Release();
		m_pBackgroundRemovalStream = NULL;
	}

	CLOSE_HANDLE(m_hNextBackgroundRemovedFrameEvent);

	if (m_pBackgroundRemovedData)
	{
		delete[] m_pBackgroundRemovedData;
		m_pBackgroundRemovedData = NULL;
	}

	m_nBackgroundRemovedSize = 0;
	m_bBackgroundRemovedValid = false;
}

void SensorContext::ResetInteractionData()
{
	if (m_pNuiInteractionStream)
	{
		m_pNuiInteractionStream->Disable();
		m_pNuiInteractionStream->Release();
		m_pNuiInteractionStream = NULL;
	}

	if (m_pNuiInteractionClient)
	{
		delete m_pNuiInteractionClient;
		m_pNuiInteractionClient = NULL;
	}

	

	CLOSE_HANDLE(m_hNextInteractionFrameEvent);
	m_bInteractionValid = FALSE;
}

bool SensorContext::IsValidUseInfo(const NUI_USER_INFO* pUserInfo) const
{
	return pUserInfo && NUI_SKELETON_INVALID_TRACKING_ID != pUserInfo->SkeletonTrackingId;
}

void SensorContext::DebugHandStateMsg(NUI_HAND_TYPE hand, DWORD state) const
{
	if (NUI_HAND_TYPE_NONE == hand)
		return;

	printf("==========hand type :");
	printf(hand == NUI_HAND_TYPE_LEFT ? "Left Hand" : "Right Hand");
	printf("==========\n");

	printf("hand point state:");
	if (state & NUI_HANDPOINTER_STATE_NOT_TRACKED)
		printf("Not_TRACKED");

	if (state & NUI_HANDPOINTER_STATE_TRACKED)
		printf("|TRACKED");

	if (state & NUI_HANDPOINTER_STATE_ACTIVE)
		printf(" | ACTIVE");

	if (state & NUI_HANDPOINTER_STATE_INTERACTIVE)
		printf(" | INTERACTIVE");

	if (state & NUI_HANDPOINTER_STATE_PRESSED)
		printf(" | PRESSED");

	if (state & NUI_HANDPOINTER_STATE_PRIMARY_FOR_USER)
		printf(" | PRIMARY_FOR_USER");

	printf("\n");
}

void SensorContext::DebugHandEventType(NUI_HAND_EVENT_TYPE t) const
{
	printf("Hand event type :");
	switch (t)
	{
	case NUI_HAND_EVENT_TYPE_GRIP:
		printf("Grip");
		break;
	case NUI_HAND_TYPE_RIGHT:
		printf("Grip_Release");
		break;
	}
	printf("\n");
}



KinectAdapter::KinectAdapter()
{

}

KinectAdapter::~KinectAdapter()
{

}

HRESULT STDMETHODCALLTYPE KinectAdapter::QueryInterface(REFIID riid, void **ppv)
{
	return S_OK;
}

ULONG STDMETHODCALLTYPE KinectAdapter::AddRef()
{
	return 2;
}

ULONG STDMETHODCALLTYPE KinectAdapter::Release()
{
	return 1;
}

HRESULT STDMETHODCALLTYPE KinectAdapter::GetInteractionInfoAtLocation(
	DWORD skeletonTrackingId, 
	NUI_HAND_TYPE handType, 
	FLOAT x, 
	FLOAT y, 
	_Out_ NUI_INTERACTION_INFO *pInteractionInfo)
{
	if (pInteractionInfo)
	{
		
		/*if (pInteractionInfo->IsGripTarget)
		{
			printf("Grip_Target|");
		}

		if (pInteractionInfo->IsPressTarget)
		{
			printf("press_target|");
		}*/
		pInteractionInfo->IsGripTarget = FALSE;
		pInteractionInfo->IsPressTarget = TRUE;
		/*printf("target point x: %f, y:%f\n", x,
			y);*/
	}
	return S_OK;
}