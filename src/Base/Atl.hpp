//
// ulib - a collection of useful classes
// Copyright (C) 2006,2007,2008,2009,2012,2014 Michael Fink
//
/// \file Atl.hpp configuration for ATL 7.x or higher
//
#pragma once

// needed includes

// exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define ATL_NO_LEAN_AND_MEAN

// no min-max macors, we use std::min / std::max instead
#define NOMINMAX

// ignore prefast warnings in ATL header files
#ifdef _PREFAST_
#pragma warning(push)
#pragma warning(disable: 6001 6011 6101 6386 6387 6506 6509 6518 28251 28252 28253 28302)
#endif

// ATL includes
#include <atlbase.h>
#include <atlstr.h>
#define _WTL_NO_CSTRING // don't use WTL CString
#define _WTL_NO_WTYPES
#include <atltypes.h>
#include <atlwin.h>

#ifdef _PREFAST_
#pragma warning(pop)
#endif

// link to static ATL libs
#ifdef _DEBUG
   #pragma comment(lib, "atlsd.lib")
#else
   #pragma comment(lib, "atls.lib")
#endif

#if (_ATL_VER <= 0x0800)
   // Visual C++ 2013 Express
   #pragma comment(linker, "/NODEFAULTLIB:atlthunk.lib")

   namespace ATL
   {
      /// allocates thunk
      inline void * __stdcall __AllocStdCallThunk()
      {
         return ::HeapAlloc(::GetProcessHeap(), 0, sizeof(_stdcallthunk));
      }

      /// frees thunk
      inline void __stdcall __FreeStdCallThunk(void *p)
      {
         ::HeapFree(::GetProcessHeap(), 0, p);
      }
   };
#else
   // Visual C++ 2013 Professional and above

   // for _stdcallthunk
   #include <atlstdthunk.h>
#endif
