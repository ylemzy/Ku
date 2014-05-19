// This is the main DLL file.

#include "stdafx.h"

#include "KinectWapper.h"
#include <stdlib.h>
KinectWapper::KWapper::KWapper()
	: m_pImp(0)
{
	m_pImp = new CKinectWapper();
}

KinectWapper::KWapper::~KWapper()
{
	delete m_pImp;
	m_pImp = 0;
}

int KinectWapper::KWapper::NuiInitContext(bool useColor, bool useDepth, bool useSkeleton)
{
	return m_pImp->NuiInitContext(useColor, useDepth, useSkeleton);
}

void KinectWapper::KWapper::NuiUnInitContext()
{
	m_pImp->NuiUnInitContext();
}
//
//int KinectWapper::KWapper::NuiEnableBackgroundRemoved(bool bOpen)
//{
//	return m_pImp->NuiEnableBackgroundRemoved(bOpen);
//}
//
//int KinectWapper::KWapper::NuiEnableInteraction(bool bOpen)
//{
//	return m_pImp->NuiEnableInteraction(bOpen);
//}
//
int KinectWapper::KWapper::NuiUpdate()
{
	return m_pImp->NuiUpdate();
}
//
//void KinectWapper::KWapper::NuiGetTextureImage(array<Byte>^ data, int% size)
//{
//	int n = 0;
//	const byte* tmpData = m_pImp->NuiGetTextureImage(&n);
//}

//const Byte* KinectWapper::KWapper::NuiGetDepthImage(int% size)
//{
//	pin_ptr<int> ptrSize = &size;
//	return m_pImp->NuiGetDepthImage(ptrSize);
//}
//
//const Byte* KinectWapper::KWapper::NuiGetBackgroundRemovedImage(int% size)
//{
//	pin_ptr<int> ptrSize = &size;
//	return m_pImp->NuiGetBackgroundRemovedImage(ptrSize);
//}
//
//void KinectWapper::KWapper::NuiGetSkeletonTransform(int player, int joint, KVector% c)
//{
//	//return m_pImp->NuiGetSkeletonTransform(player, joint, c);
//}
//
////bool KinectWapper::KinectWapper::NuiGetUseInfo(int player, KUUseInfo* pLeftHand, KUUseInfo* pRightHand)
////{
////	return m_pImp->NuiGetUseInfo(player, pLeftHand, pRightHand);
////}
//
//void KinectWapper::KWapper::NuiGetColorImageSize(int% width, int% height)
//{
//	pin_ptr<int> ptrWidth = &width;
//	pin_ptr<int> ptrHeight = &height;
//	m_pImp->NuiGetColorImageSize(ptrWidth, ptrHeight);
//}
//
//void KinectWapper::KWapper::NuiGetDepthImageSize(int%width, int% height)
//{
//	pin_ptr<int> ptrWidth = &width;
//	pin_ptr<int> ptrHeight = &height;
//	m_pImp->NuiGetDepthImageSize(ptrWidth, ptrHeight);
//}
//
//int KinectWapper::KWapper::NuiTrackedIndex()
//{
//	return m_pImp->NuiTrackedIndex();
//}
//
//bool KinectWapper::KWapper::NuiExistPlayer()
//{
//	return m_pImp->NuiExistPlayer();
//}
//
//int KinectWapper::KWapper::NuiGetMainPlayerId()
//{
//	return m_pImp->NuiGetMainPlayerId();
//}

//void KinectWapper::KWapper::NuiGetCameraAngle(float* angle)
//{
//	return m_pImp->NuiGetCameraAngle(angle);
//}
//
//bool KinectWapper::KWapper::NuiSetCameraAngle(int angle)
//{
//	return m_pImp->NuiSetCameraAngle(angle);
//}

//void KinectWapper::KWapper::NuiRunTest(bool useColor, bool useDepth, bool useSkeleton)
//{
//	return m_pImp->NuiRunTest(useColor, useDepth, useSkeleton);
//}
//
//void KinectWapper::KWapper::GetVector(interior_ptr<KVector^> vec)
//{
//	(*vec)->x = 999.0f;
//}
