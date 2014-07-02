#include "stdafx.h"
#include "KinectApi.h"
#include "..\kinectLogic\KinectContext.h"
#include "..\motionLogic\MotionRecognitionEngine.h"
#include "..\motionLogic\MotionAvatar.h"
#include "..\kinectLogic\CoordinateMapper.h"
#include "..\RendingPlugin\RendingPlugin.h"

SensorContext* CKinectWapper::m_pNuiContext = NULL;

CKinectWapper::CKinectWapper()
{
	
}

CKinectWapper::~CKinectWapper()
{
	s->close();
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
	if (FAILED(Instance()->InitSensor(flag, &hr)))
		hr = SUCCEEDED_FALSE;
	printf("init ok\n");
	s = new ofstream("to.txt");
	s->clear();
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

	if (Instance()->IsColorEnabled() 
		&& WAIT_OBJECT_0 == ::WaitForSingleObject(Instance()->GetColorData()->m_hEvent, 0))
		hrColor = Instance()->ProcessColor(&rtColor);

	if (Instance()->IsDepthEnabled() 
		&& WAIT_OBJECT_0 == ::WaitForSingleObject(Instance()->GetDepthData()->m_hEvent, 0))
		hrDepth = Instance()->ProcessDepth(&rtDepth);

	if (Instance()->IsSkeletonEnabled()
		&& WAIT_OBJECT_0 == ::WaitForSingleObject(Instance()->GetSkeletonData()->m_hEvent, 0))
		hrSk = Instance()->ProcessSkeleton(&rtSk);
	
	if (Instance()->GetBackgroundRemovedCount() > 0)
	{
		Instance()->ProcessAllBackgroundRemoved(&rtBg);
	}

	if (Instance()->IsInteractionEnabled()
		&& WAIT_OBJECT_0 == ::WaitForSingleObject(Instance()->GetSkeletonData()->m_hEvent, 0))
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

const byte* CKinectWapper::NuiGetBackgroundRemovedImage(UINT trackedId, OUT int* size)
{
	CriticalSectionScope scope(&Instance()->m_backGrmCriSec);
	const BackGroudRemvoedData* pData = Instance()->GetBackgroundRemovedData(trackedId);
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

bool CKinectWapper::NuiGetSkeletonTransform(UINT player, int joint, OUT Vector4* SkeletonTransform)
{
	CriticalSectionScope scope(&Instance()->m_skeletonCriSec);
	if (!SkeletonTransform)
		return false;

	SkeletonTransform->x = 0.f;
	SkeletonTransform->y = 0.f;
	SkeletonTransform->z = 0.f;
	SkeletonTransform->w = 0.f;

	const SkeletonData* pSkeletonData = Instance()->GetSkeletonData();
	if (!pSkeletonData)
		return false;

	const NUI_SKELETON_DATA* pData = pSkeletonData->GetSkeletonIndexByTrackedId(player);
	if (!pData)
		return false;
	if (pData->eSkeletonPositionTrackingState[joint] == NUI_SKELETON_POSITION_TRACKED)
	{
		*SkeletonTransform = pData->SkeletonPositions[joint];
		return true;
	}
	
	//Instance()->TransformCoordinates(&skTrans);
	return false;
}

bool CKinectWapper::NuiGetHandInfo(UINT player, bool bLeft, OUT SenLogic::KUseInfo* pHand)
{
	CriticalSectionScope scope(&Instance()->m_interActCriSec);
	if ( pHand == NULL)
		return false;

	const NUI_USER_INFO* pUserInfo = Instance()->GetInteractionData()->GetUserInfoByTrackedId(player);
	if (!pUserInfo)
		return false;

	const NUI_HANDPOINTER_INFO* pHandPintInfo = pUserInfo->HandPointerInfos;
	const NUI_HANDPOINTER_INFO *pLeft = NULL, *pRight = NULL;

	for (int i = 0; i < NUI_USER_HANDPOINTER_COUNT; ++i)
	{
		switch (pHandPintInfo[i].HandType)
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

	//取到左手
	if (bLeft && pLeft)
	{
		pHand->handEventType = pLeft->HandEventType;
		pHand->x = pLeft->RawX;
		pHand->y = pLeft->RawY;
		pHand->z = pLeft->RawZ;
		return true;
	}
	else if (!bLeft && pRight)//取到右手
	{
		pHand->handEventType = pRight->HandEventType;
		pHand->x = pRight->RawX;
		pHand->y = pRight->RawY;
		pHand->z = pRight->RawZ;
		return true;
	}
	
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
	CriticalSectionScope scope(&Instance()->m_skeletonCriSec);
	return Instance()->GetSkeletonData()->GetFullSkeletonCount();
}

bool CKinectWapper::NuiExistPlayer()
{
	CriticalSectionScope scope(&Instance()->m_skeletonCriSec);
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
		} 
		else {
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
		
		if (NuiExistPlayer())
		{
			int trackId = NuiGetSkeletonId(0);
			SenLogic::KUseInfo leftHand = { 0 }, rightHand = { 0 };
			
			if (NuiGetHandInfo(trackId, true, &rightHand))
			{
				printf("-------------------right hand: ");
				if (rightHand.handEventType == 0)
					printf("none\n");
				else if (rightHand.handEventType == 1)
					printf("grip\n");
				else
					printf("release\n");
			}
		}

		system("CLS");
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
	CriticalSectionScope scope(&Instance()->m_backGrmCriSec);
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
	CriticalSectionScope scope(&Instance()->m_interActCriSec);
	return Instance()->GetInteractionData()->GetCount();
}

void CKinectWapper::GetColorResolution(int* width, int* height)
{
	DWORD w = 0, h = 0;
	NuiImageResolutionToSize(Instance()->cColorResolution, w, h);
	if (width)
		*width = w;
	if (height)
		*height = h;
}

void CKinectWapper::GetDepthResolution(int* width, int* height)
{
	DWORD w = 0, h = 0;
	NuiImageResolutionToSize(Instance()->cDepthResolution, w, h);
	if (width)
		*width = w;
	if (height)
		*height = h;
}

UINT CKinectWapper::NuiGetSkeletonId(UINT i)
{
	CriticalSectionScope scope(&Instance()->m_skeletonCriSec);
	return Instance()->GetSkeletonData()->GetFullTrackedId(i);
}

int CKinectWapper::BgColor(int i)
{
	return Instance()->bgColor[i];
}

int CKinectWapper::BgDepth(int i)
{
	return Instance()->bgDepth[i];
}

int CKinectWapper::BgSk(int i)
{
	return Instance()->bgSk[i];
}

int CKinectWapper::BgBg(int i)
{
	return Instance()->bgBg[i];
}

DWORD WINAPI CKinectWapper::UpdateProc(IN LPVOID lpParameter)
{
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

	while (!g_bStopThread)
	{
		DWORD ret = ::WaitForMultipleObjects(eventVec.size(), eventVec.data(), FALSE, INFINITE);
		NuiUpdate();

		if (NuiGetFullSkeletonCount() <= 0)
		{
			continue;
		}
		int id = NuiGetSkeletonId(0);
		Vector4 footLeft;
		Vector4 clip;
		if (NuiGetSkeletonTransform(id, NUI_SKELETON_POSITION_FOOT_LEFT, &footLeft)
			&& NuiGetFloorClipPlane(&clip) && clip.y != 0 && clip.z != 0 && clip.w != 0)
		{
			if (std::abs(footLeft.z * 100.0 - a1) < 0.001 &&
				std::abs(footLeft.y * (-100) - b1) < 0.001 &&
				std::abs(clip.w * 100 - c1) < 0.001)
			{
				a1 = footLeft.z * 100;
				b1 = footLeft.y * (-100);
				c1 = clip.w * 100;
				continue;
			}
				
			a1 = footLeft.z * 100;
			b1 = footLeft.y * (-100);
			c1 = clip.w * 100;
			*s << a1 << " " << b1 << " " << c1 << endl;
		}

		/*printf("sk diff = %d\n", Instance()->bgSk[0] - Instance()->bgSk[1]);
		printf("bg[%d, %d, %d]\n",
			Instance()->bgBg[0],
			Instance()->bgBg[1],
			Instance()->bgBg[0] - Instance()->bgBg[1]);
		system("CLS");*/
	}
	return S_OK;
}

HANDLE CKinectWapper::StartProcessData()
{
	if (g_hThread == INVALID_HANDLE_VALUE)
	{
		g_hThread = CreateThread(NULL, 0, CKinectWapper::UpdateProc, NULL, 0, NULL);
		g_bStopThread = false;
	}
	return g_hThread;
}

void CKinectWapper::StopProcessData()
{
	if (g_hThread != INVALID_HANDLE_VALUE)
		CloseHandle(g_hThread);
	g_bStopThread = true;
}

bool CKinectWapper::NuiGetFloorClipPlane(OUT Vector4* vFloorClipPlane)
{
	if (vFloorClipPlane == NULL)
		return false;

	CriticalSectionScope scope(&Instance()->m_skeletonCriSec);
	*vFloorClipPlane = Instance()->GetSkeletonData()->GetFloorClipPlane();
	return true;
}

float CKinectWapper::c1;

float CKinectWapper::b1;

float CKinectWapper::a1;

bool CKinectWapper::g_bStopThread = true;

HANDLE CKinectWapper::g_hThread = INVALID_HANDLE_VALUE;
ofstream* CKinectWapper::s = NULL;
