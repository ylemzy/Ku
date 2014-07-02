#ifndef __CTYPE_API_H_
#define __CTYPE_API_H_

#ifndef C_TYPE_API
#define C_TYPE_API extern "C" __declspec(dllexport)
#endif

/*====================Sensor Context========================*/
// 初始化，并开启颜色，深度，骨骼
C_TYPE_API int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);

// 每次准备最新的数据
C_TYPE_API int NuiUpdate();

// 释放设备流
C_TYPE_API void NuiUnInitContext();

C_TYPE_API void StartProcess();

C_TYPE_API void StopProcess();

/*====================color========================*/
C_TYPE_API void NuiGetColorResolution(int* width, int* height);

// 获取图片，返回byte数组及其大小
C_TYPE_API const byte* NuiGetTextureImage(OUT int* size);

// 图片大小
C_TYPE_API void NuiGetColorImageSize(int * width, int * height);

/*=====================depth==========================*/
C_TYPE_API void NuiGetDepthResolution(int* width, int* height);
// 获取景深数据，返回byte数组，每4个byte为一个像素的数据[playerIndex，depthValue]
// 前两个byte指定playerIndex；
// 后两个byte表示深度值depthValue，以毫米为单位
C_TYPE_API const byte* NuiGetDepthImage(OUT int* size);

// 景深数据的大小
C_TYPE_API void NuiGetDepthImageSize(int* width, int* height);

/*===========-background removed======================*/

// 开启或关闭背景去除。必须先NuiInitContext(true, true, true)
C_TYPE_API int NuiSetBackgroundRemovedCount(UINT num);
// 去除背景图像大小，每个像素4个byte分别对应rgba，a为0，表示背景, size表示byte数组的长度
C_TYPE_API const byte* NuiGetBackgroundRemovedImage(UINT player, OUT int* size);

C_TYPE_API UINT NuiGetBackgroundRemovedPlayerCount();
C_TYPE_API const byte* NuiGetBackgroundRemovedComposed(OUT int* size);
C_TYPE_API void NuiSetBackgroundRemovedComposed(bool bComposed);
C_TYPE_API bool NuiIsBackgroundRemovedComposed();

/*===============interaction-===========================*/
//开启用户信息，目前只有手势的grip(抓)， grip_release,另外通过手的坐标判断其他逻辑
C_TYPE_API int NuiSetInteractionCount(UINT num);
C_TYPE_API bool NuiGetHandInfo(UINT player, bool bLeft, OUT SenLogic::KUseInfo* pHand);
C_TYPE_API UINT NuiGetInteractionCount();
/*==============skeleton========================*/
//获取骨架id
C_TYPE_API UINT NuiGetSkeletonId(UINT i);
// 玩家骨骼中某个部位的坐标
C_TYPE_API bool NuiGetSkeletonTransform(UINT player, int joint, OUT Vector4* c);
// 取得骨架对应的id
C_TYPE_API UINT NuiGetFullSkeletonCount();
// 是否检测到玩家
C_TYPE_API bool NuiExistPlayer();

C_TYPE_API bool MapSkeletonToColor(Vector4 vec, int* x, int *y);
C_TYPE_API bool NuiGetFloorClipPlane(OUT Vector4* vFloorClipPlane);
/*===================camera=============================*/
C_TYPE_API void NuiGetCameraAngle(OUT float* angle);
C_TYPE_API bool NuiSetCameraAngle(int angle);

/*====================just test========================*/
C_TYPE_API void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton);
C_TYPE_API HANDLE NuiStartProcessThread();

C_TYPE_API int NuiBgColor(int i);
C_TYPE_API int NuiBgDepth(int i);
C_TYPE_API int NuiBgSk(int i);
C_TYPE_API int NuiBgBg(int i);
#endif