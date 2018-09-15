#ifndef MAINFRAME_H
#define MAINFRAME_H
#include "clRowEntry.h"
#include "wxcrafter.h"
#include <array>

class clTreeCtrl;
class MyItemData : public wxTreeItemData
{
    wxString m_path;
    wxString m_fullname;
    wxString m_fullnameLC;
    bool m_isFolder;

public:
    MyItemData(const wxString& path, bool isFolder)
        : m_isFolder(isFolder)
    {
        wxFileName fn(path);
        m_path = fn.GetFullPath();
        m_fullname = fn.GetFullName();
        m_fullnameLC = m_fullname.Lower();
    }

    virtual ~MyItemData() {}
    void SetPath(const wxString& path) { this->m_path = path; }
    const wxString& GetPath() const { return m_path; }
    void SetFullname(const wxString& fullname) { this->m_fullname = fullname; }
    const wxString& GetFullname() const { return m_fullname; }
    void SetIsFolder(bool isFolder) { this->m_isFolder = isFolder; }
    bool IsFolder() const { return m_isFolder; }
    const wxString& GetFullnameLC() const { return m_fullnameLC; }
};

class MainFrame : public MainFrameBaseClass
{
    wxString m_path;
    std::array<clColours, 2> m_coloursArr;
    int m_selectedColours = 0;

private:
    void LogMessage(const wxString& message);
    void DoAddRoot();

protected:
    virtual void OnDeleteAllItems(wxCommandEvent& event);
    virtual void OnHideHeaders(wxCommandEvent& event);
    virtual void OnShowSBOnFocus(wxCommandEvent& event);
    virtual void OnSingleSelection(wxCommandEvent& event);
    virtual void OnHideRoot(wxCommandEvent& event);
    virtual void OnToggleTheme(wxCommandEvent& event);
    virtual void OnZebraColouring(wxCommandEvent& event);
    virtual void OnNextSibling(wxCommandEvent& event);
    virtual void OnPrevSibling(wxCommandEvent& event);
    virtual void OnSelectChildren(wxCommandEvent& event);
    virtual void OnEnsureItemVisible(wxCommandEvent& event);
    virtual void OnFirstVisible(wxCommandEvent& event);
    virtual void OnNextVisible(wxCommandEvent& event);
    virtual void OnCollapseAll(wxCommandEvent& event);
    virtual void OnExpandAll(wxCommandEvent& event);
    virtual void OnOpenFolder(wxCommandEvent& event);
    void OnItemExpanding(wxTreeEvent& event);
    void OnItemDeleted(wxTreeEvent& event);

public:
    MainFrame(wxWindow* parent);
    virtual ~MainFrame();

    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    // protected:
    //    virtual void OnScroll(wxCommandEvent& event);
};
#endif // MAINFRAME_H
