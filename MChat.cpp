#include "MChat_Core\My_Library\LOG.hpp"

#include "MChat_Core\MChat_Base.cpp"

#include <sstream>
#include <string>

extern bool DEBUG;

#define MAX_MH 20

using namespace std;

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

enum { MChat_STOP = 1, MChat_START = 2 };

class MChat_Frame : public wxFrame {
private:
  wxButton *quit, *stop, *start;
  wxGauge *progress_bars[MAX_MH];
  MChat_Base m_master;
  int MH_size;

public:
  MChat_Frame(const wxString &title)
      : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)) {
    wxPanel *panel = new wxPanel(this, wxID_ANY);

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);

    quit = new wxButton(panel, wxID_EXIT, wxT("Quit"), wxPoint(700, 500));
    stop = new wxButton(panel, MChat_STOP, wxT("Stop"), wxPoint(-1, -1));
    start = new wxButton(panel, MChat_START, wxT("Start"), wxPoint(-1, -1));

    Connect(wxID_EXIT, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MChat_Frame::OnQuit));
    Connect(MChat_STOP, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MChat_Frame::OnStop));
    Connect(MChat_START, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MChat_Frame::OnStart));

    hbox->Add(new wxPanel(panel, wxID_ANY));
    vbox->Add(hbox, 1, wxEXPAND);
    for (int i = 0; i < MAX_MH; i++) {
      progress_bars[i] = new wxGauge();
      vbox->Add(progress_bars[i]);
      progress_bars[i]->Show(false);
    }

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

  void OnQuit(wxCommandEvent &WXUNUSED(event)) { Close(true); }

  void OnStop(wxCommandEvent &WXUNUSED(event)) {
    stop->Enable(false);

    m_master.stop();

    quit->Enable(true);
    start->Enable(true);
  }

  void OnStart(wxCommandEvent &WXUNUSED(event)) {
    start->Enable(false);

    m_master.start();
    m_master.wait_until_ready();
    MH_size = m_master.get_MH_count();
    if (MAX_MH < MH_size)
      exit(1);
    for (int i = 0; i < MH_size; i++) {
      progress_bars[i]->SetRange(100);
      progress_bars[i]->SetValue(0);
      progress_bars[i]->Show(true);
      progress_bars[i]->Pulse();
      wxTheApp->Yield();
    }

    stop->Enable(true);
    quit->Enable(false);
  }
};

class MChat_App : public wxApp {
public:
  bool OnInit() {
    MChat_Frame *mchat = new MChat_Frame(wxT("MChat"));
    mchat->Show(true);
    DEBUG = true;

    return true;
  }
};

IMPLEMENT_APP(MChat_App);
DECLARE_APP(MChat_App);
