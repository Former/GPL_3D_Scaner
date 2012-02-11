#pragma once

#include <wx/wx.h>
#include "gui.h"
#include "CameraDeviceV4L2.h"
#include "Parser3D.h"

class MainApp : public wxApp
{
	public:
		virtual bool OnInit();
};

DECLARE_APP(MainApp)

#define MAIN_TIMER_ID		100

class MainDialog : public MainDialogBase
{
public:
	MainDialog( wxWindow *parent );
	virtual ~MainDialog();
	
protected:
	virtual void OnCloseDialog( wxCloseEvent& event );
	virtual void OnOKClick( wxCommandEvent& event );
	virtual void OnCancelClick( wxCommandEvent& event );
	virtual void OnTimer(wxTimerEvent& event);
	
	CameraDeviceV4L2* m_Device;
	CameraDeviceV4L2::Resolution m_CurCamResolution;
	wxTimer* m_Timer;
	Parser3D::ImageParser m_Parser;
	Parser3D::CameraCollibrator m_Collibrator;
};