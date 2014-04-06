/************************************************************************
*				   This file contains methods available					*
*				to System.Runtime.InteropServies.DllImport				*
*																		*
*						  Author: Andrew DeVine							*
*								  2013									*
************************************************************************/

#include "stdafx.h"
#include "KUInterface.h"  //DLL export declarations
#include "NuiContext.h"      //NUI object


/************************************************************************
						NUI CONTEXT MANAGEMENT
************************************************************************/


class ContextOwner
{
public:
	static SensorContext* Instance()
	{
		if (m_pNuiContext == NULL)
		{
			m_pNuiContext = new SensorContext();
		}
		return m_pNuiContext;
	}

	static void ReleaseInstance()
	{
		delete m_pNuiContext;
		m_pNuiContext = NULL;
	}
private:
	static SensorContext* m_pNuiContext;
};

SensorContext* ContextOwner::m_pNuiContext = NULL;

//Initializes NUI Context
int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton) 
{
	RUNTIME_RESULT hr = SUCCEEDED_OK;
	DWORD flag = 0;
	if (useColor)
		flag |= NUI_INITIALIZE_FLAG_USES_COLOR;
	
	if (useDepth)
		flag |= NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;

	if (useSkeleton)
		flag |= NUI_INITIALIZE_FLAG_USES_SKELETON;
	ContextOwner::Instance()->InitSensor(flag, &hr);
	return hr;
}

