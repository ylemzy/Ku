#ifndef __KINECT_API_H_
#define __KINECT_API_H_

#include "..\stdafx.h"
#include "..\RendingPlugin\RendingPlugin.h"
#include <fstream>
class SensorContext;
class KN_DLL_CLASS CKinectWapper
{

public:
	CKinectWapper();
	~CKinectWapper();


	/*====================Sensor Context========================*/
public:
	// 初始化，并开启颜色，深度，骨骼
	static int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);

	// 每次准备最新的数据
	static int NuiUpdate();

	// 释放设备流
	static void NuiUnInitContext();
	/*====================color========================*/
public:
	static void GetColorResolution(int* width, int* height);
	//Get Methods
	// 获取图片，返回byte数组及其大小
	static const byte* NuiGetTextureImage(OUT int* size);
	// 图片大小
	static void NuiGetColorImageSize(int * width, int * height);

	/*=====================depth==========================*/
public:
	static void GetDepthResolution(int* width, int* height);
	// 获取景深数据，返回byte数组，每4个byte为一个像素的数据[playerIndex，depthValue]
	// 前两个byte指定playerIndex；
	// 后两个byte表示深度值depthValue，以毫米为单位
	static const byte* NuiGetDepthImage(OUT int* size);
	// 景深数据的大小
	static void NuiGetDepthImageSize(int* width, int* height);

	/*===========-background removed======================*/
public:
	// 开启或关闭背景去除。必须先NuiInitContext(true, true, true), 目前支持两人
	static int NuSetBackgroundRemovedCount(UINT num);
	// 去除背景图像大小，每个像素4个byte分别对应rgba，a为0，表示背景, size表示byte数组的长度
	static const byte* NuiGetBackgroundRemovedImage(UINT player, OUT int* size);
	static UINT NuiGetBackgroundRemovedCount();
	static const byte* NuiGetBackgroundRemovedComposed(OUT int* size);
	static void NuiSetBackgroundRemovedComposed(bool bComposed);
	static bool NuiIsBackgroundRemovedComposed();

	/*===============interaction-===========================*/
public:
	//开启用户信息，目前只有手势的grip(抓)， grip_release,另外通过手的坐标判断其他逻辑
	static int NuiSetInteractionCount(UINT num);
	static bool NuiGetHandInfo(UINT player, bool bLeft, OUT SenLogic::KUseInfo* pHand);
	static UINT NuiGetInteractionCount();
	/*==============skeleton========================*/
public:
	static UINT NuiGetSkeletonId(UINT i);
	// 玩家骨骼中某个部位的坐标
	static bool NuiGetSkeletonTransform(UINT player, int joint, OUT Vector4* c);
	static bool MapSkeletonToColor(Vector4 vec, int* x, int *y);
	// 取得骨架对应的id
	static UINT NuiGetFullSkeletonCount();
	// 是否检测到玩家
	static bool NuiExistPlayer();
	static bool NuiGetFloorClipPlane(OUT Vector4* vFloorClipPlane);
	/*===================camera=============================*/
public:
	static void NuiGetCameraAngle(OUT float* angle);
	static bool NuiSetCameraAngle(int angle);

	/*====================just test========================*/
public:
	static void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton);

	static void RunAngleTest();

	static int BgColor(int i);
	static int BgDepth(int i);
	static int BgSk(int i);
	static int BgBg(int i);

public:
	static DWORD WINAPI UpdateProc(IN LPVOID lpParameter);
	static HANDLE StartProcessData();
	static void StopProcessData();

private:
	static SensorContext* m_pNuiContext;
	static SensorContext* Instance();

	static HANDLE g_hThread;
	static bool g_bStopThread;
	static ofstream* s;
	static float a1;
	static float b1;
	static float c1;
};
#endif