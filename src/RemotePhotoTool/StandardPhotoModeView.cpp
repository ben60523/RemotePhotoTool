//
// RemotePhotoTool - remote camera control software
// Copyright (C) 2008-2014 Michael Fink
//
/// \file StandardPhotoModeView.cpp View for taking standard photos
//

// includes
#include "stdafx.h"
#include "resource.h"
#include "StandardPhotoModeView.hpp"
#include "IPhotoModeViewHost.hpp"
#include "ImageFileManager.hpp"
#include "CameraErrorDlg.hpp"
#include "ShutterReleaseSettings.hpp"
#include "ShootingMode.hpp"

StandardPhotoModeView::StandardPhotoModeView(IPhotoModeViewHost& host) throw()
:m_host(host),
 m_cbShootingMode(propShootingMode),
 m_cbAperture(propAv),
 m_cbShutterSpeed(propTv),
 m_cbExposureComp(propExposureCompensation),
 m_cbWhiteBalance(propWhiteBalance),
 m_iPropertyHandlerId(-1)
{
}

LRESULT StandardPhotoModeView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   DoDataExchange(DDX_LOAD);

   m_spRemoteReleaseControl = m_host.StartRemoteReleaseControl(true);
   if (m_spRemoteReleaseControl == nullptr)
   {
      AtlMessageBox(m_hWnd, _T("Couldn't start remote release control."), IDR_MAINFRAME, MB_OK);
      DestroyWindow();
      return 0;
   }

   SetupImagePropertyManager();

   // shooting mode change supported?
   if (!m_spRemoteReleaseControl->GetCapability(RemoteReleaseControl::capChangeShootingMode))
   {
      // no: disable shooting mode combobox
      m_cbShootingMode.EnableWindow(FALSE);
   }
   else
   {
      // yes: wait for changes in shooting mode property
      m_iPropertyHandlerId = m_spRemoteReleaseControl->AddPropertyEventHandler(
         std::bind(&StandardPhotoModeView::OnUpdatedProperty, this, std::placeholders::_1, std::placeholders::_2));
   }

   // set default release settings
   try
   {
      ShutterReleaseSettings settings(ShutterReleaseSettings::saveToBoth,
         std::bind(&StandardPhotoModeView::OnFinishedTransfer, this, std::placeholders::_1));

      CString cszFilename =
         m_host.GetImageFileManager().NextFilename(imageTypeNormal);
      settings.Filename(cszFilename);

      m_spRemoteReleaseControl->SetReleaseSettings(settings);
   }
   catch(CameraException& ex)
   {
      CameraErrorDlg dlg(_T("Error while setting default shooting settings"), ex);
      dlg.DoModal();
      return FALSE;
   }

   return TRUE;
}

LRESULT StandardPhotoModeView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   // note: don't reset default release settings; must be done in new view

   if (m_iPropertyHandlerId != -1)
      m_spRemoteReleaseControl->RemovePropertyEventHandler(m_iPropertyHandlerId);

   m_upImagePropertyValueManager.reset();

   return 0;
}

LRESULT StandardPhotoModeView::OnShootingModeSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   UpdateShootingModeDependentValues();

   return 0;
}

LRESULT StandardPhotoModeView::OnButtonRelease(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   ShutterReleaseSettings settings(ShutterReleaseSettings::saveToBoth,
      std::bind(&StandardPhotoModeView::OnFinishedTransfer, this, std::placeholders::_1));

   CString cszFilename =
      m_host.GetImageFileManager().NextFilename(imageTypeNormal);
   settings.Filename(cszFilename);

   m_spRemoteReleaseControl->SetReleaseSettings(settings);

   m_host.SetStatusText(_T("Started shutter release"));

   m_host.LockActionMode(true);

   try
   {
      m_spRemoteReleaseControl->Release();
   }
   catch(CameraException& ex)
   {
      CameraErrorDlg dlg(_T("Couldn't release shutter"), ex);
      dlg.DoModal();
   }
   return 0;
}

void StandardPhotoModeView::OnUpdatedProperty(RemoteReleaseControl::T_enPropertyEvent enPropertyEvent, unsigned int uiValue)
{
   if (enPropertyEvent == RemoteReleaseControl::propEventPropertyChanged &&
      uiValue == m_spRemoteReleaseControl->MapImagePropertyTypeToId(propShootingMode))
   {
      // shooting mode has changed; update some fields
      UpdateShootingModeDependentValues();
   }
}

void StandardPhotoModeView::OnFinishedTransfer(const ShutterReleaseSettings& settings)
{
   CString cszText;
   cszText.Format(_T("Finished transfer: %s"), settings.Filename().GetString());

   m_host.SetStatusText(cszText);

   m_host.LockActionMode(false);
}

void StandardPhotoModeView::SetupImagePropertyManager()
{
   m_upImagePropertyValueManager.reset(new ImagePropertyValueManager(*m_spRemoteReleaseControl));

   m_cbShootingMode.SetRemoteReleaseControl(m_spRemoteReleaseControl);
   m_cbAperture.SetRemoteReleaseControl(m_spRemoteReleaseControl);
   m_cbShutterSpeed.SetRemoteReleaseControl(m_spRemoteReleaseControl);
   m_cbExposureComp.SetRemoteReleaseControl(m_spRemoteReleaseControl);
   m_cbWhiteBalance.SetRemoteReleaseControl(m_spRemoteReleaseControl);

   m_upImagePropertyValueManager->AddControl(m_cbShootingMode);
   m_upImagePropertyValueManager->AddControl(m_cbAperture);
   m_upImagePropertyValueManager->AddControl(m_cbShutterSpeed);
   m_upImagePropertyValueManager->AddControl(m_cbExposureComp);
   m_upImagePropertyValueManager->AddControl(m_cbWhiteBalance);

   m_upImagePropertyValueManager->UpdateControls();

   m_upImagePropertyValueManager->UpdateProperty(
      m_spRemoteReleaseControl->MapImagePropertyTypeToId(propShootingMode));
}

void StandardPhotoModeView::UpdateShootingModeDependentValues()
{
   ShootingMode shootingMode(m_spRemoteReleaseControl);

   ImageProperty currentMode = shootingMode.Current();

   bool bIsM  = currentMode.Value() == shootingMode.Manual().Value();
   bool bIsAv = currentMode.Value() == shootingMode.Av().Value();
   bool bIsTv = currentMode.Value() == shootingMode.Tv().Value();
   bool bIsP  = currentMode.Value() == shootingMode.Program().Value();

   bool bReadOnlyAv = !bIsM && (bIsTv || bIsP);
   bool bReadOnlyTv = !bIsM && (bIsAv || bIsP);
   bool bReadOnlyExp = bIsM;

   m_cbAperture.UpdateValuesList();
   m_cbAperture.UpdateValue();
   m_cbAperture.EnableWindow(!bReadOnlyAv);

   m_cbShutterSpeed.UpdateValuesList();
   m_cbShutterSpeed.UpdateValue();
   m_cbShutterSpeed.EnableWindow(!bReadOnlyTv);

   m_cbExposureComp.UpdateValuesList();
   m_cbExposureComp.UpdateValue();
   m_cbExposureComp.EnableWindow(!bReadOnlyExp);
}
