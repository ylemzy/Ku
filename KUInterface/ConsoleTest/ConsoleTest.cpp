// ConsoleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NUIContext.h"
#include <string>
typedef int			(_cdecl* NuiInitContext)(bool useColor, bool useDepth, bool useSkeleton);
typedef int			(_cdecl* NuiEnableBackgroundRemoved)(bool bOpen);
typedef int			(_cdecl* NuiEnableInteraction)(bool bOpen);
typedef int			(_cdecl* NuiUpdate)();
typedef void		(_cdecl* NuiUnInitContext)();

typedef byte*		(_cdecl*	NuiGetTextureImage			)(OUT int* size);
typedef byte*		(_cdecl*	NuiGetDepthImage			)(OUT int* size);
typedef const byte*	(_cdecl*	NuiGetBackgroundRemovedImage)(OUT int* size);
typedef void		(_cdecl*	NuiGetSkeletonTransform		)(int player, int joint, OUT KUVector4* SkeletonTransform);
typedef void		(_cdecl*	NuiGetColorImageSize		)(int * width, int * height);
typedef void		(_cdecl*	NuiGetDepthImageSize		)(int* width, int* height);
typedef int			(_cdecl*	NuiTrackedIndex				)();
typedef bool		(_cdecl*	NuiExistPlayer				)();
typedef void		(_cdecl*	NuiGetCameraAngle			)(OUT float* angle);
typedef bool		(_cdecl*	NuiSetCameraAngle			)(int angle);

typedef void (_cdecl* NuiRunTest)(bool useColor, bool useDepth, bool useSkeleton);

#define  MO
#ifdef MO

#endif

void RunTest();


void RunTest()
{
	HMODULE hModule;
	WCHAR path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path);
	wstring solutionPath(path, _tcsrchr(path, '\\'));
#ifdef DEBUG
	solutionPath.append(L"\\Debug\\");
#else
	solutionPath.append(L"\\Release\\");
#endif
	solutionPath.append(L"kinect_unity.dll");


#ifdef MO
	hModule = ::LoadLibrary(solutionPath.c_str());
#endif

	
#ifdef MO
	if (hModule)
	{
		HRESULT hr;
		NuiRunTest lpRunFunc = (NuiRunTest)(GetProcAddress(hModule, "NuiRunTest"));
		if (!lpRunFunc)
			return;
		(*lpRunFunc)(true, true, true);
	}
	FreeModule(hModule);
	hModule = NULL;
#endif
}

int _tmain(int argc, _TCHAR* argv[])
{
	RunTest();
	return 0;
}

