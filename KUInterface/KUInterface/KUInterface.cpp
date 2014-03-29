/************************************************************************
*				   This file contains methods available					*
*				to System.Runtime.InteropServies.DllImport				*
*																		*
*						  Author: Andrew DeVine							*
*								  2013									*
************************************************************************/

#include "stdafx.h"
#include "KUInterface.h"  //DLL export declarations
#include "NuiContext.h"      //NUI object


/************************************************************************
						NUI CONTEXT MANAGEMENT
************************************************************************/


class ContextOwner
{
public:
	static NuiContext* Instance()
	{
		if (m_pNuiContext == NULL)
		{
			m_pNuiContext = new NuiContext();
		}
		return m_pNuiContext;
	}

	static void ReleaseInstance()
	{
		delete m_pNuiContext;
		m_pNuiContext = NULL;
	}
private:
	static NuiContext* m_pNuiContext;
};

NuiContext* ContextOwner::m_pNuiContext = NULL;

//Initializes NUI Context
int NuiContextInit(bool twoPlayer) 
{
	return ContextOwner::Instance()->Nui_Init();
}


//Updates Skeleton, Image Data
void NuiUpdate() 
{
	ContextOwner::Instance()->skelValid = ContextOwner::Instance()->ProcessSkeleton();
	ContextOwner::Instance()->imageValid = ContextOwner::Instance()->ProcessColor();
	ContextOwner::Instance()->depthValid = ContextOwner::Instance()->ProcessDepth();
	ContextOwner::Instance()->backgroundRemovedValid = ContextOwner::Instance()->ProcessBackgroundRemoved();
}


//Closes NUI Context
void NuiContextUnInit() 
{
	ContextOwner::ReleaseInstance();
}


/************************************************************************
							GET METHODS
************************************************************************/

//Creates Custom KUVector4 object for export to Unity
void GetSkeletonTransform(int player, int joint, OUT KUVector4* SkeletonTransform) {

	KUVector4 &skTrans = *SkeletonTransform;

	if (player == 1) {
		if(SUCCEEDED(ContextOwner::Instance()->skelValid)) {
			skTrans.x = ContextOwner::Instance()->skData.SkeletonPositions[joint].x;
			skTrans.y = ContextOwner::Instance()->skData.SkeletonPositions[joint].y;
			skTrans.z = ContextOwner::Instance()->skData.SkeletonPositions[joint].z;
			skTrans.w = ContextOwner::Instance()->skData.SkeletonPositions[joint].w;
		} else {
			skTrans.x = 0.0f;
			skTrans.y = 0.0f;
			skTrans.z = 0.0f;
			skTrans.w = 0.0f;
		}
	} else if (player == 2) {
		if(SUCCEEDED(ContextOwner::Instance()->skelValid)) {
			skTrans.x = ContextOwner::Instance()->skData2.SkeletonPositions[joint].x;
			skTrans.y = ContextOwner::Instance()->skData2.SkeletonPositions[joint].y;
			skTrans.z = ContextOwner::Instance()->skData2.SkeletonPositions[joint].z;
			skTrans.w = ContextOwner::Instance()->skData2.SkeletonPositions[joint].w;
		} else {
			skTrans.x = 0.0f;
			skTrans.y = 0.0f;
			skTrans.z = 0.0f;
			skTrans.w = 0.0f;
		}
	} else {
		skTrans.x = 0.0f;
		skTrans.y = 0.0f;
		skTrans.z = 0.0f;
		skTrans.w = 0.0f;
	}

	ContextOwner::Instance()->TransformCoordinates(&skTrans);
}


//Gets color texture image
byte* GetTextureImage(OUT int* size) {

	*size = ContextOwner::Instance()->tsize;
	return ContextOwner::Instance()->texture;
}


//Gets depth image
byte* GetDepthImage(OUT int* size) 
{
	*size = ContextOwner::Instance()->dsize;
	return ContextOwner::Instance()->depthData;
}


//Gets current camera angle
void GetCameraAngle(OUT float* angle) {

	*angle = ContextOwner::Instance()->angle;
}


const byte* GetBackgroundRemovedImage(OUT int* size)
{
	*size = ContextOwner::Instance()->backgroundRemovedSize;
	return ContextOwner::Instance()->backgroundRemovedTexture;
}



/************************************************************************
							SET METHODS
************************************************************************/

bool SetCameraAngle(int angle) {

	HRESULT hr;
	
	if (angle >= -27 && angle <= 27) {
		hr = ContextOwner::Instance()->SetCameraAngle((long)angle);
		if (FAILED(hr)) {
			return false;
		} else {
			ContextOwner::Instance()->angle = (long)angle;
			return true;
		}
	} else {
		return false;
	}
}

bool SetBackgroundRemoved(bool bEnabled)
{
	if (bEnabled)
		ContextOwner::Instance()->CreateBackgroundRemovedColorStream();
	return true;
}

void GetColorImageSize(int * width, int * height)
{
	if (width == NULL || height == NULL)
		return;

	*width = ContextOwner::Instance()->m_colorWidth;
	*height = ContextOwner::Instance()->m_colorHeight;
}

void GetDepthImageSize(int* width, int* height)
{
	if (width == NULL || height == NULL)
		return;

	*width = ContextOwner::Instance()->m_depthWidth;
	*height = ContextOwner::Instance()->m_depthHeight;
}

int GetTrackedIndex()
{
	return ContextOwner::Instance()->m_skTackedId;
}

