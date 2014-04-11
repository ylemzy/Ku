#ifndef _KINECT_CONTEXT_H_
#define _KINECT_CONTEXT_H_

#include "NuiApi.h"
#include <assert.h>
#include <math.h>
#include <KinectBackgroundRemoval.h>
#include <KinectInteraction.h>
#include "..\include\common.h"



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

#endif