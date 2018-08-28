#include "clTreeCtrl.h"
#include "clTreeCtrlModel.h"
#include <cmath>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/dcmemory.h>
#include <wx/renderer.h>
#include <wx/settings.h>
#include <wx/wupdlock.h>

#define Y_SPACER 2

clTreeCtrl::clTreeCtrl(wxWindow* parent)
    : wxScrolled<wxWindow>(parent)
    , m_model(this)
{
    wxBitmap bmp(1, 1);
    wxMemoryDC memDC(bmp);
    wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    memDC.SetFont(font);
    wxSize textSize = memDC.GetTextExtent("Tp");
    m_lineHeight = Y_SPACER + textSize.GetHeight() + Y_SPACER;
    SetScrollRate(10, m_lineHeight);
    Bind(wxEVT_PAINT, &clTreeCtrl::OnPaint, this);
    Bind(wxEVT_SIZE, &clTreeCtrl::OnSize, this);
    Bind(wxEVT_ERASE_BACKGROUND, [&](wxEraseEvent& event) { wxUnusedVar(event); });
    Bind(wxEVT_LEFT_DOWN, &clTreeCtrl::OnMouseLeftDown, this);
    Bind(wxEVT_LEFT_DCLICK, &clTreeCtrl::OnMouseLeftDClick, this);
    DoAdjustScrollbars();
}

clTreeCtrl::~clTreeCtrl() {}

void clTreeCtrl::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC pdc(this);
    DoPrepareDC(pdc);

    wxGCDC dc(pdc);

    int realX, realY;
    CalcScrolledPosition(0, 0, &realX, &realY);
    wxRect clientRect = GetClientRect();
    clientRect.SetX(abs(realX));
    clientRect.SetY(abs(realY));
    dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    dc.DrawRectangle(clientRect);

    int startLine = abs(realY / m_lineHeight);
    int lastLine = ceil((double)clientRect.GetHeight() / (double)m_lineHeight) + startLine;
    int totalVisibleLines = GetExpandedLines();
    if(lastLine > totalVisibleLines) { lastLine = totalVisibleLines; }

    int y = clientRect.GetY();
    std::vector<clTreeCtrlNode*> items;
    m_model.GetItemsFromIndex(startLine, lastLine - startLine, items);

    // Colours
    wxColour itemTextColour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    wxColour selItemTextColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    wxColour selItemBgColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);

    for(size_t i = 0; i < items.size(); ++i) {
        clTreeCtrlNode* curitem = items[i];
        int textY = y + Y_SPACER;
        wxRect itemRect = wxRect(0, y, clientRect.GetWidth(), m_lineHeight);
        curitem->SetRect(itemRect);
        if(curitem->IsSelected()) {
            dc.SetBrush(selItemBgColour);
            dc.SetPen(selItemBgColour);
            dc.DrawRoundedRectangle(itemRect, 1.5);
        }
        dc.SetTextForeground(curitem->IsSelected() ? selItemTextColour : itemTextColour);
        dc.DrawText(curitem->GetLabel(), curitem->GetIndent(), textY);
        y += m_lineHeight;
    }
    m_model.SetVisibleItems(items); // Keep track of the visible items
}

void clTreeCtrl::OnSize(wxSizeEvent& event)
{
    Refresh();
    event.Skip();
}

void clTreeCtrl::DoAdjustScrollbars()
{
#ifdef __WXMSW__
    wxWindowUpdateLocker locker(this);
#endif

    int curx, cury;
    GetViewStart(&curx, &cury);
    wxUnusedVar(curx);

    wxRect clientRect = GetClientRect();
    int realX, realY;
    CalcScrolledPosition(0, 0, &realX, &realY);
    int startLine = abs(realY / m_lineHeight);
    int lastLine = ceil((double)clientRect.GetHeight() / (double)m_lineHeight) + startLine;
    int visibleLines = GetExpandedLines();
    if(lastLine > visibleLines) { lastLine = visibleLines; }

    wxBitmap bmp(1, 1);
    wxMemoryDC memDC(bmp);
    memDC.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    int textWidth = 150;

    // The number of horizontal units is the textwidth / 10
    int noUnitsX = ceil((double)textWidth / 10.0);

    // Calculate how many lines can be draw onto the page
    SetScrollbars(10, m_lineHeight, noUnitsX, visibleLines);
    Scroll(0, cury);
}

