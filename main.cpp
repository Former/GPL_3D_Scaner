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

MainDialog::MainDialog(wxWindow *parent) : MainDialogBase( parent )
{
	m_Device = new CameraDeviceV4L2();
	m_Device->openDevice(640, 480, -1);
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
	std::vector<unsigned char> buffer = m_Device->getFrame();
	
	if (buffer.empty())
		return;
	
	unsigned char* buf = (unsigned char*)malloc(buffer.size());
	memcpy(buf, &buffer[0], buffer.size());
	
	wxImage wximg(640, 480);
	wximg.SetData(buf);
	
	wxPaintDC dc(m_DrawWindow);
	
	dc.DrawBitmap(wximg, wxPoint(0, 0));
}

void MainDialog::OnCancelClick(wxCommandEvent& event)
{
	Destroy();
}

