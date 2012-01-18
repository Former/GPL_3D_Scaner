#include "main.h"
#include <vector>

IMPLEMENT_APP(MainApp);

bool MainApp::OnInit()
{
	wxInitAllImageHandlers();

	SetTopWindow(new MainDialog(NULL));
	GetTopWindow()->Show();
	
	return true;
}

MainDialog::MainDialog(wxWindow *parent) : MainDialogBase(parent)
{
	m_Device = new CameraDeviceV4L2();
	m_Device->OpenDevice("/dev/video0");
	
	std::vector<CameraDeviceV4L2::PixelFormat> formats = m_Device->GetPixelFormats();
	const int pixelFormat = formats[0].FormatID;

	std::vector<CameraDeviceV4L2::Resolution> resolutions = m_Device->GetResolutions(pixelFormat);
	m_CurCamResolution = resolutions[0];

	m_Device->SetFormat(m_CurCamResolution.Width, m_CurCamResolution.Height, pixelFormat);
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
	unsigned char* buf = (unsigned char*)malloc(m_CurCamResolution.Width * m_CurCamResolution.Height * 3);
	bool result = m_Device->GetFrame(buf);
	
	if (!result)
		return;
	
	wxImage wximg(m_CurCamResolution.Width, m_CurCamResolution.Height);
	wximg.SetData(buf);
	
	wxPaintDC dc(m_DrawWindow);
	
	dc.DrawBitmap(wximg, wxPoint(0, 0));
}

void MainDialog::OnCancelClick(wxCommandEvent& event)
{
	Destroy();
}

