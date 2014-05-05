#ifndef __STDAFX_H_
#define __STDAFX_H_

#include "targetver.h"

// Windows Header Files:
#include <windows.h>
#include <atlbase.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

using namespace std;

#ifndef KN_EXPORTS
#define KN_DLL_CLASS __declspec(dllexport)
#else
#define KN_DLL_CLASS __declspec(dllimport)
#endif

#define KN_EXPORTS

#include <nuiapi.h>
#define PI 3.14159265
#define PLAYER_ONE 254
#define PLAYER_TWO 255
#define WAIT_FRAME_TIME 0	//经过测试，最好不超过10ms，否则导致background_removed 取不到帧

/////////////////////////////////////////
namespace SenLogic
{
	

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

	

	typedef enum _MOTION_TYPE
	{
		RORATATION,
		TRANSLATION
	}MOTION_TYPE;

	typedef enum _JOINT_PART_TYPE
	{
		HEAD,
		WRIST_RIGHT
	}JOINT_PART_TYPE;

	typedef struct _KUseInfo
	{
		DWORD handEventType;
		float x, y, z; 
	}KUseInfo;

	struct MotionFragment
	{
		MOTION_TYPE motionType;
		JOINT_PART_TYPE jointType;
		double angle;
	};

	typedef struct _JointPoints
	{
		Vector4 pointA;
		Vector4 pointB;
	}JointPoints;

	typedef struct _JointPart
	{
		JointPoints points;
		JOINT_PART_TYPE type;
	}JointPart;

	typedef struct _MotionTile
	{
		DWORD frameNumber;
		Vector4 motionDir;
		Vector4 startVec;
		Vector4 endVec;
	}MotionTile;

	void CalDirection(IN const JointPoints& points, OUT Vector4& res);

	void CalDirection(IN const Vector4& pointA, IN const Vector4& pointB, OUT Vector4& res);

	void ReverseVector(Vector4& vec);

	bool CompareDirection(const Vector4& v1, const Vector4& v2, float precision = 0.02);

	double DotProduct(const Vector4& v1, const Vector4& v2);

	double DotProduct(const Vector4& v1);

	float IntersectionAngle(const Vector4& v1, const Vector4& v2);

	JOINT_PART_TYPE TransToJointPart(NUI_SKELETON_POSITION_INDEX index);

	NUI_SKELETON_POSITION_INDEX TransToSKeletonIndex(JOINT_PART_TYPE jointType);

	

}

class ScopeReference
{
public:
	ScopeReference(IUnknown* pPtr);
	~ScopeReference();

private:
	IUnknown* pPtr;
};
#endif