//Updates Skeleton, Image Data
int NuiUpdate() 
{
	RUNTIME_RESULT rtColor = SUCCEEDED_OK, rtDepth = SUCCEEDED_OK, rtSk = SUCCEEDED_OK, rtBg = SUCCEEDED_OK, rtIa = SUCCEEDED_OK;
	HRESULT hrColor = S_OK, hrDepth = S_OK, hrSk = S_OK, hrBg = S_OK, hrIa = S_OK;
	
	hrColor = ContextOwner::Instance()->ProcessColor(&rtColor);

	hrDepth = ContextOwner::Instance()->ProcessDepth(&rtDepth);

	//if (SUCCEEDED(hrDepth))
		hrSk = ContextOwner::Instance()->ProcessSkeleton(&rtSk);
	
	if (SUCCEEDED(hrDepth))
		hrBg = ContextOwner::Instance()->ProcessBackgroundRemoved(&rtBg);

	//if (SUCCEEDED(hrDepth) && SUCCEEDED(hrSk))
		hrIa = ContextOwner::Instance()->ProcessInteraction(&rtIa);

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


//Closes NUI Context
void NuiUnInitContext() 
{
	ContextOwner::ReleaseInstance();
}

//Creates Custom KUVector4 object for export to Unity
void NuiGetSkeletonTransform(int player, int joint, OUT KUVector4* SkeletonTransform) {

	KUVector4 &skTrans = *SkeletonTransform;

	if (player == 1) {
		if(ContextOwner::Instance()->m_bSkeletonValid) {
			skTrans.x = ContextOwner::Instance()->m_skData.SkeletonPositions[joint].x;
			skTrans.y = ContextOwner::Instance()->m_skData.SkeletonPositions[joint].y;
			skTrans.z = ContextOwner::Instance()->m_skData.SkeletonPositions[joint].z;
			skTrans.w = ContextOwner::Instance()->m_skData.SkeletonPositions[joint].w;
		} else {
			skTrans.x = 0.0f;
			skTrans.y = 0.0f;
			skTrans.z = 0.0f;
			skTrans.w = 0.0f;
		}
	} else if (player == 2) {
		if(ContextOwner::Instance()->m_bSkeletonValid) {
			skTrans.x = ContextOwner::Instance()->m_skData2.SkeletonPositions[joint].x;
			skTrans.y = ContextOwner::Instance()->m_skData2.SkeletonPositions[joint].y;
			skTrans.z = ContextOwner::Instance()->m_skData2.SkeletonPositions[joint].z;
			skTrans.w = ContextOwner::Instance()->m_skData2.SkeletonPositions[joint].w;
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

	ContextOwner::Instance()->TransformCoordinates(&skTrans);
}

//Gets color texture image
byte* NuiGetTextureImage(OUT int* size) 
{
	*size = ContextOwner::Instance()->m_nImageSize;
	return ContextOwner::Instance()->m_pImageData;
}

//Gets depth image
byte* NuiGetDepthImage(OUT int* size) 
{
	*size = ContextOwner::Instance()->m_nDepthSize;
	return ContextOwner::Instance()->m_pDepthData;
}

//Gets current camera angle
void NuiGetCameraAngle(OUT float* angle) 
{

	*angle = ContextOwner::Instance()->angle;
}

const byte* NuiGetBackgroundRemovedImage(OUT int* size)
{
	*size = ContextOwner::Instance()->m_nBackgroundRemovedSize;
	return ContextOwner::Instance()->m_pBackgroundRemovedData;
}

bool NuiSetCameraAngle(int angle) {

	HRESULT hr;
	
	if (angle >= -27 && angle <= 27) {
		hr = ContextOwner::Instance()->SetCameraAngle((long)angle);
		if (FAILED(hr)) {
			return false;
		} else {
			ContextOwner::Instance()->angle = (long)angle;
			return true;
		}
	} else {
		return false;
	}
}

void NuiGetColorImageSize(int * width, int * height)
{
	if (width == NULL || height == NULL)
		return;

	*width = ContextOwner::Instance()->m_colorWidth;
	*height = ContextOwner::Instance()->m_colorHeight;
}

void NuiGetDepthImageSize(int* width, int* height)
{
	if (width == NULL || height == NULL)
		return;

	*width = ContextOwner::Instance()->m_depthWidth;
	*height = ContextOwner::Instance()->m_depthHeight;
}

int NuiTrackedIndex()
{
	return ContextOwner::Instance()->m_skTackedId;
}

bool NuiExistPlayer()
{
	return ContextOwner::Instance()->m_skTackedId != NUI_SKELETON_INVALID_TRACKING_ID; 
}

int NuiEnableBackgroundRemoved(bool bOpen)
{
	RUNTIME_RESULT hr = SUCCEEDED_OK;
	ContextOwner::Instance()->SetEnableBackgroundRemovedColorStream(bOpen, &hr);
	return hr;
}


int NuiEnableInteraction(bool bOpen)
{
	RUNTIME_RESULT hr = SUCCEEDED_OK;
	ContextOwner::Instance()->SetEnableInteractioin(bOpen, &hr);
	return hr;
}


bool NuiGetUseInfo(int player, OUT _KUUseInfo* pLeftHand, OUT _KUUseInfo* pRightHand)
{
	if (pLeftHand == NULL && pRightHand == NULL)
		return false;

	NUI_HANDPOINTER_INFO* pHandPintInfo = ContextOwner::Instance()->m_userInfo.HandPointerInfos;
	NUI_HANDPOINTER_INFO *pLeft = NULL, *pRight = NULL;

	switch (pHandPintInfo[0].HandType)
	{
	case NUI_HAND_TYPE_NONE:
		break;
	case NUI_HAND_TYPE_LEFT:
		pLeft = pHandPintInfo;
		pRight = pHandPintInfo + 1;
		break;
	case NUI_HAND_TYPE_RIGHT:
		pRight = pHandPintInfo;
		pLeft = pHandPintInfo + 1;
		break;
	}
	
	if (pLeft)
	{
		pLeftHand->handEventType = pLeft->HandEventType;
		pLeftHand->x = pLeft->RawX;
		pLeftHand->y = pLeft->RawY;
		pLeftHand->z = pLeft->RawZ;
		pLeftHand->w = pLeft->PressExtent;
	}

	if (pRight)
	{
		pRightHand->handEventType = pRight->HandEventType;
		pRightHand->x = pRight->RawX;
		pRightHand->y = pRight->RawY;
		pRightHand->z = pRight->RawZ;
		pRightHand->w = pRight->PressExtent;
	}

	return pLeft || pRight;
}

void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton)
{
	NuiInitContext(useColor, useDepth, useSkeleton);
	NuiEnableInteraction(true);

	HANDLE hColor = ContextOwner::Instance()->m_hNextColorFrameEvent;
	HANDLE hDepth = ContextOwner::Instance()->m_hNextDepthFrameEvent;
	HANDLE hSkeleton = ContextOwner::Instance()->m_hNextSkeletonFrameEvent;
	HANDLE hInteraction = ContextOwner::Instance()->m_hNextInteractionFrameEvent;
	const HANDLE hEvents[] = 
	{
		hColor, hDepth, hSkeleton, hInteraction
	};

	while (1)
	{
		DWORD ret = ::WaitForMultipleObjects(_countof(hEvents), hEvents, FALSE, INFINITE);
		/*if (WAIT_OBJECT_0 == ::WaitForSingleObject(hColor, INFINITE))
		::ResetEvent(ContextOwner::Instance()->m_hNextDepthFrameEvent);*/
		NuiUpdate();
	}
}
