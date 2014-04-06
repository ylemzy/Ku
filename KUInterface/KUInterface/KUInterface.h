#pragma once

#include "stdafx.h"
#include <string>

#ifndef UNITYINTERFACE_API
#define UNITYINTERFACE_API extern "C" __declspec(dllexport)
#endif

//DEFINE INTERFACE METHODS

//Nui Context Management
// 初始化，并开启颜色，深度，骨骼
UNITYINTERFACE_API int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);

// 开启或关闭背景去除。必须先NuiInitContext(true, true, true)
UNITYINTERFACE_API int NuiEnableBackgroundRemoved(bool bOpen);

//开启用户信息，目前只有手势的grip(抓)， grip_release,另外通过手的坐标判断其他逻辑
UNITYINTERFACE_API int NuiEnableInteraction(bool bOpen);

// 每次准备最新的数据
UNITYINTERFACE_API int NuiUpdate();

// 释放设备流
UNITYINTERFACE_API void NuiUnInitContext();

//Get Methods
// 获取图片，返回byte数组及其大小
UNITYINTERFACE_API byte* NuiGetTextureImage(OUT int* size);

// 获取景深数据，返回byte数组，每4个byte为一个像素的数据[playerIndex，depthValue]
// 前两个byte指定playerIndex；
// 后两个byte表示深度值depthValue，以毫米为单位
UNITYINTERFACE_API byte* NuiGetDepthImage(OUT int* size);

// 去除背景图像大小，每个像素4个byte分别对应rgba，a为0，表示背景, size表示byte数组的长度
UNITYINTERFACE_API const byte* NuiGetBackgroundRemovedImage(OUT int* size);

// 玩家骨骼中某个部位的坐标
UNITYINTERFACE_API void NuiGetSkeletonTransform(int player, int joint, OUT KUVector4* c);
UNITYINTERFACE_API bool NuiGetUseInfo(int player, OUT KUUseInfo* pLeftHand, OUT KUUseInfo* pRightHand);

// 图片大小
UNITYINTERFACE_API void NuiGetColorImageSize(int * width, int * height);
// 景深数据的大小
UNITYINTERFACE_API void NuiGetDepthImageSize(int* width, int* height);

// 取得骨架对应的id
UNITYINTERFACE_API int NuiTrackedIndex();
// 是否检测到玩家
UNITYINTERFACE_API bool NuiExistPlayer();
//
UNITYINTERFACE_API int NuiGetMainPlayerId();

UNITYINTERFACE_API void NuiGetCameraAngle(OUT float* angle);
UNITYINTERFACE_API bool NuiSetCameraAngle(int angle);


UNITYINTERFACE_API void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton);
