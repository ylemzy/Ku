// ConsoleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include "..\KUInterface\apiObject\KinectApi.h"
void RunTest();


void RunTest()
{
	
}

int _tmain(int argc, _TCHAR* argv[])
{
	CKinectWapper::NuiRunTest(true, true, true);
	return 0;
}

