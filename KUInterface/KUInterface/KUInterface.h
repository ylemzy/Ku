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
UNITYINTERFACE_API bool NuiContextInit(bool twoPlayer);
UNITYINTERFACE_API void NuiUpdate();
UNITYINTERFACE_API void NuiContextUnInit();
//Get Methods
UNITYINTERFACE_API void GetSkeletonTransform(int player, int joint, OUT KUVector4* SkeletonTransform);
UNITYINTERFACE_API byte* GetTextureImage(OUT int* size);
UNITYINTERFACE_API byte* GetDepthImage(OUT int* size);
UNITYINTERFACE_API void GetCameraAngle(OUT float* angle);
//Set Methods
UNITYINTERFACE_API bool SetCameraAngle(int angle);