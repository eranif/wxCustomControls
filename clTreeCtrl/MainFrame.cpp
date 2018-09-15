#include "MainFrame.h"
#include "clTreeCtrl.h"
#include <cmath>
#include <wx/aboutdlg.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/utils.h>

MainFrame::MainFrame(wxWindow* parent)
    : MainFrameBaseClass(parent)
{
    clColours colours;
    colours.InitDefaults();
    m_coloursArr[0] = colours;

    colours.InitDarkDefaults();
    m_coloursArr[1] = colours;

    m_treeCtrl->SetColours(m_coloursArr[m_selectedColours]);

    clHeaderBar header;
    header.Add("Path");
    header.Add("Kind");
    header.Add("Size");
    m_treeCtrl->SetHeader(header);
    DoAddRoot();

    wxLog::SetActiveTarget(new wxLogTextCtrl(m_textCtrlLog));
    // Provide a sorting function to the tree
    std::function<bool(const wxTreeItemId& a, const wxTreeItemId& b)> SortFunc = [&](const wxTreeItemId& a,
                                                                                     const wxTreeItemId& b) {
        // Compare based on item type and then by label
        MyItemData* cd_a = dynamic_cast<MyItemData*>(m_treeCtrl->GetItemData(a));
        MyItemData* cd_b = dynamic_cast<MyItemData*>(m_treeCtrl->GetItemData(b));
        if(cd_a->IsFolder() && !cd_b->IsFolder()) {
            return true;
        } else if(!cd_a->IsFolder() && cd_b->IsFolder()) {
            return false;
        } else {
            return cd_a->GetFullnameLC() < cd_b->GetFullnameLC();
        }
    };
    m_treeCtrl->SetSortFunction(SortFunc);
    std::vector<wxBitmap> bitmaps;
    MyImages images;
    bitmaps.push_back(images.Bitmap("folder"));
    bitmaps.push_back(images.Bitmap("folder_open"));
    bitmaps.push_back(images.Bitmap("file"));
    m_treeCtrl->SetBitmaps(bitmaps);

    m_treeCtrl->Bind(wxEVT_TREE_ITEM_EXPANDING, &MainFrame::OnItemExpanding, this);
    m_treeCtrl->Bind(wxEVT_TREE_DELETE_ITEM, &MainFrame::OnItemDeleted, this);
    m_treeCtrl->Bind(wxEVT_TREE_BEGIN_DRAG, [&](wxTreeEvent& evt) { LogMessage(wxString() << "Drag started"); });
    m_treeCtrl->Bind(wxEVT_TREE_END_DRAG, [&](wxTreeEvent& evt) {
        LogMessage(wxString() << "Drag ended. Drop is on item: " << m_treeCtrl->GetItemText(evt.GetItem()));
    });

    m_treeCtrl->Bind(wxEVT_TREE_ITEM_EXPANDED, [&](wxTreeEvent& evt) {
        LogMessage(wxString() << m_treeCtrl->GetItemText(evt.GetItem()) << " expanded");
    });
    m_treeCtrl->Bind(wxEVT_TREE_ITEM_COLLAPSING, [&](wxTreeEvent& evt) {
        LogMessage(wxString() << m_treeCtrl->GetItemText(evt.GetItem()) << " is collapsing");
    });
    m_treeCtrl->Bind(wxEVT_TREE_ITEM_COLLAPSED, [&](wxTreeEvent& evt) {
        LogMessage(wxString() << m_treeCtrl->GetItemText(evt.GetItem()) << " collapsed");
    });
    m_treeCtrl->Bind(wxEVT_TREE_SEL_CHANGING, [&](wxTreeEvent& evt) {
        evt.Skip();
        LogMessage(wxString() << "Selection changing from: " << m_treeCtrl->GetItemText(evt.GetOldItem()));
    });
    m_treeCtrl->Bind(wxEVT_TREE_SEL_CHANGED, [&](wxTreeEvent& evt) {
        evt.Skip();
        LogMessage(wxString() << "Selection changed. selection is: " << m_treeCtrl->GetItemText(evt.GetItem()));
    });
    m_treeCtrl->Bind(wxEVT_TREE_KEY_DOWN, [&](wxTreeEvent& evt) {
        evt.Skip();
        LogMessage(wxString() << "Key down. selection is: " << m_treeCtrl->GetItemText(evt.GetItem()));
    });

    m_treeCtrl->Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, [&](wxTreeEvent& evt) {
        evt.Skip(); // Must call this for the default actions process
        LogMessage(wxString() << "Tree right click: " << m_treeCtrl->GetItemText(evt.GetItem()));
    });

    m_treeCtrl->Bind(wxEVT_TREE_ITEM_ACTIVATED, [&](wxTreeEvent& evt) {
        evt.Skip();
        wxArrayTreeItemIds items;
        m_treeCtrl->GetSelections(items);
        for(size_t i = 0; i < items.GetCount(); ++i) {
            LogMessage(wxString() << "Acticated item: " << m_treeCtrl->GetItemText(items[i]));
        }
    });

    m_treeCtrl->Bind(wxEVT_TREE_ITEM_MENU, [&](wxTreeEvent& evt) {
        evt.Skip(); // Let the default action take place
        LogMessage(wxString() << "Context menu for item: " << m_treeCtrl->GetItemText(evt.GetItem()));
        wxMenu menu;
        menu.Append(wxID_OPEN);
        menu.Bind(wxEVT_MENU,
                  [&](wxCommandEvent& e) {
                      wxArrayTreeItemIds items;
                      if(m_treeCtrl->GetSelections(items)) {
                          for(size_t i = 0; i < items.size(); ++i) {
                              MyItemData* cd = dynamic_cast<MyItemData*>(m_treeCtrl->GetItemData(items[i]));
                              if(cd) { ::wxLaunchDefaultApplication(cd->GetPath()); }
                          }
                      }
                  },
                  wxID_OPEN);
        m_treeCtrl->PopupMenu(&menu);
    });
}

