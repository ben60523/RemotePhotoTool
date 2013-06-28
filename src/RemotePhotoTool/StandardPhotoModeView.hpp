//
// RemotePhotoTool - remote camera control software
// Copyright (C) 2008-2013 Michael Fink
//
//! \file StandardPhotoModeView.hpp View for taking standard photos
//
#pragma once

// includes
#include "IPhotoModeView.hpp"
#include "CameraException.hpp"
#include "ImagePropertyValueManager.hpp"
#include "ImagePropertyCombobox.hpp"

// forward references
class IPhotoModeViewHost;

/// view for a default and simple standard photo mode
class StandardPhotoModeView :
   public CDialogImpl<StandardPhotoModeView>,
   public CDialogResize<StandardPhotoModeView>,
   public CWinDataExchange<StandardPhotoModeView>,
   public IPhotoModeView
{
public:
   /// ctor
   StandardPhotoModeView(IPhotoModeViewHost& host) throw();
   /// dtor
   virtual ~StandardPhotoModeView() throw() {}

   /// dialog id
   enum { IDD = IDD_PHOTOMODE_STANDARD_FORM };

private:
   // virtual methods from IPhotoModeView

   virtual HWND CreateView(HWND hWndParent) override
   {
      return CDialogImpl<StandardPhotoModeView>::Create(hWndParent);
   }

   virtual void SetSourceDevice(std::shared_ptr<SourceDevice> spSourceDevice) override
   {
      m_spSourceDevice = spSourceDevice;
   }

   virtual BOOL PreTranslateMessage(MSG* pMsg) override { return CWindow::IsDialogMessage(pMsg); }

   virtual void DestroyView() override
   {
      ATLVERIFY(TRUE == DestroyWindow());
   }

private:
   // ddx map
   BEGIN_DDX_MAP(StandardPhotoModeView)
      DDX_CONTROL(IDC_COMBO_SHOOTING_MODE, m_cbShootingMode)
      DDX_CONTROL(IDC_COMBO_APERTURE, m_cbAperture)
      DDX_CONTROL(IDC_COMBO_SHUTTER_SPEED, m_cbShutterSpeed)
      DDX_CONTROL(IDC_COMBO_EXPOSURE_COMP, m_cbExposureComp)
      DDX_CONTROL(IDC_COMBO_WHITE_BALANCE, m_cbWhiteBalance)
   END_DDX_MAP()

   // message map
   BEGIN_MSG_MAP(StandardPhotoModeView)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      COMMAND_HANDLER(IDC_COMBO_SHOOTING_MODE, CBN_SELCHANGE, OnShootingModeSelChange)
      COMMAND_ID_HANDLER(IDC_BUTTON_RELEASE, OnButtonRelease)
      REFLECT_NOTIFICATIONS() // to make sure superclassed controls get notification messages
   END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
// LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
// LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
// LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

   LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnShootingModeSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnButtonRelease(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

   /// called when property has changed
   void OnUpdatedProperty(RemoteReleaseControl::T_enPropertyEvent enPropertyEvent, unsigned int uiValue);

   /// called when image has finished transfer
   void OnFinishedTransfer(const ShutterReleaseSettings& settings);

   /// sets up ImagePropertyValueManager
   void SetupImagePropertyManager();

   /// updates values that depend on shooting mode changes
   void UpdateShootingModeDependentValues();

private:
   /// host access
   IPhotoModeViewHost& m_host;

   // UI

   /// image property value manager
   std::unique_ptr<ImagePropertyValueManager> m_upImagePropertyValueManager;

   /// shooting mode
   ImagePropertyCombobox m_cbShootingMode;

   /// aperture
   ImagePropertyCombobox m_cbAperture;

   /// shutter speed
   ImagePropertyCombobox m_cbShutterSpeed;

   /// exposure compensation
   ImagePropertyCombobox m_cbExposureComp;

   /// white balance
   ImagePropertyCombobox m_cbWhiteBalance;

   /// id for property handler
   int m_iPropertyHandlerId;

   // model

   /// source device
   std::shared_ptr<SourceDevice> m_spSourceDevice;

   /// remote release control
   std::shared_ptr<RemoteReleaseControl> m_spRemoteReleaseControl;
};