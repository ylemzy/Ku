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
	// ��ʼ������������ɫ����ȣ�����
	static int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);

	// ÿ��׼�����µ�����
	static int NuiUpdate();

	// �ͷ��豸��
	static void NuiUnInitContext();
	/*====================color========================*/
public:
	static void GetColorResolution(int* width, int* height);
	//Get Methods
	// ��ȡͼƬ������byte���鼰���С
	static const byte* NuiGetTextureImage(OUT int* size);
	// ͼƬ��С
	static void NuiGetColorImageSize(int * width, int * height);

	/*=====================depth==========================*/
public:
	static void GetDepthResolution(int* width, int* height);
	// ��ȡ�������ݣ�����byte���飬ÿ4��byteΪһ�����ص�����[playerIndex��depthValue]
	// ǰ����byteָ��playerIndex��
	// ������byte��ʾ���ֵdepthValue���Ժ���Ϊ��λ
	static const byte* NuiGetDepthImage(OUT int* size);
	// �������ݵĴ�С
	static void NuiGetDepthImageSize(int* width, int* height);

	/*===========-background removed======================*/
public:
	// ������رձ���ȥ����������NuiInitContext(true, true, true), Ŀǰ֧������
	static int NuSetBackgroundRemovedCount(UINT num);
	// ȥ������ͼ���С��ÿ������4��byte�ֱ��Ӧrgba��aΪ0����ʾ����, size��ʾbyte����ĳ���
	static const byte* NuiGetBackgroundRemovedImage(UINT player, OUT int* size);
	static UINT NuiGetBackgroundRemovedCount();
	static const byte* NuiGetBackgroundRemovedComposed(OUT int* size);
	static void NuiSetBackgroundRemovedComposed(bool bComposed);
	static bool NuiIsBackgroundRemovedComposed();

	/*===============interaction-===========================*/
public:
	//�����û���Ϣ��Ŀǰֻ�����Ƶ�grip(ץ)�� grip_release,����ͨ���ֵ������ж������߼�
	static int NuiSetInteractionCount(UINT num);
	static bool NuiGetHandInfo(UINT player, bool bLeft, OUT SenLogic::KUseInfo* pHand);
	static UINT NuiGetInteractionCount();
	/*==============skeleton========================*/
public:
	static UINT NuiGetSkeletonId(UINT i);
	// ��ҹ�����ĳ����λ������
	static bool NuiGetSkeletonTransform(UINT player, int joint, OUT Vector4* c);
	static bool MapSkeletonToColor(Vector4 vec, int* x, int *y);
	// ȡ�ùǼܶ�Ӧ��id
	static UINT NuiGetFullSkeletonCount();
	// �Ƿ��⵽���
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