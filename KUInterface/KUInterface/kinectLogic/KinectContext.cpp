#include "stdafx.h"
#include "KinectContext.h"
#include <float.h>
#include <assert.h>

SensorContext::SensorContext()
	: m_pNuiSensor(NULL)
	, m_bNearMode(FALSE)
	, m_bComposed(false)
	, m_bInterActEnabled(false)
{
	//freopen("out.txt", "w", stdout);
}

//初始化，多次调用不会有产生多次重复的flag初始化
HRESULT SensorContext::InitSensor(DWORD flag, RUNTIME_RESULT* rtHr /*= 0*/)
{
	HRESULT hr = S_FALSE;
	if (!m_pNuiSensor)
	{
		UnInitSensor();//先清空数据
		if (S_OK != FindSensor(rtHr))
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
			hrColor = SetColorEnabled(TRUE, rtHr);
			if (FAILED(hrColor))
				return hrColor;
		}

		if (EXIST_FLAG(flag, NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX)
			&& NO_EXIST_FLAG(oldFlag, NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX))
		{
			hrDepth = SetDepthEnabled(TRUE, rtHr);
			if (FAILED(hrDepth))
				return hrDepth;
			hrDepth = m_pNuiSensor->NuiImageStreamSetImageFrameFlags(m_depthData.m_hStream, NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE);
			if (FAILED(hrDepth))
				return hrDepth;
		}

		if (EXIST_FLAG(flag, NUI_INITIALIZE_FLAG_USES_SKELETON)
			&& NO_EXIST_FLAG(oldFlag, NUI_INITIALIZE_FLAG_USES_SKELETON))
		{
			hrSkeleton = SetSkeletonEnabled(TRUE, rtHr);
			if (FAILED(hrSkeleton))
				return hrSkeleton;
		}
		//m_pNuiSensor->NuiCameraElevationGetAngle(&angle);
		
	}

	printf("Sensor Ready...\n");
	return hr;
}


HRESULT SensorContext::FindSensor(RUNTIME_RESULT* rtHr /*= 0*/)
{
	HRESULT hr = S_FALSE;
	int sensorCount = 0;
	if (FAILED(NuiGetSensorCount(&sensorCount)))
		return hr;

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

HRESULT SensorContext::SetColorEnabled(BOOL bEnabled, RUNTIME_RESULT* rtHr)
{
	if (!m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}

	DWORD flag = m_pNuiSensor->NuiInitializationFlags();
	if (IS_VALID_HANDLE(m_colorData.m_hStream) && EXIST_FLAG(NUI_INITIALIZE_FLAG_USES_COLOR, flag))
		return S_OK;

	ResetColorData();
	m_colorData.m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HRESULT hr = S_FALSE;
	hr = m_pNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		cColorResolution,
		0,
		2,
		m_colorData.m_hEvent,
		&m_colorData.m_hStream);

	if (FAILED(hr))
	{
		ResetColorData();
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, OPEN_IMAGE_COLOR_ERROR);
	}
	m_colorData.SetResolution(cColorResolution);
	return hr;
}

HRESULT SensorContext::SetDepthEnabled(BOOL bEnabled, RUNTIME_RESULT* rtHr)
{
	if (!m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}

	DWORD flag = m_pNuiSensor->NuiInitializationFlags();
	if (IS_VALID_HANDLE(m_depthData.m_hStream) && EXIST_FLAG(flag, NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX))
		return S_OK;
	
	ResetDepthData();
	m_depthData.m_hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	HRESULT hr = S_FALSE;
	hr = m_pNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
		cDepthResolution,
		0,
		2,
		m_depthData.m_hEvent,
		&m_depthData.m_hStream);

	if (FAILED(hr))
	{
		ResetDepthData();
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, OPEN_DEPTH_AND_PLAYER_INDEX_ERROR);
	}

	m_depthData.SetResolution(cDepthResolution);
	return hr;
}

HRESULT SensorContext::SetSkeletonEnabled(BOOL bEnabled, RUNTIME_RESULT* rtHr)
{
	if (!m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}

	DWORD flag = m_pNuiSensor->NuiInitializationFlags();
	if (IS_VALID_HANDLE(m_skeletonData.m_hEvent) && EXIST_FLAG(flag, NUI_INITIALIZE_FLAG_USES_SKELETON))
		return S_OK;

	//清空遗留数据
	ResetSkeletonData(); 
	m_skeletonData.m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	HRESULT hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_skeletonData.m_hEvent, NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE);
	if (FAILED(hr))
	{
		ResetSkeletonData();
		MAKE_RUNTIMERESULT(rtHr, OPEN_SKELETON_ERROR);
		return hr;
	}
	return hr;
}

