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

NuiContext g_NuiContext;

/************************************************************************
						NUI CONTEXT MANAGEMENT
************************************************************************/

//Initializes NUI Context
bool NuiContextInit(bool twoPlayer) {

	HRESULT hr;

	hr = g_NuiContext.Nui_Init();

	if(FAILED(hr))
		return false;
	else
		g_NuiContext.twoPlayer = twoPlayer;
		return true;
}


//Updates Skeleton, Image Data
void NuiUpdate() {

	g_NuiContext.skelValid = g_NuiContext.PollNextSkeletonFrame();
	g_NuiContext.imageValid = g_NuiContext.PollNextImageFrame();
	g_NuiContext.depthValid = g_NuiContext.PollNextDepthFrame();
}


//Closes NUI Context
void NuiContextUnInit() {

	g_NuiContext.Nui_UnInit();
}


/************************************************************************
							GET METHODS
************************************************************************/

//Creates Custom KUVector4 object for export to Unity
void GetSkeletonTransform(int player, int joint, OUT KUVector4* SkeletonTransform) {

	KUVector4 &skTrans = *SkeletonTransform;

	if (player == 1) {
		if(SUCCEEDED(g_NuiContext.skelValid)) {
			skTrans.x = g_NuiContext.skData.SkeletonPositions[joint].x;
			skTrans.y = g_NuiContext.skData.SkeletonPositions[joint].y;
			skTrans.z = g_NuiContext.skData.SkeletonPositions[joint].z;
			skTrans.w = g_NuiContext.skData.SkeletonPositions[joint].w;
		} else {
			skTrans.x = 0.0f;
			skTrans.y = 0.0f;
			skTrans.z = 0.0f;
			skTrans.w = 0.0f;
		}
	} else if (player == 2) {
		if(SUCCEEDED(g_NuiContext.skelValid)) {
			skTrans.x = g_NuiContext.skData2.SkeletonPositions[joint].x;
			skTrans.y = g_NuiContext.skData2.SkeletonPositions[joint].y;
			skTrans.z = g_NuiContext.skData2.SkeletonPositions[joint].z;
			skTrans.w = g_NuiContext.skData2.SkeletonPositions[joint].w;
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

	g_NuiContext.TransformCoordinates(&skTrans);
}


//Gets color texture image
byte* GetTextureImage(OUT int* size) {

	*size = g_NuiContext.tsize;
	return g_NuiContext.texture;
}


//Gets depth image
byte* GetDepthImage(OUT int* size) {

	*size = g_NuiContext.dsize;
	return g_NuiContext.depthframe;
}


//Gets current camera angle
void GetCameraAngle(OUT float* angle) {

	*angle = g_NuiContext.angle;
}


/************************************************************************
							SET METHODS
************************************************************************/

bool SetCameraAngle(int angle) {

	HRESULT hr;
	
	if (angle >= -27 && angle <= 27) {
		hr = g_NuiContext.SetCameraAngle((long)angle);
		if (FAILED(hr)) {
			return false;
		} else {
			g_NuiContext.angle = (long)angle;
			return true;
		}
	} else {
		return false;
	}
}