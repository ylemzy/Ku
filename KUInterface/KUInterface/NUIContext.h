/************************************************************************
*						   Defines NUI Context							*
*																		*
*						  Author: Andrew DeVine							*
*								  2013									*
************************************************************************/

#pragma once

#include "stdafx.h"
#include "NuiApi.h"  //Microsoft Kinect NUI Aggregate Header
#include <assert.h>
#include <math.h>

class NuiContext
{
public:
    HRESULT					Nui_Init();
    void                    Nui_UnInit();
    void                    Nui_Zero();
	void					TransformCoordinates(OUT KUVector4* skTrans);
	HRESULT					PollNextSkeletonFrame();
	HRESULT					PollNextImageFrame();
	HRESULT					PollNextDepthFrame();
	HRESULT					SetCameraAngle(long angle);

	NUI_SKELETON_DATA		skData;
	NUI_SKELETON_DATA		skData2;
	LONG					angle;
	HRESULT					skelValid;
	HRESULT					imageValid;
	HRESULT					depthValid;
	bool					twoPlayer;

	byte*					texture;
	int						tsize;
	byte*					depthframe;
	int						dsize;

private:
	HANDLE					m_hVideoStream;
	HANDLE					m_hDepthStream;
	HANDLE					m_hNextColorFrameEvent;
	HANDLE					m_hNextDepthFrameEvent;
	INuiSensor*				m_pSensor;
};