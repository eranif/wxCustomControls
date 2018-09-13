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

int clDataViewListCtrl::ItemToRow(const wxDataViewItem& item) const
{
    int index = 0;
    std::vector<wxDataViewItem>::const_iterator iter =
        std::find_if(m_items.begin(), m_items.end(), [&](const wxDataViewItem& p) {
            if(p.GetID() == item.GetID()) { return true; }
            ++index;
            return false;
        });
    return iter == m_items.end() ? wxNOT_FOUND : index;
}

wxDataViewItem clDataViewListCtrl::RowToItem(int row) const
{
    if((row >= (int)m_items.size()) || (row < 0)) { return wxDataViewItem(); }
    return wxDataViewItem(m_items[row]);
}
