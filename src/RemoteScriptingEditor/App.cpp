//
// RemotePhotoTool - remote camera control software
// Copyright (C) 2008-2014 Michael Fink
//
/// \file RemoteScriptingEditor/App.cpp Application
//

// includes
#include "stdafx.h"
#include "App.hpp"
#include "res\Ribbon.h"
#include "resource.h"
#include "MainFrame.hpp"
#include "Filesystem.hpp"
#include "CrashReporter.hpp"
#include <crtdbg.h>

/// WTL app module
CAppModule _Module;

App::App(HINSTANCE hInstance)
{
#ifdef _DEBUG
   // turn on leak-checking
   int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
   _CrtSetDbgFlag(flag | _CRTDBG_LEAK_CHECK_DF);
#endif

   // note: Apartment Threading needed for Canon ED-SDK
   HRESULT hRes = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
   ATLASSERT(SUCCEEDED(hRes));

   // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
#pragma warning(suppress: 6387)
   ::DefWindowProc(NULL, 0, 0, 0L);

   AtlInitCommonControls(ICC_WIN95_CLASSES);

   hRes = _Module.Init(NULL, hInstance);
   ATLASSERT(SUCCEEDED(hRes));
}

App::~App() throw()
{
   _Module.Term();
   ::CoUninitialize();
}

void App::InitCrashReporter()
{
   CString cszFolder = App_GetAppDataFolder(appDataUserNonRoaming) + _T("\\RemotePhotoTool\\");

   if (!Directory_Exists(cszFolder))
      CreateDirectory(cszFolder, NULL);

   cszFolder += _T("crashdumps\\");

   if (!Directory_Exists(cszFolder))
      CreateDirectory(cszFolder, NULL);

   CrashReporter::Init(cszFolder);
}

int App::Run(LPCTSTR /*lpstrCmdLine*/, int nCmdShow)
{
   CMessageLoop theLoop;
   _Module.AddMessageLoop(&theLoop);

   MainFrame wndMain;

   if (wndMain.CreateEx() == NULL)
   {
      ATLTRACE(_T("Main window creation failed!\n"));
      return 0;
   }

   wndMain.ShowWindow(nCmdShow);

   int nRet = theLoop.Run();

   _Module.RemoveMessageLoop();
   return nRet;
}