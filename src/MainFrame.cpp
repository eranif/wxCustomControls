#include "MainFrame.h"
#include "clCaptionBar.hpp"
#include "clColours.h"
#include "clMenuBar.hpp"
#include "clTreeCtrl.h"
#include <cmath>
#include <uxtheme.h>
#include <wx/aboutdlg.h>
#include <wx/checkbox.h>
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/ffile.h>
#include <wx/minifram.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/popupwin.h>
#include <wx/stattext.h>
#include <wx/stopwatch.h>
#include <wx/textdlg.h>
#include <wx/utils.h>

#ifdef __WXMSW__
#include "wx/msw/dc.h"
#include "wx/msw/private.h"
#include "wx/msw/uxtheme.h"
#endif

class MyDvData
{
public:
    wxString m_path;
    MyDvData(const wxString& path)
        : m_path(path)
    {
    }
    ~MyDvData() {}
};

static wxVariant MakeChoice(const wxString& label, int bmp_index)
{
    clDataViewChoice ict(label, bmp_index);
    wxVariant v;
    v << ict;
    return v;
}

static wxVariant MakeIconText(const wxString& text, int bmp_index)
{
    clDataViewTextBitmap ict(text, bmp_index);
    wxVariant v;
    v << ict;
    return v;
}

static wxVariant MakeCheckBox(bool checked, const wxString& text, int bmp_index)
{
    clDataViewCheckbox ict(checked, bmp_index, text);
    wxVariant v;
    v << ict;
    return v;
}

