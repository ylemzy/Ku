

#include "stdafx.h"
#include "KUInterface.h"  //DLL export declarations
#include "NuiContext.h"   //NUI object


SensorContext* CKinectWapper::m_pNuiContext = NULL;

CKinectWapper::CKinectWapper()
{

}

CKinectWapper::~CKinectWapper()
{
	NuiUnInitContext();
}

int CKinectWapper::NuiInitContext(bool useColor, bool useDepth, bool useSkeleton)
{
	RUNTIME_RESULT hr = SUCCEEDED_OK;
	DWORD flag = 0;
	if (useColor)
		flag |= NUI_INITIALIZE_FLAG_USES_COLOR;

	if (useDepth)
		flag |= NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;

	if (useSkeleton)
		flag |= NUI_INITIALIZE_FLAG_USES_SKELETON;
	Instance()->InitSensor(flag, &hr);
	return hr;
}

int CKinectWapper::NuiEnableBackgroundRemoved(bool bOpen)
{
	RUNTIME_RESULT hr = SUCCEEDED_OK;
	Instance()->SetEnableBackgroundRemovedColorStream(bOpen, &hr);
	return hr;
}

int CKinectWapper::NuiEnableInteraction(bool bOpen)
{
	RUNTIME_RESULT hr = SUCCEEDED_OK;
	Instance()->SetEnableInteractioin(bOpen, &hr);
	return hr;
}

int CKinectWapper::NuiUpdate()
{
	RUNTIME_RESULT rtColor = SUCCEEDED_OK, rtDepth = SUCCEEDED_OK, rtSk = SUCCEEDED_OK, rtBg = SUCCEEDED_OK, rtIa = SUCCEEDED_OK;
	HRESULT hrColor = S_OK, hrDepth = S_OK, hrSk = S_OK, hrBg = S_OK, hrIa = S_OK;

	if (Instance()->IsColorEnabled())
		hrColor = Instance()->ProcessColor(&rtColor);

	if (Instance()->IsDepthEnabled())
		hrDepth = Instance()->ProcessDepth(&rtDepth);

	if (Instance()->IsSkeletonEnabled())
		hrSk = Instance()->ProcessSkeleton(&rtSk);

	if (Instance()->IsBackgroundRemovedEnabled())
		hrBg = Instance()->ProcessBackgroundRemoved(&rtBg);

	if (Instance()->IsInteractionEnabled())
		hrIa = Instance()->ProcessInteraction(&rtIa);

	//按顺序优先判断，骨架以及背景去除依赖于深度
	if (FAILED(hrColor))
	{
		return rtColor;
	}
	else if (FAILED(hrDepth))
	{
		return rtDepth;
	}
	else if (FAILED(hrSk))
	{
		return rtSk;
	}
	else if (FAILED(hrBg))
	{
		return rtBg;
	}
	else if (FAILED(hrIa))
	{
		return rtIa;
	}

	return SUCCEEDED_OK;
}

void CKinectWapper::NuiUnInitContext()
{
	Instance()->UnInitSensor();
}

byte* CKinectWapper::NuiGetTextureImage(OUT int* size)
{
	*size = Instance()->m_nImageSize;
	return Instance()->m_pImageData;
}

byte* CKinectWapper::NuiGetDepthImage(OUT int* size)
{
	*size = Instance()->m_nDepthSize;
	return Instance()->m_pDepthData;
}

const byte* CKinectWapper::NuiGetBackgroundRemovedImage(OUT int* size)
{
	*size = Instance()->m_nBackgroundRemovedSize;
	return Instance()->m_pBackgroundRemovedData;
}

void CKinectWapper::NuiGetSkeletonTransform(int player, int joint, OUT KVector4* SkeletonTransform)
{
	KVector4 &skTrans = *SkeletonTransform;

	if (player == 1) {
		if(Instance()->m_bSkeletonValid) {
			skTrans.x = Instance()->m_skData.SkeletonPositions[joint].x;
			skTrans.y = Instance()->m_skData.SkeletonPositions[joint].y;
			skTrans.z = Instance()->m_skData.SkeletonPositions[joint].z;
			skTrans.w = Instance()->m_skData.SkeletonPositions[joint].w;
		} else {
			skTrans.x = 0.0f;
			skTrans.y = 0.0f;
			skTrans.z = 0.0f;
			skTrans.w = 0.0f;
		}
	} else if (player == 2) {
		if(Instance()->m_bSkeletonValid) {
			skTrans.x = Instance()->m_skData2.SkeletonPositions[joint].x;
			skTrans.y = Instance()->m_skData2.SkeletonPositions[joint].y;
			skTrans.z = Instance()->m_skData2.SkeletonPositions[joint].z;
			skTrans.w = Instance()->m_skData2.SkeletonPositions[joint].w;
		} else {
			skTrans.x = 0.0f;
			skTrans.y = 0.0f;
			skTrans.z = 0.0f;
			skTrans.w = 0.0f;
		}
	} else {
		skTrans.x = 0.0f;
		skTrans.y = 0.0f;
		skTrans.z = 0.0f;
		skTrans.w = 0.0f;
	}

	Instance()->TransformCoordinates(&skTrans);
}

