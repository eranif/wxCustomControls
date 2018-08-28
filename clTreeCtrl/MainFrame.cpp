#include "MainFrame.h"
#include "clTreeCtrl.h"
#include <wx/aboutdlg.h>

static clTreeCtrl* ctrl = nullptr;
MainFrame::MainFrame(wxWindow* parent)
    : MainFrameBaseClass(parent)
{
    ctrl = new clTreeCtrl(m_mainPanel);
    m_mainPanel->GetSizer()->Insert(0, ctrl, 1, wxEXPAND);
    wxTreeItemId root = ctrl->AddRoot("Root Item");
    wxTreeItemId p = root;
    wxTreeItemId showMe;
    for(size_t i = 0; i < 1000; ++i) {
        wxTreeItemId child = ctrl->AppendItem(p, wxString() << "Child Item " << i);
        if(i == 51) { showMe = child; }
    }

    ctrl->Bind(wxEVT_TREE_ITEM_EXPANDING, [&](wxTreeEvent& evt) {
        clTreeCtrlNode* node = reinterpret_cast<clTreeCtrlNode*>(evt.GetItem().GetID());
        LogMessage(wxString() << node->GetLabel() << " is expanding");
    });
    ctrl->Bind(wxEVT_TREE_ITEM_EXPANDED, [&](wxTreeEvent& evt) {
        clTreeCtrlNode* node = reinterpret_cast<clTreeCtrlNode*>(evt.GetItem().GetID());
        LogMessage(wxString() << node->GetLabel() << " expanded");
    });
    ctrl->Bind(wxEVT_TREE_ITEM_COLLAPSING, [&](wxTreeEvent& evt) {
        clTreeCtrlNode* node = reinterpret_cast<clTreeCtrlNode*>(evt.GetItem().GetID());
        LogMessage(wxString() << node->GetLabel() << " is collapsing");
    });
    ctrl->Bind(wxEVT_TREE_ITEM_COLLAPSED, [&](wxTreeEvent& evt) {
        clTreeCtrlNode* node = reinterpret_cast<clTreeCtrlNode*>(evt.GetItem().GetID());
        LogMessage(wxString() << node->GetLabel() << " collapsed");
    });

    ctrl->EnsureVisible(showMe);
    ctrl->SelectItem(showMe);
}

MainFrame::~MainFrame() {}

void MainFrame::OnExit(wxCommandEvent& event)
{
    wxUnusedVar(event);
    Close();
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
    wxUnusedVar(event);
    wxAboutDialogInfo info;
    info.SetCopyright(_("My MainFrame"));
    info.SetLicence(_("GPL v2 or later"));
    info.SetDescription(_("Short description goes here"));
    ::wxAboutBox(info);
}

void MainFrame::OnScroll(wxCommandEvent& event)
{
    // Scroll to the 100th element
    ctrl->Scroll(0, 100);
}

void MainFrame::LogMessage(const wxString& message)
{
    m_stc15->AddText(message + "\n");
    m_stc15->ScrollToEnd();
}
