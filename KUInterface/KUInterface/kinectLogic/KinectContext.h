
#pragma once

#include "stdafx.h"
#include "NuiApi.h"
#include <assert.h>
#include <math.h>
#include <KinectBackgroundRemoval.h>
#include <KinectInteraction.h>

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

	REALEASE_HANDEL_ERROR = -18,

	CREATE_INTERACTION_ERROR = -19,
	ENABLE_INTERACTION_ERROR = -20,

	INTERACTION_PROCESS_DEPTH_ERROR = -21,
	INTERACTION_PROCESS_SKELETON_ERROR = -22,
	ACCELEROMETER_GET_CURRENT_READING_ERROR = -23,
	GET_INTERACTION_FRAME_ERROR = -24,
};

#define MAKE_RUNTIMERESULT(pHr, value)	\
	if (pHr && *pHr == SUCCEEDED_OK) {	\
		*pHr = value;	\
	}

#define CHECK_WITH_RUNTIMERESULT(hr, pHr, value) \
	if (FAILED(hr))	\
		MAKE_RUNTIMERESULT(pHr, value)

#define CHECK_RUTURN_WITH_RUNTIMERESULT(hr, pRt, value) \
	if (FAILED(hr)){ \
		MAKE_RUNTIMERESULT(pRt, value) \
		return hr;\
	}

#define CHECK_KN_EXIT(hr, EXIT)	\
	if (FAILED(hr))			\
		goto EXIT;		\


#define CLOSE_HANDLE(h)		\
	if (h != INVALID_HANDLE_VALUE)\
		CloseHandle(h);			\
	h = INVALID_HANDLE_VALUE;

#define IS_VALID_HANDLE(h)	\
	h != INVALID_HANDLE_VALUE

#define EXIST_FLAG(var, flag)	\
	var & flag

//注意变量和标记参数位置，另外～优先级比&高
#define NO_EXIST_FLAG(var, flag) \
	~var & flag

class KN_DLL_CLASS SensorContext
{
	static const int		cBytesPerPixel = 4;

	//interaction时，必须640×480，否则处理深度数据会成为参数错误
	static const NUI_IMAGE_RESOLUTION	cDepthResolution = NUI_IMAGE_RESOLUTION_640x480;
	static const NUI_IMAGE_RESOLUTION	cColorResolution = NUI_IMAGE_RESOLUTION_640x480;

public:
	SensorContext();
	~SensorContext();

public:
	HRESULT					InitSensor(DWORD flag, RUNTIME_RESULT* rtHr = 0);
	void                    UnInitSensor();
	void					TransformCoordinates(OUT KVector4* skTrans);
	HRESULT					ProcessSkeleton(RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessColor(RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessDepth(RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessBackgroundRemoved(RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessInteraction(RUNTIME_RESULT* rtHr = 0);

	HRESULT					SetCameraAngle(long angle);

	HRESULT					SetEnableColor(BOOL bEnabled, RUNTIME_RESULT* rtHr);
	HRESULT					SetEnableDepth(BOOL bEnabled, RUNTIME_RESULT* rtHr);
	HRESULT					SetEnableSkeleton(BOOL bEnabled, RUNTIME_RESULT* rtHr);
	HRESULT					SetEnableBackgroundRemovedColorStream(BOOL bEnabled, RUNTIME_RESULT* rtHr = 0);

	BOOL					IsBackgroundRemovedEnabled() const;
	BOOL					IsColorEnabled() const;
	BOOL					IsDepthEnabled() const;
	BOOL					IsSkeletonEnabled() const;
	BOOL					IsInteractionEnabled() const;

	HRESULT					SetEnableInteractioin(BOOL bEnabled, RUNTIME_RESULT* rtHr);
private:
	HRESULT					FindSensor(RUNTIME_RESULT* rtHr = 0);
	void					ResetColorData();
	void					ResetDepthData();
	void					ResetSkeletonData();
	void					ResetBackgroundRemovedColorData();
	void					ResetInteractionData();

	HRESULT					PickHandEventType(
								const NUI_HANDPOINTER_INFO* pHandPointerInfo,
								NUI_HAND_EVENT_TYPE lastEventType = NUI_HAND_EVENT_TYPE_NONE,
								NUI_HAND_EVENT_TYPE* pEventType = 0) const;

	HRESULT					UpdateLastHandType(IN const NUI_USER_INFO* pUserInfoArray, OUT NUI_USER_INFO* pLastUserInfoArray) const;
	void					DebugHandStateMsg(NUI_HAND_TYPE hand, DWORD state) const;
	void					DebugHandEventType(NUI_HAND_EVENT_TYPE t) const;
	bool					IsValidUseInfo(const NUI_USER_INFO* pUserInfo) const;
public:
	NUI_SKELETON_DATA		m_skData;
	NUI_SKELETON_DATA		m_skData2;
	NUI_USER_INFO			m_mainUserInfo;
	LONG					angle;

	bool					m_bSkeletonValid;			//表示是否是最新有效数据
	bool					m_bImageValid;				//表示是否是最新有效数据
	bool					m_bDepthValid;				//表示是否是最新有效数据
	bool					m_bBackgroundRemovedValid;	//表示是否是最新有效数据
	bool					m_bInteractionValid;		

	byte*					m_pImageData;
	int						m_nImageSize;
	byte*					m_pDepthData;
	int						m_nDepthSize;
	byte*					m_pBackgroundRemovedData;
	int						m_nBackgroundRemovedSize;

	BOOL					m_bNearMode;
	LONG					m_colorWidth;
	LONG					m_colorHeight;
	LONG					m_depthWidth;
	LONG					m_depthHeight;
	int						m_skTackedId;

public:
	HANDLE					m_hNextColorFrameEvent;
	HANDLE					m_hNextDepthFrameEvent;
	HANDLE					m_hNextSkeletonFrameEvent;
	HANDLE					m_hNextBackgroundRemovedFrameEvent;
	HANDLE					m_hNextInteractionFrameEvent;

private:
	HANDLE					m_hColorStream;
	HANDLE					m_hDepthStream;

	INuiBackgroundRemovedColorStream* m_pBackgroundRemovalStream;
	INuiInteractionClient*	m_pNuiInteractionClient;
	INuiInteractionStream* m_pNuiInteractionStream;
	
	INuiSensor*				m_pNuiSensor;

};

class KinectAdapter: public INuiInteractionClient
{
public:
	KinectAdapter();
	~KinectAdapter();

public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv);

	ULONG STDMETHODCALLTYPE AddRef();

	ULONG STDMETHODCALLTYPE Release();

	HRESULT STDMETHODCALLTYPE GetInteractionInfoAtLocation(DWORD skeletonTrackingId, 
		NUI_HAND_TYPE handType, 
		FLOAT x, 
		FLOAT y, 
		_Out_ NUI_INTERACTION_INFO *pInteractionInfo);
};