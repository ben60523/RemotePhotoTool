//
// RemotePhotoTool - remote camera control software
// Copyright (C) 2008-2018 Michael Fink
//
/// \file TimeLapsePhotoModeView.hpp Timelapse photo mode view
//
#pragma once

// includes
#include "IPhotoModeView.hpp"
#include "PhotoModeManager.hpp"
#include "ImagePropertyCombobox.hpp"

// forward references
class IPhotoModeViewHost;

/// timelapse photo mode view
class TimeLapsePhotoModeView :
   public CDialogImpl<TimeLapsePhotoModeView>,
   public CDialogResize<TimeLapsePhotoModeView>,
   public CWinDataExchange<TimeLapsePhotoModeView>,
   public IPhotoModeView
{
public:
   /// ctor
   TimeLapsePhotoModeView(IPhotoModeViewHost& host);
   /// dtor
   virtual ~TimeLapsePhotoModeView() {}

   /// dialog id
   enum { IDD = IDD_PHOTOMODE_TIMELAPSE_FORM };

private:
   // virtual methods from IPhotoModeView

   virtual HWND CreateView(HWND hWndParent) override
   {
      return CDialogImpl<TimeLapsePhotoModeView>::Create(hWndParent);
   }

   virtual BOOL PreTranslateMessage(MSG* pMsg) override
   {
      return CWindow::IsDialogMessage(pMsg);
   }

   virtual bool CanClose() const override;

   virtual void DestroyView() override
   {
      ATLVERIFY(TRUE == DestroyWindow());
   }

private:
   // DDX map
   BEGIN_DDX_MAP(TimeLapsePhotoModeView)
      DDX_CONTROL_HANDLE(IDC_BUTTON_TIMELAPSE_START, m_btnStart)
      DDX_CONTROL_HANDLE(IDC_BUTTON_TIMELAPSE_STOP, m_btnStop)
      DDX_CONTROL_HANDLE(IDC_DATETIMEPICKER_TIMELAPSE_RELEASE_TRIGGER_INTERVAL, m_timePickerReleaseTriggerInterval)
      DDX_RADIO(IDC_RADIO_TIMELAPSE_RELEASE_TRIGGER_INTERVAL, m_releaseTriggerRadio)
      DDX_CHECK(IDC_CHECKBOX_TIMELAPSE_SCHEDULE_STARTTIME, m_scheduleIsCheckedStartTime)
      DDX_CHECK(IDC_CHECKBOX_TIMELAPSE_SCHEDULE_ENDTIME, m_scheduleIsCheckedEndTime)
      DDX_CONTROL_HANDLE(IDC_DATETIMEPICKER_TIMELAPSE_SCHEDULE_START_DATE, m_timePickerScheduleStartDate)
      DDX_CONTROL_HANDLE(IDC_DATETIMEPICKER_TIMELAPSE_SCHEDULE_START_TIME, m_timePickerScheduleStartTime)
      DDX_CONTROL_HANDLE(IDC_DATETIMEPICKER_TIMELAPSE_SCHEDULE_END_DATE, m_timePickerScheduleEndDate)
      DDX_CONTROL_HANDLE(IDC_DATETIMEPICKER_TIMELAPSE_SCHEDULE_END_TIME, m_timePickerScheduleEndTime)
      DDX_CHECK(IDC_CHECKBOX_TIMELAPSE_OPTIONS_CREATE_MOVIE, m_optionsCreateMovie)
      DDX_CONTROL_HANDLE(IDC_BUTTON_TIMELAPSE_OPTIONS_MOVIE_CONFIGURE, m_buttonMovieConfigure)
      DDX_CHECK(IDC_CHECKBOX_TIMELAPSE_OPTIONS_HDRMODE, m_optionsUseHDR)
      DDX_CONTROL_HANDLE(IDC_COMBO_AEB_BRACKET_SHOTS, m_comboAEBBracketedShots)
      DDX_CONTROL_HANDLE(IDC_LIST_AEB_SHUTTER_SPEED_VALUES, m_listAEBShutterSpeedValues)
      DDX_CONTROL(IDC_COMBO_SHUTTER_SPEED, m_comboShutterSpeed)
   END_DDX_MAP()

   // message map
   BEGIN_MSG_MAP(TimeLapsePhotoModeView)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      COMMAND_ID_HANDLER(IDC_BUTTON_TIMELAPSE_START, OnButtonStart)
      COMMAND_ID_HANDLER(IDC_BUTTON_TIMELAPSE_STOP, OnButtonStop)
      COMMAND_ID_HANDLER(ID_CAMERA_RELEASE, OnButtonCameraRelease)
      COMMAND_ID_HANDLER(IDC_CHECKBOX_TIMELAPSE_SCHEDULE_STARTTIME, OnCheckboxScheduleStartTime)
      COMMAND_ID_HANDLER(IDC_CHECKBOX_TIMELAPSE_SCHEDULE_ENDTIME, OnCheckboxScheduleEndTime)
      COMMAND_ID_HANDLER(IDC_CHECKBOX_TIMELAPSE_OPTIONS_CREATE_MOVIE, OnCheckboxOptionsCreateMovie)
      COMMAND_ID_HANDLER(IDC_BUTTON_TIMELAPSE_OPTIONS_MOVIE_CONFIGURE, OnButtonOptionsMovieConfigure)
      COMMAND_ID_HANDLER(IDC_CHECKBOX_TIMELAPSE_OPTIONS_HDRMODE, OnCheckboxOptionsHDRMode)
      COMMAND_HANDLER(IDC_COMBO_SHUTTER_SPEED, CBN_SELCHANGE, OnComboShutterSpeedSelChange)
      COMMAND_HANDLER(IDC_COMBO_AEB_BRACKET_SHOTS, CBN_SELCHANGE, OnComboAEBBracketShotsSelChange)
      REFLECT_NOTIFICATIONS() // to make sure superclassed controls get notification messages
   END_MSG_MAP()

   // Handler prototypes (uncomment arguments if needed):
   // LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   // LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
   // LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled)

   /// called when dialog is being shown
   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called at destruction of dialog
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   /// called when button "Start" is pressed
   LRESULT OnButtonStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when button "Stop" is pressed
   LRESULT OnButtonStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when button "Release" is pressed; command from the ribbon, used for "manual trigger" event
   LRESULT OnButtonCameraRelease(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when checkbox "start time" has been clicked
   LRESULT OnCheckboxScheduleStartTime(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when checkbox "end time" has been clicked
   LRESULT OnCheckboxScheduleEndTime(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when checkbox "Create movie" has been clicked
   LRESULT OnCheckboxOptionsCreateMovie(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when button "Configure" has been clicked
   LRESULT OnButtonOptionsMovieConfigure(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when checkbox "HDR mode" has been clicked
   LRESULT OnCheckboxOptionsHDRMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when shutter speed selection in combobox changes
   LRESULT OnComboShutterSpeedSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// called when selected number of AEB bracket shots changes
   LRESULT OnComboAEBBracketShotsSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

   /// enables or disables controls
   void EnableControls(bool enable);

   /// called when property has changed
   void OnUpdatedProperty(RemoteReleaseControl::T_enPropertyEvent propertyEvent, unsigned int value);

   /// updates shutter speed list
   void UpdateAEBShutterSpeedList();

   /// called by photo mode manager when time lapse is finished
   void OnTimeLapseFinished();

private:
   /// host access
   IPhotoModeViewHost& m_host;

   // model

   /// source device
   std::shared_ptr<SourceDevice> m_spSourceDevice;

   /// remote release control
   std::shared_ptr<RemoteReleaseControl> m_spRemoteReleaseControl;

   /// manager for panorama photo mode
   TimeLapsePhotoModeManager m_manager;

   /// indicates release trigger radio button selection
   int m_releaseTriggerRadio;

   /// schedule:indicates if start time option is checked
   bool m_scheduleIsCheckedStartTime;

   /// schedule:indicates if end time option is checked
   bool m_scheduleIsCheckedEndTime;

   /// options: indicates if a movie should be created at end of timelapse shooting
   bool m_optionsCreateMovie;

   /// options: indicates if HDR mode should be used
   bool m_optionsUseHDR;

   // UI

   /// button to start time lapse shooting
   CButton m_btnStart;

   /// button to stop time lapse shooting
   CButton m_btnStop;

   /// release trigger: time picker for interval
   CDateTimePickerCtrl m_timePickerReleaseTriggerInterval;

   /// schedule: picker for start date
   CDateTimePickerCtrl m_timePickerScheduleStartDate;

   /// schedule: picker for start time
   CDateTimePickerCtrl m_timePickerScheduleStartTime;

   /// schedule: picker for end date
   CDateTimePickerCtrl m_timePickerScheduleEndDate;

   /// schedule: picker for end time
   CDateTimePickerCtrl m_timePickerScheduleEndTime;

   /// button to configure movie options
   CButton m_buttonMovieConfigure;

   /// shutter speed combobox
   ImagePropertyCombobox m_comboShutterSpeed;

   /// combobox with number of bracket shots
   CComboBox m_comboAEBBracketedShots;

   /// list of AEB shutter speed values
   CListViewCtrl m_listAEBShutterSpeedValues;

   /// handler ID for property callback
   int m_propertyHandlerId;
};
