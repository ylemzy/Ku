#ifndef __KINECT_API_H_
#define __KINECT_API_H_

#include "..\stdafx.h"
class SensorContext;
class KN_DLL_CLASS CKinectWapper
{
public:
	CKinectWapper();
	~CKinectWapper();

public:
	/*====================Sensor Context========================*/
	// ��ʼ������������ɫ����ȣ�����
	static int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);

	// ÿ��׼�����µ�����
	static int NuiUpdate();

	// �ͷ��豸��
	static void NuiUnInitContext();
	/*====================color========================*/
	//Get Methods
	// ��ȡͼƬ������byte���鼰���С
	static const byte* NuiGetTextureImage(OUT int* size);
	// ͼƬ��С
	static void NuiGetColorImageSize(int * width, int * height);

	/*=====================depth==========================*/
	// ��ȡ�������ݣ�����byte���飬ÿ4��byteΪһ�����ص�����[playerIndex��depthValue]
	// ǰ����byteָ��playerIndex��
	// ������byte��ʾ���ֵdepthValue���Ժ���Ϊ��λ
	static const byte* NuiGetDepthImage(OUT int* size);
	// �������ݵĴ�С
	static void NuiGetDepthImageSize(int* width, int* height);

	/*===========-background removed======================*/
	// ������رձ���ȥ����������NuiInitContext(true, true, true), Ŀǰ֧������
	static int NuSetBackgroundRemovedCount(UINT num);
	// ȥ������ͼ���С��ÿ������4��byte�ֱ��Ӧrgba��aΪ0����ʾ����, size��ʾbyte����ĳ���
	static const byte* NuiGetBackgroundRemovedImage(UINT player, OUT int* size);
	static UINT NuiGetBackgroundRemovedCount();
	static const byte* NuiGetBackgroundRemovedComposed(OUT int* size);
	static void NuiSetBackgroundRemovedComposed(bool bComposed);
	static bool NuiIsBackgroundRemovedComposed();


	/*===============interaction-===========================*/
	//�����û���Ϣ��Ŀǰֻ�����Ƶ�grip(ץ)�� grip_release,����ͨ���ֵ������ж������߼�
	static int NuiSetInteractionCount(UINT num);
	static bool NuiGetUseInfo(UINT player, OUT SenLogic::KUseInfo* pLeftHand, OUT SenLogic::KUseInfo* pRightHand);
	static UINT NuiGetInteractionCount();
	/*==============skeleton========================*/
	// ��ҹ�����ĳ����λ������
	static void NuiGetSkeletonTransform(UINT player, int joint, OUT Vector4* c);
	static bool MapSkeletonToColor(Vector4 vec, int* x, int *y);
	// ȡ�ùǼܶ�Ӧ��id
	static UINT NuiGetFullSkeletonCount();
	// �Ƿ��⵽���
	static bool NuiExistPlayer();
	/*===================camera=============================*/
	static void NuiGetCameraAngle(OUT float* angle);
	static bool NuiSetCameraAngle(int angle);

	/*====================just test========================*/

	static void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton);

	static void RunAngleTest();

	
private:
	static SensorContext* m_pNuiContext;
	static SensorContext* Instance();
};
#endif