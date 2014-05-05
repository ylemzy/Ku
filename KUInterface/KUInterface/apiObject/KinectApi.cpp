

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

	
	if (Instance()->GetBackgroundRemovedCount() > 0)
	{
		Instance()->ProcessAllBackgroundRemoved(&rtBg);
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

const byte* CKinectWapper::NuiGetBackgroundRemovedImage(UINT trackedId, OUT int* size)
{
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

bool CKinectWapper::NuiGetBackgroundRemovedTexture(UINT trackedId, void* texture)
{
	/*if (!texture)
		return false;*/
	const BackGroudRemvoedData* pData = Instance()->GetBackgroundRemovedData(trackedId);
	if (!pData)
	{
		return false;
	}
	DWORD width = 0, height = 0;
	pData->GetFrameData()->GetSize(width, height);
	RenderTexture(width, height, pData->GetFrameData()->GetData(), texture);
	return true;
}

bool CKinectWapper::NuiGetSkeletonTransform(UINT player, int joint, OUT Vector4* SkeletonTransform)
{
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
	if (pData->eSkeletonPositionTrackingState[joint] != NUI_SKELETON_POSITION_NOT_TRACKED)
	{
		*SkeletonTransform = pData->SkeletonPositions[joint];
	}
	
	//Instance()->TransformCoordinates(&skTrans);
	return true;
}

bool CKinectWapper::NuiGetUseInfo(UINT player, OUT SenLogic::KUseInfo* pLeftHand, OUT SenLogic::KUseInfo* pRightHand)
{
	if (pLeftHand == NULL && pRightHand == NULL)
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

	if (pLeft)
	{
		pLeftHand->handEventType = pLeft->HandEventType;
		pLeftHand->x = pLeft->RawX;
		pLeftHand->y = pLeft->RawY;
		pLeftHand->z = pLeft->RawZ;
	}
	else if (pLeftHand)
	{
		pLeftHand->handEventType = -1;//表示没有手
	}

	if (pRight)
	{
		pRightHand->handEventType = pRight->HandEventType;
		pRightHand->x = pRight->RawX;
		pRightHand->y = pRight->RawY;
		pRightHand->z = pRight->RawZ;
	}
	else if (pRightHand)
	{
		pRightHand->handEventType = -1;
	}
	return pLeft || pRight;
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
//测试用的代码
		if (NuiGetFullSkeletonCount())
		{
			int id = NuiGetSkeletonId(0);
		}

		if (NuiGetBackgroundRemovedCount())
		{
			int id = 0;
			id++;
		}

		int size = 0;
		NuiGetBackgroundRemovedImage(0, &size);
		if (NuiExistPlayer())
		{
			/*int id = NuiGetSkeletonId(0);
			NuiGetBackgroundRemoveduTexture(id, NULL);*/
			/*Vector4 trans = {0};
			NuiGetSkeletonTransform(id, NUI_SKELETON_POSITION_HEAD, &trans);
			printf("head position is %d %d %d", trans.x, trans.y, trans.z);*/

			int id = NuiGetSkeletonId(0);
			SenLogic::KUseInfo lHand = {0}, rHand = {0};
			NuiGetUseInfo(id, &lHand, &rHand);
			int a = 0;
			switch (rHand.handEventType)
			{
			case -1:
				a++;
				break;
			case 0:
				a++;
				break;
			case 1:
				a++;
				break;
			case 2:
				a++;
				break;
			}

		}
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
	return Instance()->GetSkeletonData()->GetFullTrackedId(i);
}

void CKinectWapper::FillTextureFromCode(DWORD width, DWORD height, int stride, unsigned char* dst)
{
	/*int totalLenght = width * height * 4;
	memcpy(dst, src, totalLenght);*/

	MessageBox(NULL, L"FillTextureFromCode", NULL, MB_OK);
	const float t = 1 * 4.0f;

	for (int y = 0; y < height; ++y)
	{
		unsigned char* ptr = dst;
		for (int x = 0; x < width; ++x)
		{
			// Simple oldskool "plasma effect", a bunch of combined sine waves
			int vv = int(
				(127.0f + (127.0f * sinf(x/7.0f+t))) +
				(127.0f + (127.0f * sinf(y/5.0f-t))) +
				(127.0f + (127.0f * sinf((x+y)/6.0f-t))) +
				(127.0f + (127.0f * sinf(sqrtf(float(x*x + y*y))/4.0f-t)))
				) / 4;

			// Write the texture pixel
			ptr[0] = vv;
			ptr[1] = vv;
			ptr[2] = vv;
			ptr[3] = vv;

			// To next pixel (our pixels are 4 bpp)
			ptr += 4;
		}

		// To next image row
		dst += stride;
	}
}

bool CKinectWapper::RenderTexture(DWORD width, DWORD height, const byte* src, void* texture)
{
	
	return true;
}

void CKinectWapper::UnityRenderEvent(int eventID)
{
	// Unknown graphics device type? Do nothing.
	MessageBox(NULL, L"ffffffffffffffff", NULL, MB_OK);
	if (g_DeviceType == -1)
		return;

	if (g_TexturePointer == NULL || g_textureTrackId == 0)
		return;

	MessageBox(NULL, L"rrrrrrrrr", NULL, MB_OK);
	DoRendering();
}

void CKinectWapper::DoRendering()
{
#if SUPPORT_OPENGL
	// OpenGL case
	if (g_DeviceType == kGfxRendererOpenGL)
	{
		
		// update native texture from code
		if (g_TexturePointer)
		{
			GLuint gltex = (GLuint)(size_t)(g_TexturePointer);
			glBindTexture (GL_TEXTURE_2D, gltex);
			int texWidth, texHeight;
			glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
			glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);

			unsigned char* data = new unsigned char[texWidth*texHeight*4];
			FillTextureFromCode (texWidth, texHeight, texHeight*4, data);
			glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
			delete[] data;
		}
	}
#endif
}

void CKinectWapper::SetTextureFromUnity(UINT trackedId, void* texturePtr)
{
	g_TexturePointer = texturePtr;
	g_textureTrackId = trackedId;
	MessageBox(NULL, L"SetTexture", NULL, MB_OK);
}

UINT CKinectWapper::g_textureTrackId = 0;

void* CKinectWapper::g_TexturePointer = NULL;

int CKinectWapper::g_DeviceType = kGfxRendererOpenGL;