MainFrame::~MainFrame() {}

void MainFrame::OnExit(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_treeCtrl->DeleteAllItems();
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

void MainFrame::LogMessage(const wxString& message)
{
    static int counter = 0;
    wxLogMessage(wxString() << (++counter) << ": " << message);
}

void MainFrame::OnOpenFolder(wxCommandEvent& event)
{
    wxString path = wxDirSelector();
    if(path.IsEmpty()) { return; }

    m_path = path;
    wxTreeItemId item = m_treeCtrl->AppendItem(m_treeCtrl->GetRootItem(), path, 0, 1, new MyItemData(m_path, true));
    m_treeCtrl->SetItemText(item, "Folder", 1);
    m_treeCtrl->SetItemText(item, "0KB", 2);
    m_treeCtrl->AppendItem(item, "dummy-node");
    m_treeCtrl->SelectItem(item);
    m_treeCtrl->CallAfter(&clTreeCtrl::SetFocus);
}

void MainFrame::OnItemExpanding(wxTreeEvent& event)
{
    event.Skip();
    wxTreeItemId item = event.GetItem();
    wxString text = m_treeCtrl->GetItemText(item);
    LogMessage(wxString() << text << " is expanding");

    wxTreeItemIdValue cookie;
    if(m_treeCtrl->ItemHasChildren(item)) {
        wxTreeItemId child = m_treeCtrl->GetFirstChild(item, cookie);
        if(m_treeCtrl->GetItemText(child) == "dummy-node") {
            m_treeCtrl->DeleteChildren(item);
            // load the folders
            MyItemData* cd = dynamic_cast<MyItemData*>(m_treeCtrl->GetItemData(item));
            wxString filename;
            wxDir dir(cd->GetPath());
            if(dir.IsOpened()) {
                bool cont = dir.GetFirst(&filename);
                while(cont) {
                    wxFileName fn(cd->GetPath(), filename);
                    if(wxDirExists(fn.GetFullPath())) {
                        // A directory
                        wxTreeItemId folderItem =
                            m_treeCtrl->AppendItem(item, filename, 0, 1, new MyItemData(fn.GetFullPath(), true));
                        m_treeCtrl->SetItemText(folderItem, "Folder", 1);
                        m_treeCtrl->SetItemText(folderItem, "0KB", 2);
                        m_treeCtrl->AppendItem(folderItem, "dummy-node");
                    } else {
                        // A file
                        wxTreeItemId fileItem =
                            m_treeCtrl->AppendItem(item, filename, 2, 2, new MyItemData(fn.GetFullPath(), false));
                        m_treeCtrl->SetItemText(fileItem, "File", 1);
                        wxString t;
                        t << std::ceil((double)fn.GetSize().ToDouble() / 1024.0) << "KB";
                        m_treeCtrl->SetItemText(fileItem, t, 2);
                    }
                    cont = dir.GetNext(&filename);
                }
            }
        }
    }
}

void MainFrame::OnExpandAll(wxCommandEvent& event) { m_treeCtrl->ExpandAll(); }
void MainFrame::OnCollapseAll(wxCommandEvent& event) { m_treeCtrl->CollapseAll(); }
void MainFrame::OnFirstVisible(wxCommandEvent& event)
{
    wxTreeItemId item = m_treeCtrl->GetFirstVisibleItem();
    m_treeCtrl->SelectItem(item);
}

void MainFrame::OnNextVisible(wxCommandEvent& event)
{
    wxTreeItemId item = m_treeCtrl->GetNextVisible(m_treeCtrl->GetFocusedItem());
    m_treeCtrl->SelectItem(item);
}
void MainFrame::OnEnsureItemVisible(wxCommandEvent& event)
{
    // long index = wxGetNumberFromUser("Item index:", "Item index:", "clTreeCtrl");
    // wxTreeItemId item = m_treeCtrl->RowToItem(index);
    // if(item.IsOk()) { wxMessageBox("Item is: " + m_treeCtrl->GetItemText(item)); }
    // m_treeCtrl->SelectItem(item);
    // m_treeCtrl->EnsureVisible(item);
}

void MainFrame::OnItemDeleted(wxTreeEvent& event)
{
    wxString text = m_treeCtrl->GetItemText(event.GetItem());
    LogMessage("Item: " + text + " was deleted");
}

void MainFrame::OnSelectChildren(wxCommandEvent& event) { m_treeCtrl->SelectChildren(m_treeCtrl->GetFocusedItem()); }
void MainFrame::OnNextSibling(wxCommandEvent& event)
{
    m_treeCtrl->SelectItem(m_treeCtrl->GetNextSibling(m_treeCtrl->GetFocusedItem()));
    m_treeCtrl->EnsureVisible(m_treeCtrl->GetFocusedItem());
}

void MainFrame::OnPrevSibling(wxCommandEvent& event)
{
    m_treeCtrl->SelectItem(m_treeCtrl->GetPrevSibling(m_treeCtrl->GetFocusedItem()));
    m_treeCtrl->EnsureVisible(m_treeCtrl->GetFocusedItem());
}
void MainFrame::OnToggleTheme(wxCommandEvent& event)
{
    if(m_selectedColours == 0) {
        m_treeCtrl->SetColours(m_coloursArr[1]);
        m_selectedColours = 1;
    } else {
        m_treeCtrl->SetColours(m_coloursArr[0]);
        m_selectedColours = 0;
    }
    m_treeCtrl->Refresh();
}

void MainFrame::OnZebraColouring(wxCommandEvent& event)
{
    m_treeCtrl->EnableStyle(wxTR_ROW_LINES, !m_treeCtrl->HasStyle(wxTR_ROW_LINES));
}
void MainFrame::OnHideRoot(wxCommandEvent& event) { m_treeCtrl->EnableStyle(wxTR_HIDE_ROOT, event.IsChecked()); }

void MainFrame::OnSingleSelection(wxCommandEvent& event) { m_treeCtrl->EnableStyle(wxTR_MULTIPLE, !event.IsChecked()); }
void MainFrame::OnShowSBOnFocus(wxCommandEvent& event) { m_treeCtrl->SetShowScrollBarOnFocus(event.IsChecked()); }
void MainFrame::OnHideHeaders(wxCommandEvent& event) { m_treeCtrl->SetShowHeader(!event.IsChecked()); }
void MainFrame::OnDeleteAllItems(wxCommandEvent& event)
{
    m_treeCtrl->DeleteAllItems();
    DoAddRoot();
}

void MainFrame::DoAddRoot()
{
    wxTreeItemId root = m_treeCtrl->AddRoot("Root", -1, -1, nullptr);
    m_treeCtrl->SetItemText(root, "??", 1);
    m_treeCtrl->SetItemText(root, "0KB", 2);
}