wxTreeItemId clTreeCtrl::AppendItem(
    const wxTreeItemId& parent, const wxString& text, int image, int selImage, wxTreeItemData* data)
{
    return m_model.AppendItem(parent, text, image, selImage, data);
}

wxTreeItemId clTreeCtrl::AddRoot(const wxString& text, int image, int selImage, wxTreeItemData* data)
{
    return m_model.AddRoot(text, image, selImage, data);
}

wxTreeItemId clTreeCtrl::GetRootItem() const { return m_model.GetRootItem(); }

void clTreeCtrl::Expand(const wxTreeItemId& item)
{
    if(!item.GetID()) return;
    clTreeCtrlNode* child = reinterpret_cast<clTreeCtrlNode*>(item.GetID());
    if(!child) return;
    child->SetExpanded(true);
    m_model.StateModified();
    Refresh();
}

void clTreeCtrl::Collapse(const wxTreeItemId& item)
{
    if(!item.GetID()) return;
    clTreeCtrlNode* child = reinterpret_cast<clTreeCtrlNode*>(item.GetID());
    if(!child) return;
    child->SetExpanded(false);
    m_model.StateModified();
    Refresh();
}

int clTreeCtrl::GetExpandedLines() { return m_model.GetExpandedLines(); }

void clTreeCtrl::SelectItem(const wxTreeItemId& item, bool select)
{
    m_model.SelectItem(item, select);
    Refresh();
}

void clTreeCtrl::OnMouseLeftDown(wxMouseEvent& event)
{
    int flags = 0;
    wxPoint pt = DoFixPoint(event.GetPosition());
    wxTreeItemId where = HitTest(pt, flags);
    if(where.IsOk()) {
        UnselectAll();
        SelectItem(where, true);
        Refresh();
    }
}

wxTreeItemId clTreeCtrl::HitTest(const wxPoint& point, int& flags) const
{
    for(size_t i = 0; i < m_model.GetVisibleItems().size(); ++i) {
        const clTreeCtrlNode* item = m_model.GetVisibleItems()[i];
        if(item->GetRect().Contains(point)) { return wxTreeItemId(const_cast<clTreeCtrlNode*>(item)); }
    }
    return wxTreeItemId();
}

void clTreeCtrl::UnselectAll()
{
    m_model.UnselectAll();
    Refresh();
}

wxPoint clTreeCtrl::DoFixPoint(const wxPoint& pt)
{
    wxPoint point = pt;
    int realX, realY;
    CalcScrolledPosition(0, 0, &realX, &realY);
    point.x += (realX);
    point.y += abs(realY);
    return point;
}

void clTreeCtrl::EnsureVisible(const wxTreeItemId& item)
{
    // Make sure that all parents of ítem are expanded
    if(!m_model.ExpandToItem(item)) { return; }
    DoAdjustScrollbars();
    Refresh();
    CallAfter(&clTreeCtrl::DoEnsureVisible, item);
}

void clTreeCtrl::DoEnsureVisible(const wxTreeItemId& item)
{
    // scroll to it
    int index = m_model.GetItemIndex(item, true);
    if(index != wxNOT_FOUND) { Scroll(0, index); }
}

void clTreeCtrl::OnMouseLeftDClick(wxMouseEvent& event)
{
    event.Skip();
    int flags = 0;
    wxPoint pt = DoFixPoint(event.GetPosition());
    wxTreeItemId where = HitTest(pt, flags);
    if(where.IsOk()) {
        UnselectAll();
        SelectItem(where, true);
        if(ItemHasChildren(where)) {
            if(IsExpanded(where)) {
                Collapse(where);
            } else {
                Expand(where);
            }
            DoAdjustScrollbars();
        }
    }
}

bool clTreeCtrl::IsExpanded(const wxTreeItemId& item) const
{
    if(!item.GetID()) return false;
    clTreeCtrlNode* child = reinterpret_cast<clTreeCtrlNode*>(item.GetID());
    if(!child) return false;
    return child->IsExpanded();
}

bool clTreeCtrl::ItemHasChildren(const wxTreeItemId& item) const
{
    if(!item.GetID()) return false;
    clTreeCtrlNode* child = reinterpret_cast<clTreeCtrlNode*>(item.GetID());
    if(!child) return false;
    return child->HasChildren();
}
