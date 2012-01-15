#include "main.h"

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
	m_Device = new Device();
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
	char* buffer = 0;
	m_Device->getFrame(&buffer);
	wxImage wximg(640, 480);
	wximg.SetData((unsigned char*)buffer);
	
	wxPaintDC dc(m_DrawWindow);
	
	dc.DrawBitmap(wximg, wxPoint(0, 0));
}

void MainDialog::OnCancelClick(wxCommandEvent& event)
{
	Destroy();
}

