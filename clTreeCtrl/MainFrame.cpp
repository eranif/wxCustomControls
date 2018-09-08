#include "MainFrame.h"
#include "clTreeCtrl.h"
#include <wx/aboutdlg.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>

MainFrame::MainFrame(wxWindow* parent)
    : MainFrameBaseClass(parent)
{
    m_tree = new clTreeCtrl(m_mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_MULTIPLE);
    clColours colours;
    colours.InitDefaults();
    m_coloursArr[0] = colours;

    colours.InitDarkDefaults();
    m_coloursArr[1] = colours;

    m_tree->SetColours(m_coloursArr[m_selectedColours]);

    clHeaderBar header;
    header.Add("Path");
    header.Add("Kind");
    m_tree->SetHeader(header);

    wxTreeItemId root = m_tree->AddRoot("Root", -1, -1, nullptr);
    m_tree->SetItemText(root, "??", 1);
    
    wxLog::SetActiveTarget(new wxLogTextCtrl(m_textCtrlLog));
    // Provide a sorting function to the tree
    std::function<bool(const wxTreeItemId& a, const wxTreeItemId& b)> SortFunc
        = [&](const wxTreeItemId& a, const wxTreeItemId& b) {
              // Compare based on item type and then by label
              MyItemData* cd_a = dynamic_cast<MyItemData*>(m_tree->GetItemData(a));
              MyItemData* cd_b = dynamic_cast<MyItemData*>(m_tree->GetItemData(b));
              if(cd_a->IsFolder() && !cd_b->IsFolder()) {
                  return true;
              } else if(!cd_a->IsFolder() && cd_b->IsFolder()) {
                  return false;
              } else {
                  return cd_a->GetFullnameLC() < cd_b->GetFullnameLC();
              }
          };
    m_tree->SetSortFunction(SortFunc);
    std::vector<wxBitmap> bitmaps;
    MyImages images;
    bitmaps.push_back(images.Bitmap("folder"));
    bitmaps.push_back(images.Bitmap("folder_open"));
    bitmaps.push_back(images.Bitmap("file"));
    m_tree->SetBitmaps(bitmaps);
    m_mainPanel->GetSizer()->Insert(0, m_tree, 1, wxEXPAND);

    m_tree->Bind(wxEVT_TREE_ITEM_EXPANDING, &MainFrame::OnItemExpanding, this);
    m_tree->Bind(wxEVT_TREE_DELETE_ITEM, &MainFrame::OnItemDeleted, this);
    m_tree->Bind(wxEVT_TREE_BEGIN_DRAG, [&](wxTreeEvent& evt) { LogMessage(wxString() << "Drag started"); });
    m_tree->Bind(wxEVT_TREE_END_DRAG, [&](wxTreeEvent& evt) {
        LogMessage(wxString() << "Drag ended. Drop is on item: " << m_tree->GetItemText(evt.GetItem()));
    });

    m_tree->Bind(wxEVT_TREE_ITEM_EXPANDED,
        [&](wxTreeEvent& evt) { LogMessage(wxString() << m_tree->GetItemText(evt.GetItem()) << " expanded"); });
    m_tree->Bind(wxEVT_TREE_ITEM_COLLAPSING,
        [&](wxTreeEvent& evt) { LogMessage(wxString() << m_tree->GetItemText(evt.GetItem()) << " is collapsing"); });
    m_tree->Bind(wxEVT_TREE_ITEM_COLLAPSED,
        [&](wxTreeEvent& evt) { LogMessage(wxString() << m_tree->GetItemText(evt.GetItem()) << " collapsed"); });
    m_tree->Bind(wxEVT_TREE_SEL_CHANGING, [&](wxTreeEvent& evt) {
        evt.Skip();
        LogMessage(wxString() << "Selection changing from: " << m_tree->GetItemText(evt.GetOldItem()));
    });
    m_tree->Bind(wxEVT_TREE_SEL_CHANGED, [&](wxTreeEvent& evt) {
        evt.Skip();
        LogMessage(wxString() << "Selection changed. selection is: " << m_tree->GetItemText(evt.GetItem()));
    });
    m_tree->Bind(wxEVT_TREE_KEY_DOWN, [&](wxTreeEvent& evt) {
        evt.Skip();
        LogMessage(wxString() << "Key down. selection is: " << m_tree->GetItemText(evt.GetItem()));
    });

    m_tree->Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, [&](wxTreeEvent& evt) {
        evt.Skip(); // Must call this for the default actions process
        LogMessage(wxString() << "Tree right click: " << m_tree->GetItemText(evt.GetItem()));
    });

    m_tree->Bind(wxEVT_TREE_ITEM_ACTIVATED, [&](wxTreeEvent& evt) {
        evt.Skip();
        wxArrayTreeItemIds items;
        m_tree->GetSelections(items);
        for(size_t i = 0; i < items.GetCount(); ++i) {
            LogMessage(wxString() << "Acticated item: " << m_tree->GetItemText(items[i]));
        }
    });

    m_tree->Bind(wxEVT_TREE_ITEM_MENU, [&](wxTreeEvent& evt) {
        evt.Skip(); // Let the default action take place
        LogMessage(wxString() << "Context menu for item: " << m_tree->GetItemText(evt.GetItem()));
        wxMenu menu;
        menu.Append(wxID_OPEN);
        menu.Append(wxID_REFRESH);
        m_tree->PopupMenu(&menu);
    });
}