HRESULT SensorContext::SetInteractionCount(UINT peopelCount, RUNTIME_RESULT* rtHr)
{
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);
	if (peopelCount <= 0)
	{
		ResetInteractionData();
		return S_OK;
	}

	if (NULL == m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, CREATE_SENSOR_ERROR);
	}
	
	if (m_interActData.m_pStream != NULL)
		return S_OK;

	HRESULT hr = S_FALSE;

	m_interActData.m_pClient = new KinectAdapter();
	hr = NuiCreateInteractionStream(m_pNuiSensor, m_interActData.m_pClient, &m_interActData.m_pStream);
	if (FAILED(hr))
	{
		m_interActData.Reset();
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, CREATE_INTERACTION_ERROR);
	}

	m_interActData.m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (FAILED(hr = m_interActData.m_pStream->Enable(m_interActData.m_hEvent)))
	{
		m_interActData.Reset();
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, ENABLE_INTERACTION_ERROR);
	}
	m_bInterActEnabled = true;
	return S_OK;
}

HRESULT SensorContext::SetBackgroundRemovedCount(UINT peopelCount, RUNTIME_RESULT* rtHr)
{
	if (peopelCount <= 0)
	{
		ResetBackgroundRemovedColorData();
		CHECK_RUTURN_WITH_RUNTIMERESULT(S_OK, rtHr, SUCCEEDED_OK);
	}

	if (NULL == m_pNuiSensor)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, CREATE_SENSOR_ERROR);
	}
	
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);
	if (m_backGrmDataVec.size() >= peopelCount)
		return S_OK;

	HRESULT hr = S_FALSE;
	for (UINT i = m_backGrmDataVec.size(); i < peopelCount; ++i)
	{
		BackGroudRemvoedData tmpData;
		hr = NuiCreateBackgroundRemovedColorStream(m_pNuiSensor, &tmpData.m_pStream);
		if (FAILED(hr) || tmpData.m_pStream == NULL)
			CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, CREATE_BACKGROUND_REMOVED_ERROR);
		tmpData.m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		hr = tmpData.m_pStream->Enable(cColorResolution, cDepthResolution, tmpData.m_hEvent);
		if (FAILED(hr))
		{
			tmpData.ResetData();
			CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, ENABLE_BACKGROUND_REMOVED_ERROR);
		}

		tmpData.m_frameData.SetResolution(cColorResolution);
		m_backGrmDataVec.push_back(tmpData);
	}
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
	nKinecAngle = 0;
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

	const NUI_SKELETON_DATA* pSkeletonData = skeletonFrame.SkeletonData;

	if (!PickValidSkeleton(pSkeletonData))
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SUCCEEDED_FALSE);
	}

	{
		//debug
		for (SkeletonIdMap::const_iterator itr = m_skeletonData.m_fullTrackedMap.begin();
			itr != m_skeletonData.m_fullTrackedMap.end(); ++itr)
		{
			printf("skeleton : %d, state=%d\n", itr->first, m_skeletonData.m_dataArray[itr->second].eTrackingState);
		}

		for (SkeletonIdMap::const_iterator itr = m_skeletonData.m_unFullTrackedMap.begin();
			itr != m_skeletonData.m_unFullTrackedMap.end(); ++itr)
		{
			printf("skeleton : %d, state=%d\n", itr->first, m_skeletonData.m_dataArray[itr->second].eTrackingState);
		}
	}
	
	m_skeletonData.m_timeStamp = skeletonFrame.liTimeStamp;
	
	if (m_skeletonData.m_fullTrackedMap.size() > 0)
	{
		HRESULT bgHr = ProcessSkeletonInBackgroundRemoved(pSkeletonData, rtHr);
		HRESULT iaHr = ProcessSkeletonInInteraction(pSkeletonData, rtHr);
		if (FAILED(iaHr))
			return iaHr;
		else if (FAILED(bgHr))
			return bgHr;
	}
	
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

	if (NO_EXIST_FLAG(m_pNuiSensor->NuiInitializationFlags(), NUI_INITIALIZE_FLAG_USES_COLOR))
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_FAIL, rtHr, SENSOR_INITIALIZE_ERROR);
	}

	HRESULT hr;
	NUI_IMAGE_FRAME imageFrame;
	hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_colorData.m_hStream, WAIT_FRAME_TIME, &imageFrame);
	CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_IMAGE_FRAME_ERROR);
	hr = CopyTextureData(imageFrame.pFrameTexture, imageFrame.liTimeStamp, m_colorData, rtHr);
	hr = m_pNuiSensor->NuiImageStreamReleaseFrame(m_colorData.m_hStream, &imageFrame);
	if (FAILED(hr))
		return hr;
	if (IsBackgroundRemovedEnabled())
	{
		HRESULT bghr = ProcessColorInBackgroundRemoved(rtHr);
		CHECK_RUTURN_WITH_RUNTIMERESULT(bghr, rtHr, BACKGROUND_REMOVED_PROCESS_COLOR_ERROR);
	}
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

	if (NO_EXIST_FLAG(m_pNuiSensor->NuiInitializationFlags(), NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX))
	{
		MAKE_RUNTIMERESULT(rtHr, SENSOR_INITIALIZE_ERROR);
		return E_FAIL;
	}
	HRESULT hr;
	
	NUI_IMAGE_FRAME depthFrame = {0};
	// Attempt to get the depth frame
	hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_depthData.m_hStream, WAIT_FRAME_TIME, &depthFrame);
	CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_DEPTH_FRAME_ERROR);
	
	INuiFrameTexture* pTexture = NULL;
	hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(m_depthData.m_hStream,
		&depthFrame, &m_bNearMode, &pTexture);
	CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_DEPTH_IMAGE_PIXEL_FRAME_TEXTURE_ERROR);

	hr = CopyTextureData(pTexture, depthFrame.liTimeStamp, m_depthData, rtHr);
	pTexture->Release();
	m_pNuiSensor->NuiImageStreamReleaseFrame(m_depthData.m_hStream, &depthFrame);
	if (FAILED(hr))
		return hr;

	HRESULT bghr = S_OK;
	if (IsBackgroundRemovedEnabled())
	{
		bghr = ProcessDepthInBackgroundRemoved(rtHr);
	}
	HRESULT iaHr = S_OK;
	if (IsInteractionEnabled())
	{
		iaHr = ProcessDepthInInteraction(rtHr);
	}
	
	CHECK_RUTURN_WITH_RUNTIMERESULT(bghr, rtHr, BACKGROUND_REMOVED_PROCESS_DEPTH_ERROR);
	CHECK_RUTURN_WITH_RUNTIMERESULT(iaHr, rtHr, INTERACTION_PROCESS_DEPTH_ERROR);
	return hr;
}

