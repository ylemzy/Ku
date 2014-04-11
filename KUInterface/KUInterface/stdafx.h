#ifndef _STDAFX_H_
#define _STDAFX_H_

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


#endif

