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


enum RUNTIME_RESULT
{
	
	SUCCEEDED_OK = 1,
	SUCCEEDED_FALSE = 0,

	CREATE_SENSOR_ERROR = -1,
	SENSOR_INITIALIZE_ERROR = -2,

	OPEN_IMAGE_COLOR_ERROR = -3,
	OPEN_DEPTH_AND_PLAYER_INDEX_ERROR = -4,
	OPEN_SKELETON_ERROR = -5,

	CREATE_BACKGROUND_REMOVED_ERROR = -6,
	ENABLE_BACKGROUND_REMOVED_ERROR = -7,

	GET_IMAGE_FRAME_ERROR = -8,
	GET_DEPTH_FRAME_ERROR = -9,
	GET_SKELETON_FRAME_ERROR = -10,
	GET_BACKGROUND_REMOVED_FRAME_ERROR = -11,

	GET_DEPTH_IMAGE_PIXEL_FRAME_TEXTURE_ERROR = -12,

	BACKGROUND_REMOVED_PROCESS_COLOR_ERROR = -13,
	BACKGROUND_REMOVED_PROCESS_DEPTH_ERROR = -14,
	BACKGROUND_REMOVED_PROCESS_SKELETON_ERROR = -15,

	NO_SKELETON_TRACKING = -16,
	SET_TRACKEDPLAYER_ERROR = -17,

	REALEASE_HANDEL_ERROR = -18
};

#ifndef CHECK_ERROR
#define CHECK_ERROR(hr)		\
	if (FAILED(hr))			\
{						\
	OUTPUT_CUSTOM_ERROR(hr);	\
	return hr;				\
	}
#endif

#define OUTPUT_CUSTOM_ERROR(hr)\
	WCHAR msg[5];	\
	_ltow_s(hr, msg, 5, 10);	\
	MessageBox( NULL, msg, L"Error", MB_OK | MB_ICONINFORMATION );\

#define OUTPUT_LAST_ERROR(hr) \
	LPVOID lpMsgBuf = NULL;	\
	FormatMessage(	\
	FORMAT_MESSAGE_ALLOCATE_BUFFER |	\
	FORMAT_MESSAGE_FROM_SYSTEM |		\
	FORMAT_MESSAGE_IGNORE_INSERTS,		\
	NULL,								\
	GetLastError(),						\
	0,									\
	(LPTSTR) &lpMsgBuf,					\
	0,									\
	NULL								\
	);										\
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION );\
	LocalFree( lpMsgBuf );

interface INuiBackgroundRemovedColorStream;
class NuiContext
{
	static const int		cBytesPerPixel = 4;
	static const NUI_IMAGE_RESOLUTION	cDepthResolution = NUI_IMAGE_RESOLUTION_320x240;
	static const NUI_IMAGE_RESOLUTION	cColorResolution = NUI_IMAGE_RESOLUTION_640x480;

public:
	NuiContext();
	~NuiContext();

public:
	HRESULT					Nui_Init(RUNTIME_RESULT* rtHr = 0);
	void                    Nui_UnInit();
	void                    Nui_Zero();
	void					TransformCoordinates(OUT KUVector4* skTrans);
	HRESULT					ProcessSkeleton(RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessColor(RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessDepth(RUNTIME_RESULT* rtHr = 0);
	HRESULT					CreateBackgroundRemovedColorStream(RUNTIME_RESULT* rtHr = 0);
	HRESULT					SetCameraAngle(long angle);
	HRESULT					ProcessBackgroundRemoved(RUNTIME_RESULT* rtHr = 0);
	
	BOOL					IsBackgroundRemovedEnabled() const;
	

public:
	NUI_SKELETON_DATA		m_skData;
	NUI_SKELETON_DATA		m_skData2;
	LONG					angle;
	HRESULT					skeletonValid;
	HRESULT					imageValid;
	HRESULT					depthValid;
	HRESULT					backgroundRemovedValid;
	bool					twoPlayer;

	byte*					m_pTexture;
	int						tsize;
	byte*					m_pDepthData;
	int						dsize;
	const byte*				m_pBackgroundRemovedTexture;
	int						backgroundRemovedSize;

	BOOL					m_bNearMode;
	LONG m_colorWidth;
	LONG m_colorHeight;
	LONG m_depthWidth;
	LONG m_depthHeight;
	int m_skTackedId;
private:
	HANDLE					m_pColorStreamHandle;
	HANDLE					m_pDepthStreamHandle;
	HANDLE					m_hNextColorFrameEvent;
	HANDLE					m_hNextDepthFrameEvent;
	HANDLE					m_hNextSkeletonFrameEvent;
	HANDLE					m_hNextBackgroundRemovedFrameEvent;
	INuiBackgroundRemovedColorStream* m_pBackgroundRemovalStream;
	INuiSensor*				m_pNuiSensor;
	;
};