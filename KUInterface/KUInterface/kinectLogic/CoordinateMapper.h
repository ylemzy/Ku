#ifndef __COORDINATE_MAPPER_H_
#define __COORDINATE_MAPPER_H_

interface INuiCoordinateMapper;
class CoordinateMapper
{
	friend class SensorContext;
public:
	CoordinateMapper(const SensorContext* pSensorContext);
	~CoordinateMapper();

public:
	HRESULT MapSkeletonPointToColorPoint(Vector4* pSkeletonPoint, NUI_COLOR_IMAGE_POINT* pColorPoint);
	HRESULT MapSkeletonPointToDepthPoint(Vector4* pSkeletonPoint, NUI_DEPTH_IMAGE_POINT* pDepthPoint);
private:
	const SensorContext* m_pSensorContext;
};
#endif