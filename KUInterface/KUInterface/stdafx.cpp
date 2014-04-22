
#include "stdafx.h"
#include <math.h>

void SenLogic::CalDirection(IN const Vector4& pointA, IN const Vector4& pointB, OUT Vector4& res)
{
	res.x = pointB.x - pointA.x;
	res.y = pointB.y - pointA.y;
	res.z = pointB.z - pointA.z;
}

void SenLogic::CalDirection(IN const JointPoints& points, OUT Vector4& res)
{
	CalDirection(points.pointA, points.pointB, res);
}

void SenLogic::ReverseVector(Vector4& vec)
{
	vec.x = -vec.x;
	vec.y = -vec.y;
	vec.z = -vec.z;
	vec.w = -vec.w;
}

bool SenLogic::CompareDirection(const Vector4& v1, const Vector4& v2, float precision)
{
	return fabs(v1.x - v2.x) <= precision 
		&& fabs(v1.y - v1.y) <= precision
		&& fabs(v1.z - v2.z) <= precision;
}

double SenLogic::DotProduct(const Vector4& v1, const Vector4& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

double SenLogic::DotProduct(const Vector4& v1)
{
	return DotProduct(v1, v1);
}

float SenLogic::IntersectionAngle(const Vector4& v1, const Vector4& v2)
{
	if (CompareDirection(v1, v2, 0))
		return 0;

	double AB = DotProduct(v1, v2);
	double AA = DotProduct(v1);
	double BB = DotProduct(v2);
	double cosValue = AB * 1.0 / sqrt(AA) / sqrt(BB);
	return acos(cosValue) * 180 / PI;
}

SenLogic::JOINT_PART_TYPE SenLogic::TransToJointPart(NUI_SKELETON_POSITION_INDEX index)
{
	_JOINT_PART_TYPE jointType = WRIST_RIGHT;
	return jointType;
}

NUI_SKELETON_POSITION_INDEX SenLogic::TransToSKeletonIndex(JOINT_PART_TYPE jointType)
{
	NUI_SKELETON_POSITION_INDEX index = NUI_SKELETON_POSITION_ELBOW_RIGHT;
	return index;
}



ScopeReference::ScopeReference(IUnknown* pPtr)
{
	if (pPtr)
		pPtr->AddRef();
}

ScopeReference::~ScopeReference()
{
	if (pPtr)
		pPtr->Release();
}
