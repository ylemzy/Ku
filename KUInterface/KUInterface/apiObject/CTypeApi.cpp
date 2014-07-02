#include "stdafx.h"
#include "CTypeApi.h"
#include "KinectApi.h"

int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton)
{
	int val = CKinectWapper::NuiInitContext(useColor, useDepth, useSkeleton);
	printf("c_type call");
	return val;
}

int NuiSetBackgroundRemovedCount(UINT num)
{
	return CKinectWapper::NuSetBackgroundRemovedCount(num);
}

int NuiSetInteractionCount(UINT num)
{
	return CKinectWapper::NuiSetInteractionCount(num);
}

int NuiUpdate()
{
	printf("begin upate\n");
	int val = CKinectWapper::NuiUpdate();
	printf("update end\n");
	return val;
}

void NuiUnInitContext()
{
	return CKinectWapper::NuiUnInitContext();
}

const byte* NuiGetTextureImage(OUT int* size)
{
	return CKinectWapper::NuiGetTextureImage(size);
}

const byte* NuiGetDepthImage(OUT int* size)
{
	return CKinectWapper::NuiGetDepthImage(size);
}

const byte* NuiGetBackgroundRemovedImage(UINT player, OUT int* size)
{
	return CKinectWapper::NuiGetBackgroundRemovedImage(player, size);
}

bool NuiGetSkeletonTransform(UINT player, int joint, OUT Vector4* c)
{
	return CKinectWapper::NuiGetSkeletonTransform(player, joint, c);
}

bool NuiGetHandInfo(UINT player, bool bLeft, OUT SenLogic::KUseInfo* pHand)
{
	return CKinectWapper::NuiGetHandInfo(player, bLeft, pHand);
}

void NuiGetColorImageSize(int * width, int * height)
{
	return CKinectWapper::NuiGetColorImageSize(width, height);
}

void NuiGetDepthImageSize(int* width, int* height)
{
	return CKinectWapper::NuiGetDepthImageSize(width, height);
}

UINT NuiGetFullSkeletonCount()
{
	return CKinectWapper::NuiGetFullSkeletonCount();
}

bool NuiExistPlayer()
{
	return CKinectWapper::NuiExistPlayer();
}

void NuiGetCameraAngle(OUT float* angle)
{
	return CKinectWapper::NuiGetCameraAngle(angle);
}

bool NuiSetCameraAngle(int angle)
{
	return CKinectWapper::NuiSetCameraAngle(angle);
}

void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton)
{
	return CKinectWapper::NuiRunTest(useColor, useDepth, useSkeleton);
}

bool MapSkeletonToColor(Vector4 vec, int* x, int *y)
{
	return CKinectWapper::MapSkeletonToColor(vec, x, y);
}

UINT NuiGetBackgroundRemovedPlayerCount()
{
	return CKinectWapper::NuiGetBackgroundRemovedCount();
}

const byte* NuiGetBackgroundRemovedComposed(OUT int* size)
{
	return CKinectWapper::NuiGetBackgroundRemovedComposed(size);
}

void NuiSetBackgroundRemovedComposed(bool bComposed)
{
	return CKinectWapper::NuiSetBackgroundRemovedComposed(bComposed);
}

bool NuiIsBackgroundRemovedComposed()
{
	return CKinectWapper::NuiIsBackgroundRemovedComposed();
}

UINT NuiGetInteractionCount()
{
	return CKinectWapper::NuiGetInteractionCount();
}

void NuiGetColorResolution(int* width, int* height)
{
	return CKinectWapper::GetColorResolution(width, height);
}

void NuiGetDepthResolution(int* width, int* height)
{
	return CKinectWapper::GetDepthResolution(width, height);
}

UINT NuiGetSkeletonId(UINT i)
{
	return CKinectWapper::NuiGetSkeletonId(i);
}

int NuiBgColor(int i)
{
	return CKinectWapper::BgColor(i);
}

int NuiBgDepth(int i)
{
	return CKinectWapper::BgDepth(i);
}

int NuiBgSk(int i)
{
	return CKinectWapper::BgSk(i);
}

int NuiBgBg(int i)
{
	return CKinectWapper::BgBg(i);
}

HANDLE NuiStartProcessThread()
{
	return CKinectWapper::StartProcessData();
}

void StartProcess()
{
	CKinectWapper::StartProcessData();
}

void StopProcess()
{
	CKinectWapper::StopProcessData();
}

bool NuiGetFloorClipPlane(OUT Vector4* vFloorClipPlane)
{
	return CKinectWapper::NuiGetFloorClipPlane(vFloorClipPlane);
}
