#ifndef MAINFRAME_H
#define MAINFRAME_H
#include "wxcrafter.h"

class clTreeCtrl;
class MyItemData : public wxTreeItemData
{
    wxString m_path;

public:
    MyItemData(const wxString& path)
        : m_path(path)
    {
    }
    virtual ~MyItemData() {}
    void SetPath(const wxString& path) { this->m_path = path; }
    const wxString& GetPath() const { return m_path; }
};

class MainFrame : public MainFrameBaseClass
{
    wxString m_path;
    clTreeCtrl* m_tree = nullptr;

private:
    void LogMessage(const wxString& message);

protected:
    virtual void OnCollapseAll(wxCommandEvent& event);
    virtual void OnExpandAll(wxCommandEvent& event);
    virtual void OnOpenFolder(wxCommandEvent& event);
    void OnItemExpanding(wxTreeEvent& event);

public:
    MainFrame(wxWindow* parent);
    virtual ~MainFrame();

    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    // protected:
    //    virtual void OnScroll(wxCommandEvent& event);
};
#endif // MAINFRAME_H
