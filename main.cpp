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

MainDialog::MainDialog(wxWindow *parent) 
: MainDialogBase(parent), m_Collibrator(1.0, 1.0, 20.0), m_Parser(&m_Collibrator)
{
	m_Device = new CameraDeviceV4L2(1);
	m_Device->OpenDevice("/dev/video0");
	
	std::vector<CameraDeviceV4L2::PixelFormat> formats = m_Device->GetPixelFormats();
	const int pixelFormat = formats[0].FormatID;

	std::vector<CameraDeviceV4L2::Resolution> resolutions = m_Device->GetResolutions(pixelFormat);
	m_CurCamResolution = resolutions[0];
	m_Collibrator.Initialize(m_CurCamResolution.Width, m_CurCamResolution.Height); 

	m_Device->SetFormat(m_CurCamResolution.Width, m_CurCamResolution.Height, pixelFormat);
	
	Connect(MAIN_TIMER_ID, wxEVT_TIMER, wxTimerEventHandler(MainDialog::OnTimer));
	
	m_Timer = new wxTimer(this, MAIN_TIMER_ID);
	m_Timer->Start(1000);
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
}

void MainDialog::OnCancelClick(wxCommandEvent& event)
{
	Destroy();
}

void MainDialog::OnTimer(wxTimerEvent& event)
{
	unsigned char* buf = (unsigned char*)malloc(m_CurCamResolution.Width * m_CurCamResolution.Height * 3);
	bool result = m_Device->GetFrame(buf);
	
	if (!result)
		return;
	
	const Parser3D::RGB mark(0, 255, 0);
	std::vector<Parser3D::ImageParser::PixelLine> lines = m_Parser.Prepare((Parser3D::RGB*)buf);
	for (size_t i = 0; i < lines.size(); i++)
	{
		std::vector<double> line = m_Parser.PapseLine(lines[i]);
		for (size_t j = 0; j < line.size(); j++)
			*((Parser3D::RGB*)buf + i + (size_t)line[j] * lines.size()) = mark;
	}
	
	std::vector<Point3D> points = m_Parser.Parse((Parser3D::RGB*)buf);
	
	wxImage wximg(m_CurCamResolution.Width, m_CurCamResolution.Height);
	wximg.SetData(buf);
	
	wxPaintDC dc(m_DrawWindow);
	
	dc.DrawBitmap(wximg, wxPoint(0, 0));
}
