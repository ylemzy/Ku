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
	//	// ��ʼ������������ɫ����ȣ�����
		int NuiInitContext(bool useColor, bool useDepth, bool useSkeleton);
	//	// �ͷ��豸��
		void NuiUnInitContext();

	//	// ������رձ���ȥ����������NuiInitContext(true, true, true)
	//	int NuiEnableBackgroundRemoved(bool bOpen);

	//	//�����û���Ϣ��Ŀǰֻ�����Ƶ�grip(ץ)�� grip_release,����ͨ���ֵ������ж������߼�
	//	int NuiEnableInteraction(bool bOpen);

	//	// ÿ��׼�����µ�����
		int NuiUpdate();

	//	//Get Methods
	//	// ��ȡͼƬ������byte���鼰���С
	/*	void NuiGetTextureImage(array<Byte>^ data, int% size);*/

	//	// ��ȡ�������ݣ�����byte���飬ÿ4��byteΪһ�����ص�����[playerIndex��depthValue]
	//	// ǰ����byteָ��playerIndex��
	//	// ������byte��ʾ���ֵdepthValue���Ժ���Ϊ��λ
	//	const Byte* NuiGetDepthImage(int% size);

	//	// ȥ������ͼ���С��ÿ������4��byte�ֱ��Ӧrgba��aΪ0����ʾ����, size��ʾbyte����ĳ���
	//	const Byte* NuiGetBackgroundRemovedImage(int% size);

	//	// ��ҹ�����ĳ����λ������
	//	void NuiGetSkeletonTransform(int player, int joint, KVector% c);
	//	//bool NuiGetUseInfo(int player, KUUseInfo* pLeftHand, KUUseInfo* pRightHand);

	//	//// ͼƬ��С
	//	void NuiGetColorImageSize(int% width, int% height);
	//	//// �������ݵĴ�С
	//	void NuiGetDepthImageSize(int% width, int% height);

	//	// ȡ�ùǼܶ�Ӧ��id
	//	int NuiTrackedIndex();
	//	// �Ƿ��⵽���
	//	bool NuiExistPlayer();

	//	//Ŀǰֻ��һ�����
	//	int NuiGetMainPlayerId();

		/*void NuiGetCameraAngle(float* angle);
		bool NuiSetCameraAngle(int angle);
*/
		/*void NuiRunTest(bool useColor, bool useDepth, bool useSkeleton);*/

	private:
		CKinectWapper* m_pImp;
	};
}
