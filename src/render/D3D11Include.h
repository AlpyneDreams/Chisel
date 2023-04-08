#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOCRYPT
#define NOMETAFILE
#define NOMINMAX
#define MMNOSOUND

#include <windows.h>
#include <unknwn.h>

#include <d3d11_1.h>
#include <dxgi.h>

// Sad macros :~( - Grabbed these from around the codebase
#undef PostMessage
#undef CreateEvent
#undef MessageBox
#undef PlaySound
#undef SetCursor
#undef ProgressBox
#undef RegisterClass
#undef AddJob
#undef GetJob
#undef Yield
#undef SetPort
#undef CreateFont
#undef ShellExecute
#undef ShellExecuteEx
#undef GetCurrentTime
#undef GetTickCount
#undef GetCurrentDirectory
#undef GetObject
#undef LoadCursorFromFile
#undef GetCharABCWidths
#undef CreateWindow
#undef far
#undef near

#include "Com.h"
