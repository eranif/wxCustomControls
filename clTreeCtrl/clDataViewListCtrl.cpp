#include "clDataViewListCtrl.h"
#include <algorithm>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>

clDataViewListCtrl::clDataViewListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                                       long style)
    : clControlWithItems(parent, id, pos, size, style)
{
    Bind(wxEVT_PAINT, &clDataViewListCtrl::OnPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent& event) { wxUnusedVar(event); });
}

clDataViewListCtrl::~clDataViewListCtrl() { Unbind(wxEVT_PAINT, &clDataViewListCtrl::OnPaint, this); }

void clDataViewListCtrl::ScrollRows(int steps, wxDirection direction) {}

void clDataViewListCtrl::ScrollToRow(int firstLine) {}

void clDataViewListCtrl::ProcessIdle() {}

void clDataViewListCtrl::OnPaint(wxPaintEvent& event)
{
    wxUnusedVar(event);
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);
    wxGCDC gcdc(dc);

    // Call the parent's Render method
    // this will render the background + header
    Render(dc);

    // Get list of items to draw
    clRowEntry::Vec_t items;

    // Draw the items
    RenderItems(dc, items);
}

int clDataViewListCtrl::ItemToRow(const wxDataViewItem& item) const { return m_model.ItemToRow(item); }

wxDataViewItem clDataViewListCtrl::RowToItem(int row) const { return m_model.RowToItem(row); }

int clDataViewListCtrl::GetFirstItemPosition() const { return m_firstItemOnScreen; }

clRowEntry::Vec_t clDataViewListCtrl::GetOnScreenItems() const
{
    clRowEntry::Vec_t items;
    m_model.GetItems(m_firstItemOnScreen, GetNumLineCanFitOnScreen(), items);
    return items;
}

int clDataViewListCtrl::GetRange() const { return m_model.size(); }

bool clDataViewListCtrl::IsEmpty() const { return m_model.empty(); }

void clDataViewListCtrl::AppendItem(const wxVector<wxVariant>& values, wxUIntPtr data)
{
    clRowEntry* child = new clRowEntry(this, "", wxNOT_FOUND, wxNOT_FOUND);
    child->SetClientData(data);
    for(size_t i = 0; i < values.size(); ++i) {
        const wxVariant& v = values[i];
        if(v.GetType() == "bool") {
            child->SetLabel((v.GetBool() ? "Yes" : "No"), i);
        } else if(v.GetType() == "string") {
            child->SetLabel(v.GetString(), i);
        }
    }
    m_model.Add(child);
}

wxDataViewColumn* clDataViewListCtrl::AppendIconTextColumn(const wxString& label, wxDataViewCellMode mode, int width,
                                                           wxAlignment align, int flags)
{
    wxUnusedVar(mode);
    wxUnusedVar(align);
    wxUnusedVar(flags);
    GetHeader().Add(label).SetWidth(width);
    return nullptr;
}

wxDataViewColumn* clDataViewListCtrl::AppendProgressColumn(const wxString& label, wxDataViewCellMode mode, int width,
                                                           wxAlignment align, int flags)
{
    wxUnusedVar(mode);
    wxUnusedVar(align);
    wxUnusedVar(flags);
    GetHeader().Add(label).SetWidth(width);
    return nullptr;
}

wxDataViewColumn* clDataViewListCtrl::AppendTextColumn(const wxString& label, wxDataViewCellMode mode, int width,
                                                       wxAlignment align, int flags)
{
    wxUnusedVar(mode);
    wxUnusedVar(align);
    wxUnusedVar(flags);
    GetHeader().Add(label).SetWidth(width);
    return nullptr;
}