MainFrame::~MainFrame() {}

void MainFrame::OnExit(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_tree->DeleteAllItems();
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
    wxTreeItemId item = m_tree->AppendItem(m_tree->GetRootItem(), path, 0, 1, new MyItemData(m_path, true));
    m_tree->SetItemText(item, "Folder", 1);
    m_tree->AppendItem(item, "dummy-node");
    m_tree->SelectItem(item);
    m_tree->CallAfter(&clTreeCtrl::SetFocus);
}

void MainFrame::OnItemExpanding(wxTreeEvent& event)
{
    event.Skip();
    wxTreeItemId item = event.GetItem();
    wxString text = m_tree->GetItemText(item);
    LogMessage(wxString() << text << " is expanding");

    wxTreeItemIdValue cookie;
    if(m_tree->ItemHasChildren(item)) {
        wxTreeItemId child = m_tree->GetFirstChild(item, cookie);
        if(m_tree->GetItemText(child) == "dummy-node") {
            m_tree->DeleteChildren(item);
            // load the folders
            MyItemData* cd = dynamic_cast<MyItemData*>(m_tree->GetItemData(item));
            wxString filename;
            wxDir dir(cd->GetPath());
            if(dir.IsOpened()) {
                bool cont = dir.GetFirst(&filename);
                while(cont) {
                    wxFileName fn(cd->GetPath(), filename);
                    if(wxDirExists(fn.GetFullPath())) {
                        // A directory
                        wxTreeItemId folderItem
                            = m_tree->AppendItem(item, filename, 0, 1, new MyItemData(fn.GetFullPath(), true));
                        m_tree->SetItemText(folderItem, "Folder", 1);
                        m_tree->AppendItem(folderItem, "dummy-node");
                    } else {
                        // A file
                        wxTreeItemId fileItem
                            = m_tree->AppendItem(item, filename, 2, 2, new MyItemData(fn.GetFullPath(), false));
                        m_tree->SetItemText(fileItem, "File", 1);
                    }
                    cont = dir.GetNext(&filename);
                }
            }
        }
    }
}

void MainFrame::OnExpandAll(wxCommandEvent& event) { m_tree->ExpandAll(); }
void MainFrame::OnCollapseAll(wxCommandEvent& event) { m_tree->CollapAll(); }
void MainFrame::OnFirstVisible(wxCommandEvent& event)
{
    wxTreeItemId item = m_tree->GetFirstVisibleItem();
    m_tree->SelectItem(item);
}

void MainFrame::OnNextVisible(wxCommandEvent& event)
{
    wxTreeItemId item = m_tree->GetNextVisible(m_tree->GetFocusedItem());
    m_tree->SelectItem(item);
}
void MainFrame::OnEnsureItemVisible(wxCommandEvent& event)
{
    // long index = wxGetNumberFromUser("Item index:", "Item index:", "clTreeCtrl");
    // wxTreeItemId item = m_tree->RowToItem(index);
    // if(item.IsOk()) { wxMessageBox("Item is: " + m_tree->GetItemText(item)); }
    // m_tree->SelectItem(item);
    // m_tree->EnsureVisible(item);
}

void MainFrame::OnItemDeleted(wxTreeEvent& event)
{
    wxString text = m_tree->GetItemText(event.GetItem());
    LogMessage("Item: " + text + " was deleted");
}

void MainFrame::OnSelectChildren(wxCommandEvent& event) { m_tree->SelectChildren(m_tree->GetFocusedItem()); }
void MainFrame::OnNextSibling(wxCommandEvent& event)
{
    m_tree->SelectItem(m_tree->GetNextSibling(m_tree->GetFocusedItem()));
    m_tree->EnsureVisible(m_tree->GetFocusedItem());
}

void MainFrame::OnPrevSibling(wxCommandEvent& event)
{
    m_tree->SelectItem(m_tree->GetPrevSibling(m_tree->GetFocusedItem()));
    m_tree->EnsureVisible(m_tree->GetFocusedItem());
}
void MainFrame::OnToggleTheme(wxCommandEvent& event)
{
    if(m_selectedColours == 0) {
        m_tree->SetColours(m_coloursArr[1]);
        m_selectedColours = 1;
    } else {
        m_tree->SetColours(m_coloursArr[0]);
        m_selectedColours = 0;
    }
    m_tree->Refresh();
}

void MainFrame::OnZebraColouring(wxCommandEvent& event)
{
    m_tree->EnableStyle(wxTR_ROW_LINES, !m_tree->HasStyle(wxTR_ROW_LINES));
}
void MainFrame::OnHideRoot(wxCommandEvent& event) { m_tree->EnableStyle(wxTR_HIDE_ROOT, event.IsChecked()); }

void MainFrame::OnSingleSelection(wxCommandEvent& event) { m_tree->EnableStyle(wxTR_MULTIPLE, !event.IsChecked()); }
