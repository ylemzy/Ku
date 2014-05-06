#ifndef __KINECT_CONTEXT_H_
#define __KINECT_CONTEXT_H_

#include "NuiApi.h"
#include <assert.h>
#include <math.h>
#include <KinectBackgroundRemoval.h>
#include <KinectInteraction.h>
#include <vector>
#include <map>
//#include <string>
////#include <iostream>
//#include <fstream>

using namespace SenLogic;
using namespace std;

#define INVALID_SKELETON_ARRAY_INDEX -1
/*
颜色，深度，骨架，背景去除，interaction的数据都各自集成。
所有设计skeleton的数据，都需要对应skeleton tracked id，
外部设计接口，都需要通过tracked id 获取对应数据
*/
typedef map<DWORD, UINT> SkeletonIdMap;
struct SkeletonData{

	friend class SensorContext;
public:
	SkeletonData();
	void Reset();
	
	UINT GetFullSkeletonCount() const;
	UINT GetPositionOnlyCount() const;
	UINT GetSkeletonCount()	const;
	DWORD GetFullTrackedId(UINT uIndex) const;
	DWORD GetPositionOnlyTrackedId(UINT uIndex) const;//从0开始
	const NUI_SKELETON_DATA* GetSkeletonIndexByTrackedId(DWORD trackedId) const;
	bool ExistFullTrackedID(DWORD trackedId) const;
	NUI_SKELETON_DATA		m_dataArray[NUI_SKELETON_COUNT];
	
	HANDLE					m_hEvent;
	LARGE_INTEGER			m_timeStamp;
	SkeletonIdMap		m_fullTrackedMap;
	SkeletonIdMap		m_unFullTrackedMap;
};


struct FrameData
{
	/*color image 或者depth image，background removed image基本数据
	*/
	friend class SensorContext;
public:
	FrameData();
	FrameData(NUI_IMAGE_RESOLUTION res);
	void Reset();
	void SetResolution(NUI_IMAGE_RESOLUTION res);
	DWORD GetSize(bool bInByte = true) const;
	void GetSize(DWORD& width, DWORD& height) const;
	DWORD GetWidth() const;
	DWORD GetHeight() const;
	NUI_IMAGE_RESOLUTION GetResolution() const;
	bool CopyData(const byte* data, DWORD size, const LARGE_INTEGER& timeStamp);
	bool IsValid() const;
	const byte* GetData() const;
	const LARGE_INTEGER& GetTimeStamp() const;
	bool Compose(const byte* pData, NUI_IMAGE_RESOLUTION res);
	bool Compose(const FrameData* pData);
private:
	byte* m_pData;
	NUI_IMAGE_RESOLUTION m_resolution;
	LARGE_INTEGER m_timeStamp;
	bool m_bInvalid;
};

struct BackGroudRemvoedData
{
	//note:一个INuiBackgroundRemovedColorStream只能处理一个full skeleton
	friend class SensorContext;
public:
	BackGroudRemvoedData();
	void Reset();
	DWORD GetTrackedId() const;
	const FrameData* GetFrameData() const;

	INuiBackgroundRemovedColorStream* m_pStream;
	HANDLE m_hEvent;

private:
	DWORD m_trackedId;//跟踪过的骨架id
	FrameData m_frameData;
};

typedef struct ImageData: public FrameData
{
	friend class SensorContext;
public:
	ImageData();
	void Reset();

	HANDLE m_hStream;
	HANDLE m_hEvent;
}ColorData, DepthData;

struct InteractionData
{
	//note:一个INuiInteractionStream同时处理多个full skeleton
	friend class SensorContext;
public:
	InteractionData();
	void Reset();
	UINT GetCount() const;
	DWORD GetTrackedId(UINT index) const;
	const NUI_USER_INFO* GetUserInfoByTrackedId(DWORD index) const;
public:
	INuiInteractionClient* m_pClient;
	INuiInteractionStream* m_pStream;
	HANDLE					m_hEvent;

private:
	NUI_USER_INFO			m_userInfos[6];
	SkeletonIdMap			m_userIdMap;//已经获取到的骨架以及对应userInfo下标
};

class KN_DLL_CLASS SensorContext
{
	
public:
	static const int		cBytesPerPixel = 4;