HRESULT SensorContext::ProcessBackgroundRemoved(UINT index, RUNTIME_RESULT *rtHr)
{
	/*注意需要提前处理color，depth，skeleton的数据*/
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);

	if (index < 0 || index >= m_backGrmDataVec.size() || m_backGrmDataVec.size() <= 0)
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(E_INVALIDARG, rtHr, SENSOR_INITIALIZE_ERROR);
	}
		
	HRESULT hr = S_FALSE;
	NUI_BACKGROUND_REMOVED_COLOR_FRAME bgRemovedFrame = {0};
	hr = m_backGrmDataVec[index].m_pStream->GetNextFrame(WAIT_FRAME_TIME, &bgRemovedFrame);
	
	/*数据结构
	4个byte组成一组rgba
	如果a为0，说明是背景；否则，是人像
	*/
	if (hr == S_OK)
	{
		m_backGrmDataVec[index].m_frameData.CopyData(
			bgRemovedFrame.pBackgroundRemovedColorData, 
			bgRemovedFrame.backgroundRemovedColorDataLength,
			bgRemovedFrame.liTimeStamp);
		printf("with m_backGrmDataVec index %d, trackid %d\n", index, m_backGrmDataVec[index].m_trackedId);
		//WriteFrameData(&(m_backGrmDataVec[index].m_frameData));
	}
	
	HRESULT rHr = m_backGrmDataVec[index].m_pStream->ReleaseFrame(&bgRemovedFrame);
	if (FAILED(hr))
	{

		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_BACKGROUND_REMOVED_FRAME_ERROR);
	}

	if (IsBackgroundRemovedComposed())
		ComposeBackgroundRemoved(index);
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

	printf("x:%f,Y:%f,Z:%f\n", pHandPointerInfo->RawX, pHandPointerInfo->RawY, pHandPointerInfo->RawZ);
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

	HRESULT hr = S_FALSE;
	NUI_INTERACTION_FRAME iaFrame = {0};
	hr = m_interActData.m_pStream->GetNextFrame(WAIT_FRAME_TIME, &iaFrame);
	if (FAILED(hr))
	{
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, GET_INTERACTION_FRAME_ERROR);
	}

	
	printf("new frame data--------------------------------------------\n");

	bool updatedFlags[6] = {false};

	UINT sum = 0;
	UINT updatedSum = 0;
	for (UINT i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		const NUI_USER_INFO* pUserInfo = iaFrame.UserInfos + i;
		if (NUI_SKELETON_INVALID_TRACKING_ID == pUserInfo->SkeletonTrackingId)
		{
			continue;
		}
		++sum;
		SkeletonIdMap::const_iterator itr = m_interActData.m_userIdMap.find(pUserInfo->SkeletonTrackingId);
		//更形上次有效的
		if (itr != m_interActData.m_userIdMap.end())
		{
			printf("-----interaction skeleton %d with SkeletonTrackingId %d------------\n", i, pUserInfo->SkeletonTrackingId);
			UpdateLastHandType(pUserInfo, &(m_interActData.m_userInfos[itr->second]));
			updatedFlags[itr->second] = true;
			++updatedSum;
		}
	}

	if (sum == updatedSum && m_interActData.m_userIdMap.size() == sum)
		return S_OK;
		
	m_interActData.m_userIdMap.clear();
	for (UINT i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		if (updatedFlags[i] == true)
		{
			m_interActData.m_userIdMap[m_interActData.m_userInfos[i].SkeletonTrackingId] = i;
		}
	}

	for (UINT i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		const NUI_USER_INFO* pUserInfo = iaFrame.UserInfos + i;
		if (NUI_SKELETON_INVALID_TRACKING_ID == pUserInfo->SkeletonTrackingId 
			|| updatedFlags[i] == true)
		{
			continue;
		}

		for (UINT j = 0; j < NUI_SKELETON_COUNT; ++j)
		{
			if (updatedFlags[j] == true)
				continue;

			printf("-----interaction skeleton %d with SkeletonTrackingId %d------------\n", i, pUserInfo->SkeletonTrackingId);
			m_interActData.m_userInfos[j] = *pUserInfo;
			updatedFlags[j] = true;
			m_interActData.m_userIdMap[pUserInfo->SkeletonTrackingId] = j;
			break;
		}	
	}
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
void SensorContext::TransformCoordinates(OUT Vector4* skTrans) {

	Vector4 &in = *skTrans;

	in.x = in.x;
	in.y = (in.y * cos((float)nKinecAngle * (PI / 180))) + (in.z * sin((float)nKinecAngle * (PI / 180)));
	in.z = (-in.y * sin((float)nKinecAngle * (PI / 180))) + (in.z * cos((float)nKinecAngle * (PI / 180)));
}

