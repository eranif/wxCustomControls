#include "clDataViewListCtrl.h"
#include "clHeaderItem.h"
#include <wx/dataview.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>

clDataViewListCtrl::clDataViewListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                                       long style)
    : clTreeCtrl(parent, id, pos, size, 0)
{
    // Map clDataViewListCtrl to clTreeCtrl style
    SetShowHeader(true);
    if(style & wxDV_ROW_LINES) { EnableStyle(wxTR_ROW_LINES, true, false); }
    if(style & wxDV_MULTIPLE) { EnableStyle(wxTR_MULTIPLE, true, false); }
    if(style & wxDV_NO_HEADER) { SetShowHeader(false); }
    EnableStyle(wxTR_HIDE_ROOT, true);
}

clDataViewListCtrl::~clDataViewListCtrl() {}

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
        SetItemText(item, child->GetLabel(i), i);
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
