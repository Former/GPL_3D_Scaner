#include "main.h"
#include <vector>

IMPLEMENT_APP(MainApp);

bool MainApp::OnInit()
{
	SetTopWindow( new MainDialog( NULL ) );
	GetTopWindow()->Show();
	
	wxInitAllImageHandlers();
	
	// true = enter the main loop
	return true;
}

MainDialog::MainDialog(wxWindow *parent) : MainDialogBase(parent)
{
	m_Device = new CameraDeviceV4L2();
	m_Device->OpenDevice("/dev/video0");
	
	std::vector<CameraDeviceV4L2::PixelFormat> formats = m_Device->GetPixelFormats();
	m_Device->SetFormat(640, 480, formats[0].FormatID);
}

MainDialog::~MainDialog()
{
	delete m_Device;	
}

void MainDialog::OnCloseDialog(wxCloseEvent& event)
{
	Destroy();
}

void MainDialog::OnOKClick(wxCommandEvent& event)
{
	unsigned char* buf = (unsigned char*)malloc(640 * 480 * 3);
	bool result = m_Device->GetFrame(buf);
	
	if (!result)
		return;
	
	
	wxImage wximg(640, 480);
	wximg.SetData(buf);
	
	wxPaintDC dc(m_DrawWindow);
	
	dc.DrawBitmap(wximg, wxPoint(0, 0));
}

void MainDialog::OnCancelClick(wxCommandEvent& event)
{
	Destroy();
}