MainFrame::MainFrame(wxWindow* parent)
    : MainFrameBaseClass(parent)
{
    MyImages images;
    m_menuBar = new clMenuBar(this, 0, nullptr, nullptr);
    m_menuBar->FromMenuBar(GetMenuBar());
    GetSizer()->Insert(0, m_menuBar, 0, wxEXPAND);

    m_captionBar = new clCaptionBar(this, this);
    GetSizer()->Insert(0, m_captionBar, 0, wxEXPAND);
    m_captionBar->SetCaption("My Custom Caption");
    m_captionBar->SetBitmap(images.Bitmap("logo"));

    // Create some themes so we can toggle through them
    m_treeCtrl = new clTreeCtrl();
    m_treeCtrl->Create(m_panelControls, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                       wxTR_ROW_LINES | wxTR_ENABLE_SEARCH);
    m_panelControls->GetSizer()->Insert(0, m_treeCtrl, 1, wxEXPAND | wxALL, 2);

    m_comboBox->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event) {
        wxString combo_text;
        combo_text << "wxEVT_COMBOBOX captured. New Selection is " << event.GetSelection() << "\n";
        m_textCtrlLog->AppendText(combo_text);
    });

    m_buttonAction->SetText("Open workspace!");
    m_buttonAction->SetSubText("click me...");

    m_comboBox->Bind(wxEVT_TEXT, [this](wxCommandEvent& event) {
        wxUnusedVar(event);
        wxString combo_text;
        combo_text << "wxEVT_TEXT captured (combobox)\n";
        m_textCtrlLog->AppendText(combo_text);

        wxString str;
        wxArrayString selections = m_comboBox->GetStrings();
        for(const auto& s : selections) {
            str << s << " ";
        }
        m_textCtrlLog->AppendText("Strings: " + str + "\n");
    });

    m_comboBox->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent& event) {
        wxUnusedVar(event);
        wxString combo_text;
        combo_text << "wxEVT_TEXT_ENTER captured (combobox)\n";
        m_textCtrlLog->AppendText(combo_text);
    });
    std::vector<wxString> V = { "new string 1", "new string 2" };
    m_comboBox->Append(V);
    m_comboBox->SetHint("Hint...");
    m_comboBox->SetSelection(INVALID_SIZE_T);
    m_comboBox->CallAfter(&clComboBox::SetFocus);

    clColours colours;
    colours.InitDefaults();
    m_coloursArr[0] = colours;

    colours.InitFromColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    m_coloursArr[1] = colours;

    colours.InitFromColour(wxColour("WHITE"));
    m_coloursArr[2] = colours;

    colours.InitFromColour(wxColour("#424242"));
    m_coloursArr[3] = colours;

    m_treeCtrl->SetColours(m_coloursArr[m_selectedColours]);

    m_treeCtrl->AddHeader("Path");
    m_treeCtrl->AddHeader("Kind");
    m_treeCtrl->AddHeader("Size");
    m_treeCtrl->SetShowHeader(true);
    m_treeCtrl->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
    DoAddRoot();

    wxLog::SetActiveTarget(new wxLogTextCtrl(m_textCtrlLog));
    // Provide a sorting function to the tree
    clSortFunc_t SortFunc = [&](clRowEntry* a, clRowEntry* b) {
        // Compare based on item type and then by label
        MyItemData* cd_a = dynamic_cast<MyItemData*>(a->GetClientObject());
        MyItemData* cd_b = dynamic_cast<MyItemData*>(b->GetClientObject());
        if(cd_a->IsFolder() && !cd_b->IsFolder()) {
            return true;
        } else if(!cd_a->IsFolder() && cd_b->IsFolder()) {
            return false;
        } else {
            return cd_a->GetFullnameLC() < cd_b->GetFullnameLC();
        }
    };
    m_treeCtrl->SetSortFunction(SortFunc);

    m_bitmaps.push_back(images.Bitmap("folder"));
    m_bitmaps.push_back(images.Bitmap("folder_open"));
    m_bitmaps.push_back(images.Bitmap("file"));
    m_treeCtrl->SetBitmaps(&m_bitmaps);
    m_dataView->SetBitmaps(&m_bitmaps);
    // m_dataView->SetDefaultFont(wxFont(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE).FaceName("Fira Code")));
    // m_treeCtrl->SetDefaultFont(wxFont(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE).FaceName("Fira Code")));

    m_choice->SetBitmap(images.Bitmap("folder_open"));

    m_treeCtrl->Bind(wxEVT_TREE_ITEM_EXPANDING, &MainFrame::OnItemExpanding, this);
    m_treeCtrl->Bind(wxEVT_TREE_DELETE_ITEM, &MainFrame::OnItemDeleted, this);
    m_treeCtrl->Bind(wxEVT_TREE_BEGIN_DRAG, [&](wxTreeEvent& evt) {
        wxUnusedVar(evt);
        LogMessage(wxString() << "Drag started");
    });

    m_treeCtrl->Bind(wxEVT_TREE_END_DRAG, [&](wxTreeEvent& evt) {
        wxUnusedVar(evt);
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
        // LogMessage(wxString() << "Key down. selection is: " << m_treeCtrl->GetItemText(evt.GetItem()));
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
        LogMessage(wxString() << "Context menu for item: " << m_treeCtrl->GetItemText(evt.GetItem())
                              << ", Column: " << evt.GetInt());
        wxMenu menu;
        menu.Append(wxID_OPEN);
        menu.Bind(
            wxEVT_MENU,
            [&](wxCommandEvent& e) {
                wxUnusedVar(e);
                wxArrayTreeItemIds items;
                if(m_treeCtrl->GetSelections(items)) {
                    for(size_t i = 0; i < items.size(); ++i) {
                        MyItemData* cd = dynamic_cast<MyItemData*>(m_treeCtrl->GetItemData(items[i]));
                        if(cd) {
                            ::wxLaunchDefaultApplication(cd->GetPath());
                        }
                    }
                }
            },
            wxID_OPEN);
        menu.Append(wxID_NEW);
        menu.Bind(
            wxEVT_MENU,
            [&](wxCommandEvent& e) {
                wxUnusedVar(e);
                wxArrayTreeItemIds items;
                m_treeCtrl->GetSelections(items);
                MyItemData* cd = dynamic_cast<MyItemData*>(m_treeCtrl->GetItemData(items[0]));
                wxString name = ::wxGetTextFromUser("File name:");
                if(name.IsEmpty()) {
                    return;
                }
                wxFileName filename(cd->GetPath(), name);

                // Create the new file
                wxFFile fp;
                fp.Open(filename.GetFullPath(), "a+");
                fp.Close();

                // Add the file to the tree
                wxTreeItemId fileItem = m_treeCtrl->AppendItem(items[0], filename.GetFullName(), 2, 2,
                                                               new MyItemData(filename.GetFullPath(), false));
                m_treeCtrl->SetItemText(fileItem, "File", 1);
                wxString t;
                t << std::ceil((double)filename.GetSize().ToDouble() / 1024.0) << "KB";
                m_treeCtrl->SetItemText(fileItem, t, 2);
                m_treeCtrl->EnsureVisible(fileItem);
            },
            wxID_NEW);
        bool enableNew = false;
        wxArrayTreeItemIds items;
        if(m_treeCtrl->GetSelections(items) == 1) {
            MyItemData* cd = dynamic_cast<MyItemData*>(m_treeCtrl->GetItemData(items[0]));
            if(cd) {
                enableNew = cd->IsFolder();
            }
        }
        menu.Enable(wxID_NEW, enableNew);
        m_treeCtrl->PopupMenu(&menu);
    });

    //===-----------------------------------------------------------------------------
    //===-----------------------------------------------------------------------------
    // Data view events
    //===-----------------------------------------------------------------------------
    //===-----------------------------------------------------------------------------
    m_dataView->Bind(wxEVT_DATAVIEW_CHOICE_BUTTON, [&](wxDataViewEvent& e) {
        e.Skip();

        MyDvData* d = reinterpret_cast<MyDvData*>(m_dataView->GetItemData(e.GetItem()));
        wxFileName fn(d->m_path);

        wxArrayString choices;
        choices.Add(fn.GetFullName());
        choices.Add(fn.GetFullPath());
        m_dataView->ShowStringSelectionMenu(e.GetItem(), choices, e.GetColumn());
    });

    m_dataView->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, [&](wxDataViewEvent& event) {
        event.Skip();
        wxDataViewItemArray items;
        if(m_dataView->GetSelections(items)) {
            for(size_t i = 0; i < items.size(); ++i) {
                LogMessage(wxString() << "DV item activated: "
                                      << m_dataView->GetItemText(items.Item(i), event.GetColumn()));
                LogMessage(wxString() << "DV item state: "
                                      << m_dataView->IsItemChecked(items.Item(i), event.GetColumn()));
            }
        }
    });

    m_dataView->Bind(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, [&](wxDataViewEvent& event) {
        event.Skip();
        LogMessage(wxString() << "DV item value changed. Item label: "
                              << m_dataView->GetItemText(event.GetItem(), event.GetColumn())
                              << ". Column: " << event.GetColumn()
                              << ". New value is: " << m_dataView->IsItemChecked(event.GetItem(), event.GetColumn()));
    });

    m_dataView->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, [&](wxDataViewEvent& event) {
        event.Skip();
        wxDataViewItem item = m_dataView->GetSelection();
        LogMessage(wxString() << "DV selection changed: " << m_dataView->GetItemText(item));
    });

    m_dataView->Bind(wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, [&](wxDataViewEvent& event) {
        event.Skip();
        LogMessage("DV DnD started: ");
        wxDataViewItemArray items;
        if(m_dataView->GetSelections(items)) {
            for(size_t i = 0; i < items.size(); ++i) {
                LogMessage(wxString() << "DV DnD Item: " << m_dataView->GetItemText(items.Item(i)));
            }
        }
    });

    m_dataView->Bind(wxEVT_DATAVIEW_ITEM_DROP, [&](wxDataViewEvent& event) {
        event.Skip();
        LogMessage("DV DnD dropped: ");
        LogMessage(wxString() << "DV DnD Item: " << m_dataView->GetItemText(event.GetItem()));
    });

    m_dataView->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, [&](wxDataViewEvent& event) {
        event.Skip();
        LogMessage("DV Context menu dropped: ");
        LogMessage(wxString() << "Context menu on item: " << m_dataView->GetItemText(event.GetItem())
                              << ". Column: " << event.GetColumn());
    });
    m_treeCtrl->Bind(wxEVT_TREE_SEARCH_TEXT, &MainFrame::OnIncrementalSearch, this);
    m_treeCtrl->Bind(wxEVT_TREE_CLEAR_SEARCH, &MainFrame::OnResetSearch, this);
    m_dataView->Bind(wxEVT_DATAVIEW_SEARCH_TEXT, &MainFrame::OnDVIncrementalSearch, this);
    m_dataView->Bind(wxEVT_DATAVIEW_CLEAR_SEARCH, &MainFrame::OnDVResetSearch, this);

    // Toolbar
    m_toolbar->Bind(wxEVT_TOOL_DROPDOWN, &MainFrame::OnOpenMenu, this, wxID_OPEN);
    m_toolbar->Bind(wxEVT_TOOL, &MainFrame::OnOpen, this, wxID_OPEN);
    m_toolbar->AddControl(new wxCheckBox(m_toolbar, wxID_ANY, _("My Checkbox")));
    m_toolbar->SetMiniToolBar(false);
    m_toolbar->Realize();

    clToolBar* tb = new clToolBar(this);
    tb->SetMiniToolBar(true);
    tb->AddControl(new wxStaticText(tb, wxID_ANY, "", wxDefaultPosition, wxSize(250, -1)));
    tb->AddStretchableSpace();
    tb->AddTool(wxID_OPEN, _("Open"), images.Bitmap("folder"), "", wxITEM_NORMAL);
    tb->AddTool(wxID_CLEAR, _("Clear"), images.Bitmap("file"), "", wxITEM_NORMAL);
    tb->Realize();
    GetSizer()->Add(tb, 0, wxEXPAND);
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

