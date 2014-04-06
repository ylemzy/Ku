#pragma once

#include "stdafx.h"
#include <string>

#ifndef UNITYINTERFACE_API
#define UNITYINTERFACE_API extern "C" __declspec(dllexport)
#endif

//DEFINE INTERFACE METHODS

//Nui Context Management
// ��ʼ������������ɫ����ȣ�����
UNITYINTERFACE_API int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);

// ������رձ���ȥ����������NuiInitContext(true, true, true)
UNITYINTERFACE_API int NuiEnableBackgroundRemoved(bool bOpen);

//�����û���Ϣ��Ŀǰֻ�����Ƶ�grip(ץ)�� grip_release,����ͨ���ֵ������ж������߼�
UNITYINTERFACE_API int NuiEnableInteraction(bool bOpen);

// ÿ��׼�����µ�����
UNITYINTERFACE_API int NuiUpdate();

// �ͷ��豸��
UNITYINTERFACE_API void NuiUnInitContext();

//Get Methods
// ��ȡͼƬ������byte���鼰���С
UNITYINTERFACE_API byte* NuiGetTextureImage(OUT int* size);

// ��ȡ�������ݣ�����byte���飬ÿ4��byteΪһ�����ص�����[playerIndex��depthValue]
// ǰ����byteָ��playerIndex��
// ������byte��ʾ���ֵdepthValue���Ժ���Ϊ��λ
UNITYINTERFACE_API byte* NuiGetDepthImage(OUT int* size);

// ȥ������ͼ���С��ÿ������4��byte�ֱ��Ӧrgba��aΪ0����ʾ����, size��ʾbyte����ĳ���
UNITYINTERFACE_API const byte* NuiGetBackgroundRemovedImage(OUT int* size);

// ��ҹ�����ĳ����λ������
UNITYINTERFACE_API void NuiGetSkeletonTransform(int player, int joint, OUT KUVector4* c);
UNITYINTERFACE_API bool NuiGetUseInfo(int player, OUT KUUseInfo* pLeftHand, OUT KUUseInfo* pRightHand);

// ͼƬ��С
UNITYINTERFACE_API void NuiGetColorImageSize(int * width, int * height);
// �������ݵĴ�С
UNITYINTERFACE_API void NuiGetDepthImageSize(int* width, int* height);

// ȡ�ùǼܶ�Ӧ��id
UNITYINTERFACE_API int NuiTrackedIndex();
// �Ƿ��⵽���
UNITYINTERFACE_API bool NuiExistPlayer();
//
UNITYINTERFACE_API int NuiGetMainPlayerId();

UNITYINTERFACE_API void NuiGetCameraAngle(OUT float* angle);
UNITYINTERFACE_API bool NuiSetCameraAngle(int angle);


UNITYINTERFACE_API void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton);