SensorContext::~SensorContext()
{
	UnInitSensor();

}

BOOL SensorContext::IsInteractionEnabled() const
{
	return m_bInterActEnabled;
}

BOOL SensorContext::IsBackgroundRemovedEnabled() const
{
	return m_backGrmDataVec.size() > 0;
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
	m_colorData.Reset();
}

void SensorContext::ResetDepthData()
{
	m_depthData.Reset();
}

void SensorContext::ResetSkeletonData()
{
	m_skeletonData.Reset();
}

void SensorContext::ResetBackgroundRemovedColorData()
{
	for (UINT i = 0; i < m_backGrmDataVec.size(); ++i)
	{
		m_backGrmDataVec[i].ResetData();
	}
	m_backGrmDataVec.clear();
	m_backGrmIdMap.clear();
	m_bComposed = false;
}

void SensorContext::ResetInteractionData()
{
	m_interActData.Reset();
	m_bInterActEnabled = false;
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
	if (state == NUI_HANDPOINTER_STATE_NOT_TRACKED)
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

INuiSensor* SensorContext::GetSensor()
{
	return m_pNuiSensor;
}

bool SensorContext::PickValidSkeleton(const NUI_SKELETON_DATA pNewData[])
{
	m_skeletonData.m_fullTrackedMap.clear();
	m_skeletonData.m_unFullTrackedMap.clear();
	
	for (int i = 0; i < NUI_SKELETON_COUNT; i++, ++pNewData)
	{
		if (pNewData->eTrackingState == NUI_SKELETON_NOT_TRACKED
			|| pNewData->dwTrackingID == NUI_SKELETON_INVALID_TRACKING_ID)
			continue;

		m_skeletonData.m_dataArray[i] = *pNewData;

		if (pNewData->eTrackingState == NUI_SKELETON_TRACKED)
		{
			m_skeletonData.m_fullTrackedMap[pNewData->dwTrackingID] = i;
		}
		else if (pNewData->eTrackingState == NUI_SKELETON_POSITION_ONLY)
		{
			m_skeletonData.m_unFullTrackedMap[pNewData->dwTrackingID] = i;
		}
	}
	
	return m_skeletonData.m_fullTrackedMap.size() + m_skeletonData.m_unFullTrackedMap.size() > 0;
}


HRESULT SensorContext::ProcessSkeletonInBackgroundRemoved(const NUI_SKELETON_DATA* skeletons, RUNTIME_RESULT* pHr)
{
	//背景去除支持两个人，以后要支持多个人，可能需要重复更换跟踪的full骨架。
	MAKE_RUNTIMERESULT(pHr, SUCCEEDED_OK);

	if (!IsBackgroundRemovedEnabled())
	{
		MAKE_RUNTIMERESULT(pHr, SUCCEEDED_FALSE);
		return S_FALSE;
	}

	HRESULT hr = S_OK;

	bool bTracked[NUI_SKELETON_COUNT] = {false};
	//保持持续有效的id继续跟踪
	for (SkeletonIdMap::const_iterator itr = m_skeletonData.m_fullTrackedMap.begin();
		itr != m_skeletonData.m_fullTrackedMap.end(); ++itr)
	{
		SkeletonIdMap::const_iterator bgItr = m_backGrmIdMap.find(itr->first);

		if (bgItr != m_backGrmIdMap.end())//持续的id
		{
			bTracked[bgItr->second] = true;
			hr = m_backGrmDataVec[bgItr->second].m_pStream->ProcessSkeleton(NUI_SKELETON_COUNT, skeletons, m_skeletonData.m_timeStamp);
			CHECK_RUTURN_WITH_RUNTIMERESULT(hr, pHr, BACKGROUND_REMOVED_PROCESS_SKELETON_ERROR);
		}
	}

	//查找新的骨架，并跟踪
	for (SkeletonIdMap::const_iterator itr = m_skeletonData.m_fullTrackedMap.begin();
		itr != m_skeletonData.m_fullTrackedMap.end(); ++itr)
	{
		SkeletonIdMap::const_iterator bgItr = m_backGrmIdMap.find(itr->first);

		if (bgItr != m_backGrmIdMap.end())
		{
			continue;//已经跟踪过
		}

		//获取空余的流来处理
		for (UINT i = 0; i < m_backGrmDataVec.size(); ++i)
		{
			if (bTracked[i] == true)
			{
				continue;
			}
			hr = m_backGrmDataVec[i].m_pStream->SetTrackedPlayer(itr->first);
			CHECK_RUTURN_WITH_RUNTIMERESULT(hr, pHr, SET_TRACKEDPLAYER_ERROR)
			hr = m_backGrmDataVec[i].m_pStream->ProcessSkeleton(NUI_SKELETON_COUNT, skeletons, m_skeletonData.m_timeStamp);
			CHECK_RUTURN_WITH_RUNTIMERESULT(hr, pHr, BACKGROUND_REMOVED_PROCESS_SKELETON_ERROR);
			m_backGrmDataVec[i].m_trackedId = itr->first;
			bTracked[i] = true;
			break;
		}
	}
	
	//更新map数组
	m_backGrmIdMap.clear();
	for (UINT i = 0; i < m_backGrmDataVec.size(); ++i)
	{
		if (bTracked[i] == false)
			continue;

		m_backGrmIdMap[m_backGrmDataVec[i].GetTrackedId()] = i;//选取有效的数据
		printf("background <-- skeleton %d by stream %d\n", m_backGrmDataVec[i].GetTrackedId(), i);
	}
	return hr;
}

HRESULT SensorContext::ProcessSkeletonInInteraction(const NUI_SKELETON_DATA* skeletons, RUNTIME_RESULT* rtHr)
{
	MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_OK);
	if (!IsInteractionEnabled() || m_skeletonData.GetFullSkeletonCount() == 0)
	{
		MAKE_RUNTIMERESULT(rtHr, SUCCEEDED_FALSE);
		return S_FALSE;
	}
	Vector4 vTmp = {0};
	HRESULT hr = m_pNuiSensor->NuiAccelerometerGetCurrentReading(&vTmp);
	CHECK_WITH_RUNTIMERESULT(hr, rtHr, ACCELEROMETER_GET_CURRENT_READING_ERROR)

	hr = m_interActData.m_pStream->ProcessSkeleton(NUI_SKELETON_COUNT, skeletons, &vTmp, m_skeletonData.m_timeStamp);
	CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, INTERACTION_PROCESS_SKELETON_ERROR)
	return S_OK;
}