	//interaction时，必须640×480，否则处理深度数据会成为参数错误
	static const NUI_IMAGE_RESOLUTION	cDepthResolution = NUI_IMAGE_RESOLUTION_640x480;
	static const NUI_IMAGE_RESOLUTION	cColorResolution = NUI_IMAGE_RESOLUTION_1280x960;

public:
	SensorContext();
	~SensorContext();

public:
	HRESULT					InitSensor(DWORD flag, RUNTIME_RESULT* rtHr = 0);
	void                    UnInitSensor();
	void					TransformCoordinates(OUT Vector4* skTrans);
	HRESULT					ProcessSkeleton(RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessColor(RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessDepth(RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessBackgroundRemoved(UINT index, RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessAllBackgroundRemoved(RUNTIME_RESULT* rtHr = 0);
	HRESULT					ProcessInteraction(RUNTIME_RESULT* rtHr = 0);

	HRESULT					SetCameraAngle(long angle);

	HRESULT					SetColorEnabled(BOOL bEnabled, RUNTIME_RESULT* rtHr);
	HRESULT					SetDepthEnabled(BOOL bEnabled, RUNTIME_RESULT* rtHr);
	HRESULT					SetSkeletonEnabled(BOOL bEnabled, RUNTIME_RESULT* rtHr);
	HRESULT					SetBackgroundRemovedCount(UINT peopelCount, RUNTIME_RESULT* rtHr = 0);

	BOOL					IsBackgroundRemovedEnabled() const;
	BOOL					IsColorEnabled() const;
	BOOL					IsDepthEnabled() const;
	BOOL					IsSkeletonEnabled() const;
	BOOL					IsInteractionEnabled() const;

	HRESULT					SetInteractionCount(UINT peopelCount, RUNTIME_RESULT* rtHr);
	INuiSensor*				GetSensor() const;

	const ColorData*		GetColorData() const;
	const DepthData*		GetDepthData() const;
	UINT					GetSkeletonSize() const;
	UINT					GetBackgroundRemovedCount() const;
	const FrameData*		GetBackgroundRemovedComposed() const;
	const BackGroudRemvoedData*		GetBackgroundRemovedData(UINT index) const;
	const InteractionData*	GetInteractionData() const;
	const SkeletonData*		GetSkeletonData() const;
	UINT					GetInteractionCount() const;

	void					SetBackgroundReomovedComposed(bool bCompose);
	bool					IsBackgroundRemovedComposed();
private:
	HRESULT					ComposeBackgroundRemoved(UINT index);//数组下标
	
	HRESULT					ProcessDepthInBackgroundRemoved(RUNTIME_RESULT* rtHr);
	HRESULT					ProcessDepthInInteraction(RUNTIME_RESULT* rtHr);
	HRESULT					CopyTextureData(IN INuiFrameTexture* pTexture, 
								const LARGE_INTEGER& timeStamp, 
								OUT ImageData& dst, 
								RUNTIME_RESULT* rtHr);
	bool					PickValidSkeleton(const NUI_SKELETON_DATA pNewData[]);
	
	HRESULT					ProcessSkeletonInBackgroundRemoved(const NUI_SKELETON_DATA* skeletons, RUNTIME_RESULT* rtHr);
	HRESULT					ProcessSkeletonInInteraction(const NUI_SKELETON_DATA* skeletons, RUNTIME_RESULT* rtHr);

	HRESULT					ProcessColorInBackgroundRemoved(RUNTIME_RESULT* rtHr);

	HRESULT					FindSensor(RUNTIME_RESULT* rtHr = 0);
	void					ResetColorData();
	void					ResetDepthData();
	void					ResetSkeletonData();
	void					ResetBackgroundRemovedColorData();
	void					ResetInteractionData();
	bool					IsDepthPointValid(const NUI_DEPTH_IMAGE_POINT& point) const;

	HRESULT					PickHandEventType(
								const NUI_HANDPOINTER_INFO* pHandPointerInfo,
								NUI_HAND_EVENT_TYPE lastEventType = NUI_HAND_EVENT_TYPE_NONE,
								NUI_HAND_EVENT_TYPE* pEventType = 0) const;

	HRESULT					UpdateLastHandType(IN const NUI_USER_INFO* pUserInfoArray, OUT NUI_USER_INFO* pLastUserInfoArray) const;
	void					DebugHandStateMsg(NUI_HAND_TYPE hand, DWORD state) const;
	void					DebugHandEventType(NUI_HAND_EVENT_TYPE t) const;
	bool					IsValidUseInfo(const NUI_USER_INFO* pUserInfo) const;

	void					WriteFrameData(const FrameData* pData) const;
	void					ProcessDepthForFullHead() const;
public:
	SkeletonData			m_skeletonData;
	BOOL					m_bNearMode;

public:
	ColorData				m_colorData;
	DepthData				m_depthData;

	FrameData				m_backgroundComposed;
public:
	vector<BackGroudRemvoedData> m_backGrmDataVec;
	SkeletonIdMap			m_backGrmIdMap;

public:
	InteractionData			m_interActData;	
	bool					m_bInterActEnabled;//是否开启interaction
private:
	INuiSensor*				m_pNuiSensor;
	LONG					nKinecAngle;
	bool					m_bComposed;//多个背景去除组合
	//ofstream				myfile;

	int personCount;
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