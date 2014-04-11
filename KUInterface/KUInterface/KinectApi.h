#pragma once

#include "stdafx.h"

//DEFINE INTERFACE METHODS
class SensorContext;
class KN_DLL_CLASS CKinectWapper
{
public:
	CKinectWapper();
	~CKinectWapper();

public:
	//Nui Context Management
	// ��ʼ������������ɫ����ȣ�����
	static int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);

	// ������رձ���ȥ����������NuiInitContext(true, true, true)
	static int NuiEnableBackgroundRemoved(bool bOpen);

	//�����û���Ϣ��Ŀǰֻ�����Ƶ�grip(ץ)�� grip_release,����ͨ���ֵ������ж������߼�
	static int NuiEnableInteraction(bool bOpen);

	// ÿ��׼�����µ�����
	static int NuiUpdate();

	// �ͷ��豸��
	static void NuiUnInitContext();

	//Get Methods
	// ��ȡͼƬ������byte���鼰���С
	static byte* NuiGetTextureImage(OUT int* size);

	// ��ȡ�������ݣ�����byte���飬ÿ4��byteΪһ�����ص�����[playerIndex��depthValue]
	// ǰ����byteָ��playerIndex��
	// ������byte��ʾ���ֵdepthValue���Ժ���Ϊ��λ
	static byte* NuiGetDepthImage(OUT int* size);

	// ȥ������ͼ���С��ÿ������4��byte�ֱ��Ӧrgba��aΪ0����ʾ����, size��ʾbyte����ĳ���
	static const byte* NuiGetBackgroundRemovedImage(OUT int* size);

	// ��ҹ�����ĳ����λ������
	static void NuiGetSkeletonTransform(int player, int joint, OUT KVector4* c);
	static bool NuiGetUseInfo(int player, OUT KUseInfo* pLeftHand, OUT KUseInfo* pRightHand);

	// ͼƬ��С
	static void NuiGetColorImageSize(int * width, int * height);
	// �������ݵĴ�С
	static void NuiGetDepthImageSize(int* width, int* height);

	// ȡ�ùǼܶ�Ӧ��id
	static int NuiTrackedIndex();
	// �Ƿ��⵽���
	static bool NuiExistPlayer();
	//
	static int NuiGetMainPlayerId();

	static void NuiGetCameraAngle(OUT float* angle);
	static bool NuiSetCameraAngle(int angle);


	static void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton);
private:
	static SensorContext* m_pNuiContext;
	static SensorContext* Instance();
};

