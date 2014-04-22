

#include "stdafx.h"
#include "KinectApi.h"
#include "..\kinectLogic\KinectContext.h"
#include "..\motionLogic\MotionRecognitionEngine.h"
#include "..\motionLogic\MotionAvatar.h"
#include "..\kinectLogic\CoordinateMapper.h"

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
	printf("init ok\n");
	return hr;
}

int CKinectWapper::NuSetBackgroundRemovedCount(UINT num)
{
	RUNTIME_RESULT hr = SUCCEEDED_OK;
	Instance()->SetBackgroundRemovedCount(num, &hr);
	return hr;
}

int CKinectWapper::NuiSetInteractionCount(UINT num)
{
	RUNTIME_RESULT hr = SUCCEEDED_OK;
	Instance()->SetInteractionCount(num, &hr);
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

	
	UINT num = Instance()->GetBackgroundRemovedCount();
	
	for (UINT i = 0; i < num; ++i)
	{
		Instance()->ProcessBackgroundRemoved(i, &rtBg);
	}
	if (Instance()->IsInteractionEnabled())
	{
		hrIa = Instance()->ProcessInteraction(&rtIa);
	}
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

const byte* CKinectWapper::NuiGetTextureImage(OUT int* size)
{
	*size = Instance()->GetColorData()->GetSize(true);
	return Instance()->GetColorData()->GetData();
}

const byte* CKinectWapper::NuiGetDepthImage(OUT int* size)
{
	*size = Instance()->GetDepthData()->GetSize(true);
	return Instance()->GetDepthData()->GetData();
}

const byte* CKinectWapper::NuiGetBackgroundRemovedImage(UINT player, OUT int* size)
{
	const BackGroudRemvoedData* pData = Instance()->GetBackgroundRemovedData(player);
	if (!pData)
	{
		if (size)
			*size = 0;
		return NULL;
	}
	if (size)
		*size = pData->GetFrameData()->GetSize(true);
	return pData->GetFrameData()->GetData();
}

void CKinectWapper::NuiGetSkeletonTransform(UINT player, int joint, OUT Vector4* SkeletonTransform)
{
	if (!SkeletonTransform)
		return;

	SkeletonTransform->x = 0.f;
	SkeletonTransform->y = 0.f;
	SkeletonTransform->z = 0.f;
	SkeletonTransform->w = 0.f;

	const SkeletonData* pSkeletonData = Instance()->GetSkeletonData();
	if (player < 0 || player >= pSkeletonData->GetFullSkeletonCount())
	{
		return;
	}

	const NUI_SKELETON_DATA* pData = pSkeletonData->GetSkeletonIndexByTrackedId(player);
	*SkeletonTransform = pData->SkeletonPositions[joint];
	//Instance()->TransformCoordinates(&skTrans);
}

bool CKinectWapper::NuiGetUseInfo(UINT player, OUT SenLogic::KUseInfo* pLeftHand, OUT SenLogic::KUseInfo* pRightHand)
{
	assert(player < Instance()->GetInteractionData()->GetCount());
	if (pLeftHand == NULL && pRightHand == NULL)
		return false;

	if (player < Instance()->GetInteractionData()->GetCount())
		return false;

	const NUI_HANDPOINTER_INFO* pHandPintInfo = Instance()->GetInteractionData()->GetUserInfoByTrackedId(player)->HandPointerInfos;
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

	return false;
}

void CKinectWapper::NuiGetColorImageSize(int * width, int * height)
{
	if (width == NULL || height == NULL)
		return;

	*width = Instance()->GetColorData()->GetWidth();
	*height = Instance()->GetColorData()->GetHeight();
}

void CKinectWapper::NuiGetDepthImageSize(int* width, int* height)
{
	if (width == NULL || height == NULL)
		return;

	*width = Instance()->GetDepthData()->GetWidth();
	*height = Instance()->GetDepthData()->GetHeight();
}

UINT CKinectWapper::NuiGetFullSkeletonCount()
{
	return Instance()->GetSkeletonData()->GetFullSkeletonCount();
}

bool CKinectWapper::NuiExistPlayer()
{
	return Instance()->GetSkeletonData()->GetSkeletonCount() > 0;
}

void CKinectWapper::NuiGetCameraAngle(OUT float* angle)
{
	/**angle = Instance()->nKinecAngle;*/
}

bool CKinectWapper::NuiSetCameraAngle(int angle)
{
	HRESULT hr;

	/*if (angle >= -27 && angle <= 27) {
		hr = Instance()->SetCameraAngle((long)angle);
		if (FAILED(hr)) {
			return false;
		} else {
			Instance()->nKinecAngle = (long)angle;
			return true;
		}
	} else {
		return false;
	}*/
	return false;
}

void CKinectWapper::NuiRunTest(bool useColor, bool useDepth, bool useSkeleton)
{
	NuiInitContext(useColor, useDepth, useSkeleton);
	NuiUpdate();
	NuiSetInteractionCount(2);
	NuSetBackgroundRemovedCount(2);

	vector<HANDLE> eventVec;
	HANDLE hColor = Instance()->GetColorData()->m_hEvent;
	HANDLE hDepth = Instance()->GetDepthData()->m_hEvent;
	HANDLE hSkeleton = Instance()->GetSkeletonData()->m_hEvent;
	eventVec.push_back(hColor);
	eventVec.push_back(hDepth);
	eventVec.push_back(hSkeleton);

	HANDLE hInteraction = Instance()->GetInteractionData()->m_hEvent;
	eventVec.push_back(hInteraction);
	for (UINT i = 0; i < Instance()->m_backGrmDataVec.size(); ++i)
	{
		eventVec.push_back(Instance()->m_backGrmDataVec[i].m_hEvent);
	}

	MotionAvatar avatar;
	avatar.AddJoint(WRIST_RIGHT);
	while (1)
	{
		
		DWORD ret = ::WaitForMultipleObjects(eventVec.size(), eventVec.data(), FALSE, INFINITE);
		/*if (WAIT_OBJECT_0 == ::WaitForSingleObject(hColor, INFINITE))
		::ResetEvent(Instance()->m_hNextDepthFrameEvent);*/
		NuiUpdate();

		system("CLS");
		//if (NuiExistPlayer())
		//{
		//	const NUI_SKELETON_DATA* pData = &(Instance()->m_skData);
		//	if (pData->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] == NUI_SKELETON_POSITION_NOT_TRACKED)
		//		continue;
		//	Vector4 v = pData->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT];
		//	int x = 0, y = 0;
		//	MapSkeletonToColor(v, &x, &y);
		//	printf("%f %f %f ", v.x, v.y, v.z);
		//	printf("%d %d\n", x, y);

		//	int nSize = 0;
		//	NuiGetBackgroundRemovedImage(&nSize);
		//	//avatar.UpdateData(pData);
		//}
	}
	NuiUnInitContext();
	
}

SensorContext* CKinectWapper::Instance()
{
	if (m_pNuiContext == NULL)
	{
		m_pNuiContext = new SensorContext();
	}
	return m_pNuiContext;
}

void CKinectWapper::RunAngleTest()
{
	Vector4 start = {1, 0, 0};
	Vector4 end = start;
	bool halfx = false, halfy = false;
	float fx = -0.1, fy = 0.1;
	do
	{
		if (start.x + fx < -1)
			fx = 0.1;

		if (start.x + fx > 1)
			fx = -0.1;

		if (start.y + fy > 1)
			fy = -0.1;

		if (start.y + fy < -1)
			fy = 0.1;

		start.x += fx;
		start.y += fy;

		float angle = IntersectionAngle(start, end);
		printf("%f\n", angle);

	}while(!CompareDirection(start, end));
}

bool CKinectWapper::MapSkeletonToColor(Vector4 vec, int* x, int *y)
{
	CoordinateMapper mapper(Instance());
	NUI_COLOR_IMAGE_POINT point = {0};
	HRESULT hr = mapper.MapSkeletonPointToColorPoint(&vec, &point);
	if (hr != S_OK)
		return false;

	if (x)
		*x = point.x;

	if (y)
		*y = point.y;
	return true;
}

UINT CKinectWapper::NuiGetBackgroundRemovedCount()
{
	return Instance()->GetBackgroundRemovedCount();
}

const byte* CKinectWapper::NuiGetBackgroundRemovedComposed(OUT int* size)
{
	if (Instance()->IsBackgroundRemovedComposed())
	{
		if (size)
			*size = Instance()->GetBackgroundRemovedComposed()->GetSize(true);
		return Instance()->GetBackgroundRemovedComposed()->GetData();
	}
	return NULL;
}

void CKinectWapper::NuiSetBackgroundRemovedComposed(bool bComposed)
{
	Instance()->SetBackgroundReomovedComposed(bComposed);
}

bool CKinectWapper::NuiIsBackgroundRemovedComposed()
{
	return Instance()->IsBackgroundRemovedComposed();
}

UINT CKinectWapper::NuiGetInteractionCount()
{
	return Instance()->GetInteractionData()->GetCount();
}
