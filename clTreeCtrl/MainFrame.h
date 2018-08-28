#ifndef MAINFRAME_H
#define MAINFRAME_H
#include "wxcrafter.h"

class MainFrame : public MainFrameBaseClass
{
protected:
    void LogMessage(const wxString& message);

public:
    MainFrame(wxWindow* parent);
    virtual ~MainFrame();

    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

protected:
    virtual void OnScroll(wxCommandEvent& event);
};
#endif // MAINFRAME_H
