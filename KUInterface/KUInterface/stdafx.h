// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

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
//Custom Vector struct for easier export
struct KVector4
{
	float x;
	float y;
	float z;
	float w;
};

struct KUseInfo
{
	DWORD handEventType;
	float x, y, z;
};