HRESULT SensorContext::ProcessColorInBackgroundRemoved(RUNTIME_RESULT* rtHr)
{
	assert(IsBackgroundRemovedEnabled());
	if (!m_colorData.IsValid())
		return E_FAIL;

	HRESULT hr = S_FALSE;
	for (UINT i = 0; i < m_backGrmDataVec.size(); ++i)
	{
		hr = m_backGrmDataVec[i].m_pStream->ProcessColor(m_colorData.GetSize(),
			m_colorData.GetData(), 
			m_colorData.GetTimeStamp());

		if (FAILED(hr))
		{
			CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, BACKGROUND_REMOVED_PROCESS_COLOR_ERROR);
		}
	}
	return S_OK;
}

HRESULT SensorContext::CopyTextureData(IN INuiFrameTexture* pTexture, 
	const LARGE_INTEGER& timeStamp, 
	OUT ImageData& dst, 
	RUNTIME_RESULT* rtHr)
{
	NUI_LOCKED_RECT LockedRect;
	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	if (LockedRect.Pitch == 0 
		|| LockedRect.size <= 0
		|| !dst.CopyData(LockedRect.pBits, LockedRect.size, timeStamp))
	{
		MAKE_RUNTIMERESULT(rtHr, GET_IMAGE_FRAME_ERROR);
	}
	pTexture->UnlockRect(0);
	return S_OK;
}