bool CKinectWapper::NuiGetUseInfo(int player, OUT KUseInfo* pLeftHand, OUT KUseInfo* pRightHand)
{
	assert(player == Instance()->m_mainUserInfo.SkeletonTrackingId);
	if (pLeftHand == NULL && pRightHand == NULL)
		return false;

	const NUI_HANDPOINTER_INFO* pHandPintInfo = Instance()->m_mainUserInfo.HandPointerInfos;
	const NUI_HANDPOINTER_INFO *pLeft = NULL, *pRight = NULL;

	for (int i = 0; i < NUI_USER_HANDPOINTER_COUNT; ++i)
	{
		switch (pHandPintInfo[0].HandType)
		{
		case NUI_HAND_TYPE_NONE:
			break;
		case NUI_HAND_TYPE_LEFT:
			pLeft = pHandPintInfo;
			break;
		case NUI_HAND_TYPE_RIGHT:
			pRight = pHandPintInfo;
			break;
		}
	}

	if (pLeft)
	{
		pLeftHand->handEventType = pLeft->HandEventType;
		pLeftHand->x = pLeft->RawX;
		pLeftHand->y = pLeft->RawY;
		pLeftHand->z = pLeft->RawZ;
	}

	if (pRight)
	{
		pRightHand->handEventType = pRight->HandEventType;
		pRightHand->x = pRight->RawX;
		pRightHand->y = pRight->RawY;
		pRightHand->z = pRight->RawZ;
	}

	return pLeft || pRight;
}

void CKinectWapper::NuiGetColorImageSize(int * width, int * height)
{
	if (width == NULL || height == NULL)
		return;

	*width = Instance()->m_colorWidth;
	*height = Instance()->m_colorHeight;
}

void CKinectWapper::NuiGetDepthImageSize(int* width, int* height)
{
	if (width == NULL || height == NULL)
		return;

	*width = Instance()->m_depthWidth;
	*height = Instance()->m_depthHeight;
}

int CKinectWapper::NuiTrackedIndex()
{
	return Instance()->m_skTackedId;
}

bool CKinectWapper::NuiExistPlayer()
{
	return Instance()->m_skTackedId != NUI_SKELETON_INVALID_TRACKING_ID; 
}

int CKinectWapper::NuiGetMainPlayerId()
{
	return Instance()->m_mainUserInfo.SkeletonTrackingId;
}

void CKinectWapper::NuiGetCameraAngle(OUT float* angle)
{
	*angle = Instance()->angle;
}

bool CKinectWapper::NuiSetCameraAngle(int angle)
{
	HRESULT hr;

	if (angle >= -27 && angle <= 27) {
		hr = Instance()->SetCameraAngle((long)angle);
		if (FAILED(hr)) {
			return false;
		} else {
			Instance()->angle = (long)angle;
			return true;
		}
	} else {
		return false;
	}
}

void CKinectWapper::NuiRunTest(bool useColor, bool useDepth, bool useSkeleton)
{
	NuiInitContext(useColor, useDepth, useSkeleton);
	//NuiEnableInteraction(true);
	NuiEnableBackgroundRemoved(true);
	HANDLE hColor = Instance()->m_hNextColorFrameEvent;
	HANDLE hDepth = Instance()->m_hNextDepthFrameEvent;
	HANDLE hSkeleton = Instance()->m_hNextSkeletonFrameEvent;
	HANDLE hInteraction = Instance()->m_hNextInteractionFrameEvent;
	HANDLE hBg = Instance()->m_hNextBackgroundRemovedFrameEvent;
	const HANDLE hEvents[] = 
	{
		hColor, hDepth, hSkeleton, hInteraction, hBg
	};

	while (1)
	{
		DWORD ret = ::WaitForMultipleObjects(_countof(hEvents), hEvents, FALSE, INFINITE);
		/*if (WAIT_OBJECT_0 == ::WaitForSingleObject(hColor, INFINITE))
		::ResetEvent(Instance()->m_hNextDepthFrameEvent);*/
		NuiUpdate();
	}
}

SensorContext* CKinectWapper::Instance()
{
	if (m_pNuiContext == NULL)
	{
		m_pNuiContext = new SensorContext();
	}
	return m_pNuiContext;
}
