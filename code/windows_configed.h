#ifndef HY3D_WINDOWS_H
#define HY3D_WINDOWS_H 1
// target Windows 7 or later
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include <sdkddkver.h>
// NOTE: Credit for most of this goes to ChiliTomatoNoodle
// The following #defines disable a bunch of unused windows stuff. If you
// get weird errors when trying to do some windows stuff, try removing some
// (or all) of these defines (it will increase build time though).
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
//#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
//#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE
//#define NMHDR

#define NOMINMAX

#define STRICT

#include <Windows.h>
//#include "commdlg.h" // for open file dialog 

#endif
