#ifndef __CTYPE_API_H_
#define __CTYPE_API_H_

#ifndef C_TYPE_API
#define C_TYPE_API extern "C" __declspec(dllexport)
#endif

/*====================Sensor Context========================*/
// ��ʼ������������ɫ����ȣ�����
C_TYPE_API int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);

// ÿ��׼�����µ�����
C_TYPE_API int NuiUpdate();

// �ͷ��豸��
C_TYPE_API void NuiUnInitContext();

C_TYPE_API void StartProcess();

C_TYPE_API void StopProcess();

/*====================color========================*/
C_TYPE_API void NuiGetColorResolution(int* width, int* height);

// ��ȡͼƬ������byte���鼰���С
C_TYPE_API const byte* NuiGetTextureImage(OUT int* size);

// ͼƬ��С
C_TYPE_API void NuiGetColorImageSize(int * width, int * height);

/*=====================depth==========================*/
C_TYPE_API void NuiGetDepthResolution(int* width, int* height);
// ��ȡ�������ݣ�����byte���飬ÿ4��byteΪһ�����ص�����[playerIndex��depthValue]
// ǰ����byteָ��playerIndex��
// ������byte��ʾ���ֵdepthValue���Ժ���Ϊ��λ
C_TYPE_API const byte* NuiGetDepthImage(OUT int* size);

// �������ݵĴ�С
C_TYPE_API void NuiGetDepthImageSize(int* width, int* height);

/*===========-background removed======================*/

// ������رձ���ȥ����������NuiInitContext(true, true, true)
C_TYPE_API int NuiSetBackgroundRemovedCount(UINT num);
// ȥ������ͼ���С��ÿ������4��byte�ֱ��Ӧrgba��aΪ0����ʾ����, size��ʾbyte����ĳ���
C_TYPE_API const byte* NuiGetBackgroundRemovedImage(UINT player, OUT int* size);

C_TYPE_API UINT NuiGetBackgroundRemovedPlayerCount();
C_TYPE_API const byte* NuiGetBackgroundRemovedComposed(OUT int* size);
C_TYPE_API void NuiSetBackgroundRemovedComposed(bool bComposed);
C_TYPE_API bool NuiIsBackgroundRemovedComposed();

/*===============interaction-===========================*/
//�����û���Ϣ��Ŀǰֻ�����Ƶ�grip(ץ)�� grip_release,����ͨ���ֵ������ж������߼�
C_TYPE_API int NuiSetInteractionCount(UINT num);
C_TYPE_API bool NuiGetHandInfo(UINT player, bool bLeft, OUT SenLogic::KUseInfo* pHand);
C_TYPE_API UINT NuiGetInteractionCount();
/*==============skeleton========================*/
//��ȡ�Ǽ�id
C_TYPE_API UINT NuiGetSkeletonId(UINT i);
// ��ҹ�����ĳ����λ������
C_TYPE_API bool NuiGetSkeletonTransform(UINT player, int joint, OUT Vector4* c);
// ȡ�ùǼܶ�Ӧ��id
C_TYPE_API UINT NuiGetFullSkeletonCount();
// �Ƿ��⵽���
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