// ConsoleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include "..\KUInterface\apiObject\KinectApi.h"
int _tmain(int argc, _TCHAR* argv[])
{
	if (SenLogic::SUCCEEDED_OK != CKinectWapper::NuiInitContext(true, true, true))
	{
		return 0;
	}
	CKinectWapper::NuSetBackgroundRemovedCount(2);
	HANDLE h = CKinectWapper::StartProcessData();
	::WaitForSingleObject(h, INFINITE);
	return 0;
}