void MainFrame::LogMessage(const wxString& message) { wxLogMessage(message); }

void MainFrame::OnOpenFolder(wxCommandEvent& event)
{
    wxUnusedVar(event);
    wxString path = wxDirSelector();
    if(path.IsEmpty()) {
        return;
    }

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

void MainFrame::OnExpandAll(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_treeCtrl->ExpandAll();
}
void MainFrame::OnCollapseAll(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_treeCtrl->CollapseAll();
}
void MainFrame::OnFirstVisible(wxCommandEvent& event)
{
    wxUnusedVar(event);
    wxTreeItemId item = m_treeCtrl->GetFirstVisibleItem();
    m_treeCtrl->SelectItem(item);
}

void MainFrame::OnNextVisible(wxCommandEvent& event)
{
    wxUnusedVar(event);
    wxTreeItemId item = m_treeCtrl->GetNextVisible(m_treeCtrl->GetFocusedItem());
    m_treeCtrl->SelectItem(item);
}
void MainFrame::OnEnsureItemVisible(wxCommandEvent& event)
{
    wxUnusedVar(event);
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

void MainFrame::OnSelectChildren(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_treeCtrl->SelectChildren(m_treeCtrl->GetFocusedItem());
}
void MainFrame::OnNextSibling(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_treeCtrl->SelectItem(m_treeCtrl->GetNextSibling(m_treeCtrl->GetFocusedItem()));
    m_treeCtrl->EnsureVisible(m_treeCtrl->GetFocusedItem());
}

void MainFrame::OnPrevSibling(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_treeCtrl->SelectItem(m_treeCtrl->GetPrevSibling(m_treeCtrl->GetFocusedItem()));
    m_treeCtrl->EnsureVisible(m_treeCtrl->GetFocusedItem());
}
void MainFrame::OnToggleTheme(wxCommandEvent& event)
{
    wxUnusedVar(event);
    ++m_selectedColours;
    if(m_selectedColours >= m_coloursArr.size()) {
        m_selectedColours = 0;
    }

    clColours& colours = m_coloursArr[m_selectedColours];
    m_treeCtrl->SetColours(colours);
    m_dataView->SetColours(colours);

    m_panelButtons->SetBackgroundColour(colours.GetBgColour());
    m_panelButtons->Refresh();
    m_buttonOne->SetColours(colours);
    m_buttonTwo->SetColours(colours);
    m_buttonDisabled->SetColours(colours);
    m_choice->SetColours(colours);
    m_captionBar->SetColours(colours);
    m_menuBar->SetColours(colours);
    m_treeCtrl->Refresh();
    m_dataView->Refresh();
}

void MainFrame::OnZebraColouring(wxCommandEvent& event)
{
    m_treeCtrl->EnableStyle(wxTR_ROW_LINES, event.IsChecked());
    m_dataView->EnableStyle(wxDV_ROW_LINES, event.IsChecked());
}

void MainFrame::OnHideRoot(wxCommandEvent& event) { m_treeCtrl->EnableStyle(wxTR_HIDE_ROOT, event.IsChecked()); }

void MainFrame::OnSingleSelection(wxCommandEvent& event)
{
    m_treeCtrl->EnableStyle(wxTR_MULTIPLE, !event.IsChecked());
    m_dataView->EnableStyle(wxDV_MULTIPLE, !event.IsChecked());
}

void MainFrame::OnShowSBOnFocus(wxCommandEvent& event) { m_treeCtrl->SetShowScrollBarOnFocus(event.IsChecked()); }
void MainFrame::OnHideHeaders(wxCommandEvent& event)
{
    m_treeCtrl->SetShowHeader(!event.IsChecked());
    m_dataView->SetShowHeader(!event.IsChecked());
}
void MainFrame::OnDeleteAllItems(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_treeCtrl->DeleteAllItems();
    DoAddRoot();
}

void MainFrame::DoAddRoot()
{
    wxTreeItemId root = m_treeCtrl->AddRoot("Root", -1, -1, nullptr);
    m_treeCtrl->SetItemText(root, "??", 1);
    m_treeCtrl->SetItemText(root, "0KB", 2);
}

void MainFrame::OnDVOpenFolder(wxCommandEvent& event)
{
    wxUnusedVar(event);
    wxString path = ::wxDirSelector();
    if(path.IsEmpty()) {
        return;
    }
    for(size_t i = 0; i < m_dataView->GetItemCount(); ++i) {
        MyDvData* d = reinterpret_cast<MyDvData*>(m_dataView->GetItemData(m_dataView->RowToItem(i)));
        wxDELETE(d);
    }
    m_dataView->DeleteAllItems();

    m_dataView->SetSortFunction(nullptr);
    // load the folders
    wxDir dir(path);
    if(dir.IsOpened()) {
        wxString filename;
        bool cont = dir.GetFirst(&filename);
        while(cont) {
            wxFileName fn(path, filename);
            wxVector<wxVariant> cols;
            MyDvData* d = new MyDvData(fn.GetFullPath());
            if(wxDirExists(fn.GetFullPath())) {
                // A directory
                cols.push_back(MakeCheckBox(false, fn.GetFullName(), 0));
                cols.push_back(false);
                cols.push_back("0KB");

            } else {
                // A file
                wxString t;
                t << std::ceil((double)fn.GetSize().ToDouble() / 1024.0) << "KB";
                cols.push_back(MakeChoice(fn.GetFullName(), 2));
                cols.push_back(true);
                cols.push_back(t);
            }
            m_dataView->AppendItem(cols, (wxUIntPtr)d);
            cont = dir.GetNext(&filename);
        }
    }

    // sort the view
    auto SortFunc = [&](clRowEntry* a, clRowEntry* b) { return a->GetLabel(0).CmpNoCase(b->GetLabel(0)) < 0; };
    m_dataView->SetSortFunction(SortFunc);
}

void MainFrame::OnNativeHeader(wxCommandEvent& event)
{
    m_treeCtrl->SetNativeTheme(event.IsChecked());
    m_dataView->SetNativeTheme(event.IsChecked());
}

void MainFrame::OnFillWith500Entries(wxCommandEvent& event)
{
    wxUnusedVar(event);
    wxBusyCursor bc;
    m_dataView->DeleteAllItems();

    wxStopWatch sw;
    sw.Start();
    size_t itemCount = 10000;
    wxString file_name = "A file name";
    wxString file_type = "File";
    wxString file_size = "100KB";

    // A nice trick to boost performance: remove the sorting method
    m_dataView->SetSortFunction(nullptr);
    for(size_t i = 0; i < itemCount; ++i) {
        wxVector<wxVariant> cols;
        cols.push_back(MakeIconText(wxString() << "File #" << i, 0));
        cols.push_back("File");
        cols.push_back("0KB");
        m_dataView->AppendItem(cols);
    }
    auto SortFunc = [&](clRowEntry* a, clRowEntry* b) { return a->GetLabel(0).CmpNoCase(b->GetLabel(0)) < 0; };
    m_dataView->SetSortFunction(SortFunc);
    long timepassed = sw.Time();

    // Now that we got all the items populated, set a sorting function
    LogMessage(wxString() << "Added " << itemCount << " entries in: " << timepassed << "ms");
}

void MainFrame::OnDVDeleteAllItems(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_dataView->DeleteAllItems();
}

void MainFrame::OnMenuitemsupportsearchMenuSelected(wxCommandEvent& event)
{
    m_treeCtrl->EnableStyle(wxTR_ENABLE_SEARCH, event.IsChecked(), true);
    m_dataView->EnableStyle(wxDV_ENABLE_SEARCH, event.IsChecked(), true);
}

void MainFrame::OnTreeFind(wxCommandEvent& event)
{
    wxUnusedVar(event);
    wxString findwhat = wxGetTextFromUser("Text to search", "Find");
    if(findwhat.IsEmpty()) {
        return;
    }

    m_treeCtrl->ClearAllHighlights();
    wxTreeItemId where =
        m_treeCtrl->FindNext(m_treeCtrl->GetSelection().IsOk() ? m_treeCtrl->GetSelection() : wxTreeItemId(), findwhat,
                             0, wxTR_SEARCH_DEFAULT);
    if(where.IsOk()) {
        m_treeCtrl->SelectItem(where);
        m_treeCtrl->EnsureVisible(where);
        m_treeCtrl->HighlightText(where, true);
    }
}

void MainFrame::OnIncrementalSearch(wxTreeEvent& event)
{
    wxString findWhat = event.GetString();
    m_treeCtrl->ClearHighlight(m_matchedItem);
    m_matchedItem = m_treeCtrl->FindNext(m_treeCtrl->GetSelection(), findWhat, 0, wxTR_SEARCH_DEFAULT);
    if(m_matchedItem.IsOk()) {
        // Select the item
        m_treeCtrl->SelectItem(m_matchedItem);
        // Make sure its visible
        m_treeCtrl->EnsureVisible(m_matchedItem);
        // Highlight the result
        m_treeCtrl->HighlightText(m_matchedItem, true);
    }
}

void MainFrame::OnResetSearch(wxTreeEvent& event)
{
    wxUnusedVar(event);
    m_treeCtrl->ClearAllHighlights();
    m_matchedItem = wxTreeItemId();
}

void MainFrame::OnDVIncrementalSearch(wxDataViewEvent& event)
{
    wxString findWhat = event.GetString();
    m_dataView->ClearHighlight(m_dvMatchedItem);
    m_dvMatchedItem = m_dataView->FindNext(m_dataView->GetSelection(), findWhat, 0, wxTR_SEARCH_DEFAULT);
    if(m_dvMatchedItem.IsOk()) {
        // Select the item
        m_dataView->Select(m_dvMatchedItem);
        // Make sure its visible
        m_dataView->EnsureVisible(m_dvMatchedItem);
        // Highlight the result
        m_dataView->HighlightText(m_dvMatchedItem, true);
    }
}

void MainFrame::OnDVResetSearch(wxDataViewEvent& event)
{
    wxUnusedVar(event);
    m_dataView->ClearAllHighlights();
    m_dvMatchedItem = wxDataViewItem();
}
void MainFrame::OnOpen(wxCommandEvent& event)
{
    wxUnusedVar(event);
    wxMessageBox("Open button clicked!");
}

void MainFrame::OnOpenMenu(wxCommandEvent& event)
{
    wxUnusedVar(event);
    wxMenu m;
    m.Append(XRCID("item_1"), _("Open file 1"));
    m.Append(XRCID("item_1"), _("Open file 2"));
    m_toolbar->ShowMenuForButton(event.GetId(), &m);
}
void MainFrame::OnColoursUI(wxUpdateUIEvent& event) { event.Enable(false); }
void MainFrame::OnSetTreeColWidth(wxCommandEvent& event)
{
    wxUnusedVar(event);
    int width = ::wxGetNumberFromUser(_("Set the column width"), _("New width"), _("New width"), 300, -2, 1000, this);
    m_treeCtrl->SetColumnWidth(0, width);
}
void MainFrame::OnButtonClicked(wxCommandEvent& event)
{
    wxUnusedVar(event);
    LogMessage("Button clicked!");
}

void MainFrame::OnCloseFolderUI(wxUpdateUIEvent& event)
{
    event.Enable(m_treeCtrl->ItemHasChildren(m_treeCtrl->GetRootItem()));
}

void MainFrame::OnButtonCloseFolder(wxCommandEvent& event) { OnDeleteAllItems(event); }
void MainFrame::OnChoice(wxCommandEvent& event)
{
    wxUnusedVar(event);
    LogMessage(wxString() << "Choice event. " << m_choice->GetStringSelection());
}
void MainFrame::OnLineDown(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_dataView->LineDown();
}
void MainFrame::OnLineUp(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_dataView->LineUp();
}
void MainFrame::OnPgDown(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_dataView->PageDown();
}
void MainFrame::OnPgUp(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_dataView->PageUp();
}
void MainFrame::OnHideScrollbars(wxCommandEvent& event)
{
    m_dataView->SetNeverShowScrollBar(wxHORIZONTAL, event.IsChecked());
    m_dataView->SetNeverShowScrollBar(wxVERTICAL, event.IsChecked());

    m_treeCtrl->SetNeverShowScrollBar(wxHORIZONTAL, event.IsChecked());
    m_treeCtrl->SetNeverShowScrollBar(wxVERTICAL, event.IsChecked());
}