HRESULT SensorContext::ProcessDepthInBackgroundRemoved(RUNTIME_RESULT* rtHr)
{
	if (!IsBackgroundRemovedEnabled())
		return S_FALSE;

	if (!m_depthData.IsValid())
		return E_FAIL;

	HRESULT hr = S_FALSE;
	for (UINT i = 0; i < m_backGrmDataVec.size(); ++i)
	{
		hr = m_backGrmDataVec[i].m_pStream->ProcessDepth(m_depthData.GetSize(),
			m_depthData.GetData(), 
			m_depthData.GetTimeStamp());
		if (FAILED(hr))
			CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, BACKGROUND_REMOVED_PROCESS_DEPTH_ERROR);
	}
	return S_OK;
}

HRESULT SensorContext::ProcessDepthInInteraction(RUNTIME_RESULT* rtHr)
{
	if (!IsInteractionEnabled())
	{
		return S_FALSE;
	}
	
	HRESULT hr = S_FALSE;
	hr = m_interActData.m_pStream->ProcessDepth(m_depthData.GetSize(),
		m_depthData.GetData(),
		m_depthData.GetTimeStamp());
	if (FAILED(hr))
		CHECK_RUTURN_WITH_RUNTIMERESULT(hr, rtHr, INTERACTION_PROCESS_SKELETON_ERROR);
	return S_OK;
}

const ColorData* SensorContext::GetColorData() const
{
	return &m_colorData;
}

const DepthData* SensorContext::GetDepthData() const
{
	return &m_depthData;
}

UINT SensorContext::GetSkeletonSize() const
{
	return m_skeletonData.GetSkeletonCount();
}

UINT SensorContext::GetBackgroundRemovedCount() const
{
	return m_backGrmIdMap.size();
}

HRESULT SensorContext::ComposeBackgroundRemoved(UINT index)
{
	if (index >= m_backGrmDataVec.size())
		return E_FAIL;

	const FrameData* pData = m_backGrmDataVec[index].GetFrameData();
	if (false == m_backgroundComposed.Compose(pData))
		return S_FALSE;
	printf("Composing %d\n", m_backGrmDataVec[index].GetTrackedId());
	return S_OK;
}

const FrameData* SensorContext::GetBackgroundRemovedComposed() const
{
	return &m_backgroundComposed;
}

const InteractionData* SensorContext::GetInteractionData() const
{
	return &m_interActData;
}

const SkeletonData* SensorContext::GetSkeletonData() const
{
	return &m_skeletonData;
}

void SensorContext::SetBackgroundReomovedComposed(bool bCompose)
{
	if (bCompose == m_bComposed)
		return;
	m_bComposed = bCompose;
	if (m_bComposed == true)
	{
		m_backgroundComposed.SetResolution(cColorResolution);
	}
}

bool SensorContext::IsBackgroundRemovedComposed()
{
	return m_bComposed;
}

