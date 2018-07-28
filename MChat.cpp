#include "MChat_Core\My_Library\LOG.hpp"

#include "MChat_Core\MChat_Base.cpp"

#include <sstream>
#include <string>

extern bool DEBUG;

using namespace std;

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

enum{
  MChat_STOP = 1,
  MChat_START = 2
};

class MChat_Frame : public wxFrame{
private:
  wxButton *quit, *stop, *start;
  MChat_Base m_master;
public:
  MChat_Frame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)){
    wxPanel *panel = new wxPanel(this, wxID_ANY);

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);

    quit = new wxButton(panel, wxID_EXIT, wxT("Quit"), wxPoint(700, 500));
    stop = new wxButton(panel, MChat_STOP, wxT("Stop"), wxPoint(-1, -1));
    start = new wxButton(panel, MChat_START, wxT("Start"), wxPoint(-1, -1));

    Connect(wxID_EXIT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MChat_Frame::OnQuit));
    Connect(MChat_STOP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MChat_Frame::OnStop));
    Connect(MChat_START, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MChat_Frame::OnStart));

    hbox->Add(new wxPanel(panel, wxID_ANY));
    vbox->Add(hbox, 1, wxEXPAND);

    hbox2->Add(start);
    hbox2->Add(stop);
    hbox2->Add(quit);

    vbox->Add(hbox2, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);
    panel->SetSizer(vbox);

    stop->Enable(false);

    Centre();

    // initializes the MChat_Base object
    m_master = MChat_Base();
  }

  void OnQuit(wxCommandEvent& WXUNUSED(event)){
    Close(true);
  }

  void OnStop(wxCommandEvent& WXUNUSED(event)){
    quit->Enable(true);
    start->Enable(true);
    stop->Enable(false);

    m_master.stop();
  }

  void OnStart(wxCommandEvent& WXUNUSED(event)){
    start->Enable(false);

    m_master.start();

    stop->Enable(true);
    quit->Enable(false);
  }

};

class MChat_App : public wxApp{
public:
  bool OnInit(){
    MChat_Frame *mchat = new MChat_Frame(wxT("MChat"));
    mchat->Show(true);
    DEBUG = true;

    return true;
  }
};

IMPLEMENT_APP(MChat_App);
