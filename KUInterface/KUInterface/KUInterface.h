/************************************************************************
*					 Defines Methods for DLL Export						*
*																		*
*						  Author: Andrew DeVine							*
*								  2013									*
************************************************************************/

#pragma once

#include "stdafx.h"
#include <string>

#ifndef UNITYINTERFACE_API
#define UNITYINTERFACE_API extern "C" __declspec(dllexport)
#endif

//DEFINE INTERFACE METHODS

//Nui Context Management
UNITYINTERFACE_API int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);
UNITYINTERFACE_API int NuiEnableBackgroundRemoved(bool bOpen);
UNITYINTERFACE_API int NuiEnableInteraction(bool bOpen);
UNITYINTERFACE_API int NuiUpdate();
UNITYINTERFACE_API void NuiUnInitContext();
//Get Methods
UNITYINTERFACE_API byte* NuiGetTextureImage(OUT int* size);
UNITYINTERFACE_API byte* NuiGetDepthImage(OUT int* size);
UNITYINTERFACE_API const byte* NuiGetBackgroundRemovedImage(OUT int* size);
UNITYINTERFACE_API void NuiGetSkeletonTransform(int player, int joint, OUT KUVector4* SkeletonTransform);
UNITYINTERFACE_API bool NuiGetUseInfo(int player, OUT _KUUseInfo* pLeftHand, OUT _KUUseInfo* pRightHand);
UNITYINTERFACE_API void NuiGetColorImageSize(int * width, int * height);
UNITYINTERFACE_API void NuiGetDepthImageSize(int* width, int* height);

UNITYINTERFACE_API int NuiTrackedIndex();
UNITYINTERFACE_API bool NuiExistPlayer();


UNITYINTERFACE_API void NuiGetCameraAngle(OUT float* angle);
UNITYINTERFACE_API bool NuiSetCameraAngle(int angle);

UNITYINTERFACE_API void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton);