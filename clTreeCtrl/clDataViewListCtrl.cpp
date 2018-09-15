#include "clDataViewListCtrl.h"
#include "clHeaderItem.h"
#include <wx/dataview.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>

#define DV_ITEM(tree_item) wxDataViewItem(tree_item.GetID())
#define TREE_ITEM(dv_item) wxTreeItemId(dv_item.GetID())

clDataViewListCtrl::clDataViewListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                                       long style)
    : clTreeCtrl(parent, id, pos, size, 0)
{
    // Map clDataViewListCtrl to clTreeCtrl style
    SetShowHeader(true);
    int my_style = 0;
    if(style & wxDV_ROW_LINES) { my_style |= wxTR_ROW_LINES; }
    if(style & wxDV_MULTIPLE) { my_style |= wxTR_MULTIPLE; }
    if(style & wxDV_NO_HEADER) { SetShowHeader(false); }
    my_style |= wxTR_HIDE_ROOT;
    m_treeStyle = my_style;

    // Ignore these events
    Bind(wxEVT_TREE_ITEM_EXPANDING, [](wxTreeEvent& e) { e.Skip(); });
    Bind(wxEVT_TREE_ITEM_EXPANDED, [](wxTreeEvent& e) { e.Skip(); });
    Bind(wxEVT_TREE_ITEM_COLLAPSING, [](wxTreeEvent& e) { e.Skip(); });
    Bind(wxEVT_TREE_ITEM_COLLAPSED, [](wxTreeEvent& e) { e.Skip(); });
    Bind(wxEVT_TREE_DELETE_ITEM, [](wxTreeEvent& e) { e.Skip(); });
    Bind(wxEVT_TREE_DELETE_ITEM, [](wxTreeEvent& e) { e.Skip(); });
    Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, [](wxTreeEvent& e) { e.Skip(); });

    // Translate the following events to wxDVC events
    Bind(wxEVT_TREE_BEGIN_DRAG, &clDataViewListCtrl::OnConvertEvent, this);
    Bind(wxEVT_TREE_END_DRAG, &clDataViewListCtrl::OnConvertEvent, this);
    Bind(wxEVT_TREE_SEL_CHANGED, &clDataViewListCtrl::OnConvertEvent, this);
    Bind(wxEVT_TREE_ITEM_ACTIVATED, &clDataViewListCtrl::OnConvertEvent, this);
    Bind(wxEVT_TREE_ITEM_MENU, &clDataViewListCtrl::OnConvertEvent, this);
}

clDataViewListCtrl::~clDataViewListCtrl()
{
    Unbind(wxEVT_TREE_BEGIN_DRAG, &clDataViewListCtrl::OnConvertEvent, this);
    Unbind(wxEVT_TREE_END_DRAG, &clDataViewListCtrl::OnConvertEvent, this);
    Unbind(wxEVT_TREE_SEL_CHANGED, &clDataViewListCtrl::OnConvertEvent, this);
    Unbind(wxEVT_TREE_ITEM_ACTIVATED, &clDataViewListCtrl::OnConvertEvent, this);
    Unbind(wxEVT_TREE_ITEM_MENU, &clDataViewListCtrl::OnConvertEvent, this);
}

void clDataViewListCtrl::AppendItem(const wxVector<wxVariant>& values, wxUIntPtr data)
{
    clRowEntry* child = new clRowEntry(this, "", wxNOT_FOUND, wxNOT_FOUND);
    child->SetData(data);
    wxTreeItemId item = clTreeCtrl::AppendItem(GetRootItem(), "", -1, -1, nullptr);
    for(size_t i = 0; i < values.size(); ++i) {
        const wxVariant& v = values[i];
        if(v.GetType() == "bool") {
            child->SetLabel((v.GetBool() ? "Yes" : "No"), i);
        } else if(v.GetType() == "string") {
            child->SetLabel(v.GetString(), i);
        }
        clTreeCtrl::SetItemText(item, child->GetLabel(i), i);
    }
}

wxDataViewColumn* clDataViewListCtrl::AppendIconTextColumn(const wxString& label, wxDataViewCellMode mode, int width,
                                                           wxAlignment align, int flags)
{
    wxUnusedVar(mode);
    wxUnusedVar(align);
    wxUnusedVar(flags);
    DoAddHeader(label, width);
    return nullptr;
}

wxDataViewColumn* clDataViewListCtrl::AppendProgressColumn(const wxString& label, wxDataViewCellMode mode, int width,
                                                           wxAlignment align, int flags)
{
    wxUnusedVar(mode);
    wxUnusedVar(align);
    wxUnusedVar(flags);
    DoAddHeader(label, width);
    return nullptr;
}

wxDataViewColumn* clDataViewListCtrl::AppendTextColumn(const wxString& label, wxDataViewCellMode mode, int width,
                                                       wxAlignment align, int flags)
{
    wxUnusedVar(mode);
    wxUnusedVar(align);
    wxUnusedVar(flags);
    DoAddHeader(label, width);
    return nullptr;
}

void clDataViewListCtrl::DoAddHeader(const wxString& label, int width)
{
    if(m_needToClearDefaultHeader) {
        m_needToClearDefaultHeader = false;
        GetHeader().Clear();
    }
    clHeaderItem& col = GetHeader().Add(label);
    if(width > 0) { col.SetWidth(width); }
}

int clDataViewListCtrl::GetSelections(wxDataViewItemArray& sel) const
{
    wxArrayTreeItemIds items;
    clTreeCtrl::GetSelections(items);
    for(size_t i = 0; i < items.size(); ++i) {
        sel.Add(DV_ITEM(items[i]));
    }
    return sel.size();
}

int clDataViewListCtrl::GetSelectedItemsCount() const { return m_model.GetSelectionsCount(); }

void clDataViewListCtrl::OnConvertEvent(wxTreeEvent& event)
{
    wxEventType type = wxEVT_ANY;
    if(event.GetEventType() == wxEVT_TREE_BEGIN_DRAG) {
        type = wxEVT_DATAVIEW_ITEM_BEGIN_DRAG;
    } else if(event.GetEventType() == wxEVT_TREE_END_DRAG) {
        type = wxEVT_DATAVIEW_ITEM_DROP;
    } else if(event.GetEventType() == wxEVT_TREE_SEL_CHANGED) {
        type = wxEVT_DATAVIEW_SELECTION_CHANGED;
    } else if(event.GetEventType() == wxEVT_TREE_ITEM_ACTIVATED) {
        type = wxEVT_DATAVIEW_ITEM_ACTIVATED;
    } else if(event.GetEventType() == wxEVT_TREE_ITEM_MENU) {
        type = wxEVT_DATAVIEW_ITEM_CONTEXT_MENU;
    }
    if(type != wxEVT_ANY) { SendDataViewEvent(type, event); }
}

bool clDataViewListCtrl::SendDataViewEvent(const wxEventType& type, wxTreeEvent& treeEvent)
{
    wxDataViewEvent e(type, &m_dummy, DV_ITEM(treeEvent.GetItem()));
    e.SetEventObject(this);
    if(!GetEventHandler()->ProcessEvent(e)) {
        treeEvent.Skip();
        return false;
    }
    return true;
}

void clDataViewListCtrl::DeleteAllItems()
{
    // DVC must allways have the hidden root
    clTreeCtrl::DeleteAllItems();
    AddRoot("Hidden Root", -1, -1, nullptr);
}

wxDataViewItem clDataViewListCtrl::AppendItem(const wxString& text, int image, int selImage, wxUIntPtr data)
{
    wxTreeItemId child = clTreeCtrl::AppendItem(GetRootItem(), text, image, selImage, nullptr);
    wxDataViewItem dvItem = DV_ITEM(child);
    SetItemData(dvItem, data);
    return dvItem;
}

wxDataViewItem clDataViewListCtrl::InsertItem(const wxDataViewItem& previous, const wxString& text, int image,
                                              int selImage, wxUIntPtr data)
{
    wxTreeItemId child =
        clTreeCtrl::InsertItem(GetRootItem(), wxTreeItemId(previous.GetID()), text, image, selImage, nullptr);
    wxDataViewItem dvItem = DV_ITEM(child);
    SetItemData(dvItem, data);
    return dvItem;
}

wxUIntPtr clDataViewListCtrl::GetItemData(const wxDataViewItem& item) const
{
    clRowEntry* r = m_model.ToPtr(TREE_ITEM(item));
    return r->GetData();
}

void clDataViewListCtrl::SetItemData(const wxDataViewItem& item, wxUIntPtr data)
{
    clRowEntry* r = m_model.ToPtr(TREE_ITEM(item));
    r->SetData(data);
}

void clDataViewListCtrl::SetItemBackgroundColour(const wxDataViewItem& item, const wxColour& colour, size_t col)
{
    clTreeCtrl::SetItemBackgroundColour(TREE_ITEM(item), colour, col);
}

wxColour clDataViewListCtrl::GetItemBackgroundColour(const wxDataViewItem& item, size_t col) const
{
    return clTreeCtrl::GetItemBackgroundColour(TREE_ITEM(item), col);
}

void clDataViewListCtrl::SetItemTextColour(const wxDataViewItem& item, const wxColour& colour, size_t col)
{
    clTreeCtrl::SetItemTextColour(TREE_ITEM(item), colour, col);
}

wxColour clDataViewListCtrl::GetItemTextColour(const wxDataViewItem& item, size_t col) const
{
    return clTreeCtrl::GetItemTextColour(TREE_ITEM(item), col);
}

void clDataViewListCtrl::SetItemText(const wxDataViewItem& item, const wxString& text, size_t col)
{
    clTreeCtrl::SetItemText(TREE_ITEM(item), text, col);
}

wxString clDataViewListCtrl::GetItemText(const wxDataViewItem& item, size_t col) const
{
    return clTreeCtrl::GetItemText(TREE_ITEM(item), col);
}

void clDataViewListCtrl::SetItemBold(const wxDataViewItem& item, bool bold, size_t col)
{
    clTreeCtrl::SetItemBold(TREE_ITEM(item), bold, col);
}

void clDataViewListCtrl::SetItemFont(const wxDataViewItem& item, const wxFont& font, size_t col)
{
    clTreeCtrl::SetItemFont(TREE_ITEM(item), font, col);
}

wxFont clDataViewListCtrl::GetItemFont(const wxDataViewItem& item, size_t col) const
{
    return clTreeCtrl::GetItemFont(TREE_ITEM(item), col);
}
