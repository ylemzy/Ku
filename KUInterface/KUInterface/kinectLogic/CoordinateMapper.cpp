#include "stdafx.h"
#include "CoordinateMapper.h"
#include "KinectContext.h"

CoordinateMapper::CoordinateMapper(const SensorContext* pSensorContext)
	: m_pSensorContext(pSensorContext)
{
	
}

CoordinateMapper::~CoordinateMapper()
{
}

HRESULT CoordinateMapper::MapSkeletonPointToColorPoint(Vector4* pSkeletonPoint, NUI_COLOR_IMAGE_POINT* pColorPoint)
{
	if (!m_pSensorContext)
		return E_FAIL;

	if (!pSkeletonPoint || !pColorPoint)
		return E_INVALIDARG;

	INuiCoordinateMapper* pNuiCoordinateMapper = NULL;
	HRESULT hr = S_OK;
	INuiSensor* pSensor = m_pSensorContext->GetSensor();
	if (FAILED(hr = pSensor->NuiGetCoordinateMapper(&pNuiCoordinateMapper)))
		return hr;

	
	hr = pNuiCoordinateMapper->MapSkeletonPointToColorPoint(
		pSkeletonPoint, 
		NUI_IMAGE_TYPE_COLOR,
		SensorContext::cColorResolution, pColorPoint);
	return hr;
}

HRESULT CoordinateMapper::MapSkeletonPointToDepthPoint(Vector4* pSkeletonPoint, NUI_DEPTH_IMAGE_POINT* pDepthPoint)
{
	if (!m_pSensorContext)
		return E_FAIL;

	if (!pSkeletonPoint || !pDepthPoint)
		return E_INVALIDARG;

	INuiCoordinateMapper* pNuiCoordinateMapper = NULL;
	HRESULT hr = S_OK;
	INuiSensor* pSensor = m_pSensorContext->GetSensor();
	if (FAILED(hr = pSensor->NuiGetCoordinateMapper(&pNuiCoordinateMapper)))
		return hr;


	hr = pNuiCoordinateMapper->MapSkeletonPointToDepthPoint(
		pSkeletonPoint,
		SensorContext::cDepthResolution, pDepthPoint);

	return hr;
}
