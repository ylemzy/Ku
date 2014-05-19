// KinectWapper.h

#pragma once
#include "KinectApi.h"

using namespace System;
namespace KinectWapper {

	/*public ref struct KVector
	{
		float x, y, z, w;
	};*/

	public ref class KWapper
	{
		// TODO: Add your methods for this class here.
	public:
		KWapper();
		~KWapper();

	public:
		/*void GetVector(interior_ptr<KVector^> vec);*/


	//public:
	//	//Nui Context Management
	//	// 初始化，并开启颜色，深度，骨骼
		int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);
	//	// 释放设备流
		void NuiUnInitContext();

	//	// 开启或关闭背景去除。必须先NuiInitContext(true, true, true)
	//	int NuiEnableBackgroundRemoved(bool bOpen);

	//	//开启用户信息，目前只有手势的grip(抓)， grip_release,另外通过手的坐标判断其他逻辑
	//	int NuiEnableInteraction(bool bOpen);

	//	// 每次准备最新的数据
		int NuiUpdate();

	//	//Get Methods
	//	// 获取图片，返回byte数组及其大小
	/*	void NuiGetTextureImage(array<Byte>^ data, int% size);*/

	//	// 获取景深数据，返回byte数组，每4个byte为一个像素的数据[playerIndex，depthValue]
	//	// 前两个byte指定playerIndex；
	//	// 后两个byte表示深度值depthValue，以毫米为单位
	//	const Byte* NuiGetDepthImage(int% size);

	//	// 去除背景图像大小，每个像素4个byte分别对应rgba，a为0，表示背景, size表示byte数组的长度
	//	const Byte* NuiGetBackgroundRemovedImage(int% size);

	//	// 玩家骨骼中某个部位的坐标
	//	void NuiGetSkeletonTransform(int player, int joint, KVector% c);
	//	//bool NuiGetUseInfo(int player, KUUseInfo* pLeftHand, KUUseInfo* pRightHand);

	//	//// 图片大小
	//	void NuiGetColorImageSize(int% width, int% height);
	//	//// 景深数据的大小
	//	void NuiGetDepthImageSize(int% width, int% height);

	//	// 取得骨架对应的id
	//	int NuiTrackedIndex();
	//	// 是否检测到玩家
	//	bool NuiExistPlayer();

	//	//目前只有一个玩家
	//	int NuiGetMainPlayerId();

		/*void NuiGetCameraAngle(float* angle);
		bool NuiSetCameraAngle(int angle);
*/
		/*void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton);*/

	private:
		CKinectWapper* m_pImp;
	};
}
