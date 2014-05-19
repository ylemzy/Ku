// ConsoleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include "..\KUInterface\apiObject\KinectApi.h"
int _tmain(int argc, _TCHAR* argv[])
{
	CKinectWapper::NuiInitContext(true, true, true);
	CKinectWapper::NuSetBackgroundRemovedCount(2);
	HANDLE h = CKinectWapper::StartProcessData();
	::WaitForSingleObject(h, INFINITE);
	return 0;
}