const BackGroudRemvoedData* SensorContext::GetBackgroundRemovedData(UINT index) const
{
	if (index >= m_backGrmDataVec.size())
		return NULL;
	return &m_backGrmDataVec[index];
}

UINT SensorContext::GetInteractionCount() const
{
	return m_interActData.GetCount();
}

void SensorContext::WriteFrameData(const FrameData* pData) const
{
	DWORD w = 0, h = 0;
	pData->GetSize(w, h);
	const byte* pByteData = pData->GetData();
	if (w == 0 || h == 0 || pByteData == NULL)
		return;

	printf("new frame data ---\n\n\n\n---------------------------------------------------------\n");
	bool hash[48][64] = {false};
	for (int i = 0; i < h; ++i)
	{
		for (int j = 0; j < w; ++j)
		{
			if (pByteData[i*w*4 + j*4 + 3] == 255)
			{
				hash[i/10][j/10] = 1;
			}
		}
	}

	for (int i = 0; i < 48; ++i)
	{
		for (int j = 0; j < 64; ++j)
		{
			printf("%d", hash[i][j]);
		}
		printf("\n");
	}
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

FrameData::FrameData() 
	: m_pData(NULL)
	, m_resolution(NUI_IMAGE_RESOLUTION_INVALID)
	, m_bInvalid(false)
{
	m_timeStamp.QuadPart = -1;
}

FrameData::FrameData(NUI_IMAGE_RESOLUTION res) 
	:m_pData(NULL)
	, m_bInvalid(false)
{
	SetResolution(res);
	m_timeStamp.QuadPart = -1;
}

void FrameData::Reset()
{
	delete m_pData;
	m_resolution = NUI_IMAGE_RESOLUTION_INVALID;
	m_bInvalid = false;
	m_timeStamp.QuadPart = -1;
}

void FrameData::SetResolution(NUI_IMAGE_RESOLUTION res)
{
	m_resolution = res;
	if (m_pData)
		delete m_pData;
	if (m_resolution == NUI_IMAGE_RESOLUTION_INVALID)
		return;
	
	DWORD size = GetSize(true);
	m_pData = new byte[size];
}

DWORD FrameData::GetSize(bool bInByte) const
{
	DWORD width = 0, height = 0;
	GetSize(width, height);
	if (bInByte)
		return width * height * 4;
	return width * height;
}

void FrameData::GetSize(DWORD& width, DWORD& height) const
{
	NuiImageResolutionToSize(m_resolution, width, height);
}

NUI_IMAGE_RESOLUTION FrameData::GetResolution() const
{
	return m_resolution;
}

bool FrameData::CopyData(const byte* data, DWORD size, const LARGE_INTEGER& timeStamp)
{
	DWORD tmpSize = GetSize(true);
	//assert(GetSize(true) == size);
	if (GetSize(true) != size)
	{
		m_bInvalid = false;
		return false;
	}
	memcpy(m_pData, data, size);
	m_timeStamp = timeStamp;
	m_bInvalid = true;
	return true;
}

bool FrameData::IsValid() const
{
	return m_bInvalid;
}

const byte* FrameData::GetData() const
{
	return m_pData;
}

const LARGE_INTEGER& FrameData::GetTimeStamp() const
{
	assert(m_timeStamp.QuadPart >= 0);
	return m_timeStamp;
}

DWORD FrameData::GetWidth() const
{
	DWORD width = 0, height = 0;
	GetSize(width, height);
	return width;
}

DWORD FrameData::GetHeight() const
{
	DWORD width = 0, height = 0;
	GetSize(width, height);
	return height;
}

bool FrameData::Compose(const byte* pData, NUI_IMAGE_RESOLUTION res)
{
	if (pData == NULL)
		return false;

	if (m_pData == NULL)
	{
		SetResolution(res);
	}
	else if (res != m_resolution)
	{
		return false;
	}

	int size = GetSize(true);
	for (int i = 0; i < size; i += 4)
	{
		if (pData[i+3] == 255)
		{
			m_pData[i] = pData[i];
			m_pData[i+1] = pData[i+1];
			m_pData[i+2] = pData[i+2];
		}
	}
	m_bInvalid = true;
	return true;
}

bool FrameData::Compose(const FrameData* pData)
{
	if (!pData || pData->GetTimeStamp().QuadPart < m_timeStamp.QuadPart)
		return false;
	return Compose(pData->GetData(), pData->GetResolution());
}

BackGroudRemvoedData::BackGroudRemvoedData() 
	: m_pStream(NULL)
	, m_hEvent(INVALID_HANDLE_VALUE)
	, m_trackedId(NUI_SKELETON_INVALID_TRACKING_ID)
{

}

void BackGroudRemvoedData::ResetData()
{
	if (m_pStream)
	{
		m_pStream->Disable();
		m_pStream->Release();
	}
	CLOSE_HANDLE(m_hEvent);
	m_trackedId = NUI_SKELETON_INVALID_TRACKING_ID;
	m_frameData.Reset();
}

DWORD BackGroudRemvoedData::GetTrackedId() const
{
	return m_trackedId;
}

const FrameData* BackGroudRemvoedData::GetFrameData() const
{
	return &m_frameData;
}

InteractionData::InteractionData()
	: m_pStream(NULL)
	, m_pClient(NULL)
	, m_hEvent(INVALID_HANDLE_VALUE)
{
	
}

void InteractionData::Reset()
{
	if (m_pStream)
	{
		m_pStream->Disable();
		m_pStream->Release();
	}

	delete m_pClient;
	CLOSE_HANDLE(m_hEvent);
	m_userIdMap.clear();
}

UINT InteractionData::GetCount() const
{
	return m_userIdMap.size();
}

const NUI_USER_INFO* InteractionData::GetUserInfoByTrackedId(DWORD index) const
{
	SkeletonIdMap::const_iterator itr = m_userIdMap.find(index);
	if (itr != m_userIdMap.end())
		return &m_userInfos[itr->second];
	return NULL;
}

DWORD InteractionData::GetTrackedId(UINT uIndex) const
{
	if (uIndex < m_userIdMap.size())
	{
		SkeletonIdMap::const_iterator itr = m_userIdMap.begin();
		while (uIndex && itr != m_userIdMap.end())
		{
			--uIndex;
			++itr;
		}
		return itr->first;
	}
	return NUI_SKELETON_INVALID_TRACKING_ID;
}

ImageData::ImageData()
	: FrameData()
	, m_hStream(INVALID_HANDLE_VALUE)
	, m_hEvent(INVALID_HANDLE_VALUE)
{

}

void ImageData::Reset()
{
	m_hStream = INVALID_HANDLE_VALUE;
	CLOSE_HANDLE(m_hEvent);
	FrameData::Reset();
}

SkeletonData::SkeletonData()
	: m_hEvent(INVALID_HANDLE_VALUE)
{
	m_timeStamp.QuadPart = -1;
}

void SkeletonData::Reset()
{
	CLOSE_HANDLE(m_hEvent);
	m_timeStamp.QuadPart = -1;
	m_fullTrackedMap.clear();
	m_unFullTrackedMap.clear();
}

UINT SkeletonData::GetFullSkeletonCount() const
{
	return m_fullTrackedMap.size();
}

UINT SkeletonData::GetPositionOnlyCount() const
{
	return m_unFullTrackedMap.size();
}

UINT SkeletonData::GetSkeletonCount() const
{
	return m_fullTrackedMap.size() + m_unFullTrackedMap.size();
}

DWORD SkeletonData::GetPositionOnlyTrackedId(UINT uIndex) const
{
	if (uIndex < m_unFullTrackedMap.size())
	{
		SkeletonIdMap::const_iterator itr = m_unFullTrackedMap.begin();
		while (uIndex && itr != m_unFullTrackedMap.end())
		{
			--uIndex;
			++itr;
		}
		return itr->first;
	}
	return NUI_SKELETON_INVALID_TRACKING_ID;
}

const NUI_SKELETON_DATA* SkeletonData::GetSkeletonIndexByTrackedId(DWORD trackedId) const
{
	SkeletonIdMap::const_iterator itr = m_fullTrackedMap.find(trackedId);
	if (itr == m_fullTrackedMap.end())
		return &(m_dataArray[itr->second]);
	
	itr = m_unFullTrackedMap.find(trackedId);
	if (itr != m_unFullTrackedMap.end())
		return &(m_dataArray[itr->second]);
	return NULL;
}

DWORD SkeletonData::GetFullTrackedId(UINT uIndex) const
{
	if (uIndex < m_unFullTrackedMap.size())
	{
		SkeletonIdMap::const_iterator itr = m_fullTrackedMap.begin();
		while (uIndex && itr != m_fullTrackedMap.end())
		{
			--uIndex;
			++itr;
		}
		return itr->first;
	}
	return NUI_SKELETON_INVALID_TRACKING_ID;
}

bool SkeletonData::ExistFullTrackedID(DWORD trackedId) const
{
	return m_fullTrackedMap.find(trackedId) != m_fullTrackedMap.end();
}
