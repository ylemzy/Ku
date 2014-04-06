// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <atlbase.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

using namespace std;

#pragma comment (lib, "d3d9.lib")
#pragma comment ( lib, "winmm.lib" )

//Custom Vector struct for easier export
typedef struct _KUVector4
{
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT w;
} KUVector4;

typedef struct _KUUseInfo
{
	DWORD handEventType;
	FLOAT x, y, z;
}KUUseInfo;