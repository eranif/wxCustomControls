#include "clScrollBar.h"
#include "clTreeCtrl.h"
#include "clTreeCtrlModel.h"
#include <algorithm>
#include <cmath>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/dcmemory.h>
#include <wx/log.h>
#include <wx/renderer.h>
#include <wx/settings.h>
#include <wx/utils.h>
#include <wx/wupdlock.h>

#define CHECK_PTR_RET(p) \
    if(!p) {             \
        return;          \
    }
#define CHECK_ITEM_RET(item) \
    if(!item.IsOk()) {       \
        return;              \
    }

#define CHECK_ROOT_RET()     \
    if(!m_model.GetRoot()) { \
        return;              \
    }

clTreeCtrl::clTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, wxID_ANY, pos, size, wxWANTS_CHARS | wxTAB_TRAVERSAL)
    , m_model(this)
    , m_treeStyle(style)
{
    wxBitmap bmp(1, 1);
    wxMemoryDC memDC(bmp);
    wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    memDC.SetFont(font);
    wxSize textSize = memDC.GetTextExtent("Tp");
    m_lineHeight = clTreeCtrlNode::Y_SPACER + textSize.GetHeight() + clTreeCtrlNode::Y_SPACER;
    SetIndent(m_lineHeight);
    Bind(wxEVT_IDLE, &clTreeCtrl::OnIdle, this);
    Bind(wxEVT_PAINT, &clTreeCtrl::OnPaint, this);
    Bind(wxEVT_SIZE, &clTreeCtrl::OnSize, this);
    Bind(wxEVT_ERASE_BACKGROUND, [&](wxEraseEvent& event) { wxUnusedVar(event); });
    Bind(wxEVT_LEFT_DOWN, &clTreeCtrl::OnMouseLeftDown, this);
    Bind(wxEVT_LEFT_DCLICK, &clTreeCtrl::OnMouseLeftDClick, this);
    Bind(wxEVT_MOUSEWHEEL, &clTreeCtrl::OnMouseScroll, this);
    Bind(wxEVT_LEAVE_WINDOW, &clTreeCtrl::OnLeaveWindow, this);
    Bind(wxEVT_KEY_DOWN, &clTreeCtrl::OnKeyDown, this);
    Bind(wxEVT_CONTEXT_MENU, &clTreeCtrl::OnContextMenu, this);
    Bind(wxEVT_RIGHT_DOWN, &clTreeCtrl::OnRightDown, this);

    m_scrollBar = new clScrollBar(this, wxVERTICAL);
    m_scrollBar->Bind(wxEVT_SCROLL_THUMBTRACK, &clTreeCtrl::OnScroll, this);
    // Initialise default colours
    m_colours.InitDefaults();
}

clTreeCtrl::~clTreeCtrl()
{
    Unbind(wxEVT_IDLE, &clTreeCtrl::OnIdle, this);
    Unbind(wxEVT_PAINT, &clTreeCtrl::OnPaint, this);
    Unbind(wxEVT_SIZE, &clTreeCtrl::OnSize, this);
    Unbind(wxEVT_ERASE_BACKGROUND, [&](wxEraseEvent& event) { wxUnusedVar(event); });
    Unbind(wxEVT_LEFT_DOWN, &clTreeCtrl::OnMouseLeftDown, this);
    Unbind(wxEVT_LEFT_DCLICK, &clTreeCtrl::OnMouseLeftDClick, this);
    Unbind(wxEVT_MOUSEWHEEL, &clTreeCtrl::OnMouseScroll, this);
    Unbind(wxEVT_LEAVE_WINDOW, &clTreeCtrl::OnLeaveWindow, this);
    Unbind(wxEVT_KEY_DOWN, &clTreeCtrl::OnKeyDown, this);
    Unbind(wxEVT_CONTEXT_MENU, &clTreeCtrl::OnContextMenu, this);
    Unbind(wxEVT_RIGHT_DOWN, &clTreeCtrl::OnRightDown, this);
}

void clTreeCtrl::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC pdc(this);
    wxGCDC dc(pdc);

    wxRect clientRect = GetClientRect();
    dc.SetPen(m_colours.bgColour);
    dc.SetBrush(m_colours.bgColour);
    dc.DrawRectangle(clientRect);

    if(!m_model.GetRoot()) {
        // Reset the various items
        SetFirstItemOnScreen(nullptr);
        clTreeCtrlNode::Vec_t items;
        m_model.SetOnScreenItems(items);
        return;
    }

    int maxItems = GetNumLineCanFitOnScreen();
    if(!GetFirstItemOnScreen()) {
        SetFirstItemOnScreen(m_model.GetRoot());
    }
    clTreeCtrlNode* firstItem = GetFirstItemOnScreen();
    if(!firstItem) {
        return;
    }
    int y = clientRect.GetY();
    clTreeCtrlNode::Vec_t items;
    m_model.GetNextItems(firstItem, maxItems, items);

    for(size_t i = 0; i < items.size(); ++i) {
        clTreeCtrlNode* curitem = items[i];
        wxRect itemRect = wxRect(0, y, clientRect.GetWidth(), m_lineHeight);
        wxRect buttonRect;
        if(curitem->HasChildren()) {
            buttonRect = wxRect((curitem->GetIndentsCount() * GetIndent()), y, m_lineHeight, m_lineHeight);
        }
        curitem->SetRects(itemRect, buttonRect);
        curitem->Render(dc, m_colours);
        y += m_lineHeight;
    }
    m_model.SetOnScreenItems(items); // Keep track of the visible items
    UpdateScrollBar(dc);
}

void clTreeCtrl::OnSize(wxSizeEvent& event)
{
    event.Skip();
    Refresh();
}

wxTreeItemId clTreeCtrl::InsertItem(const wxTreeItemId& parent, const wxTreeItemId& previous, const wxString& text,
    int image, int selImage, wxTreeItemData* data)
{
    wxTreeItemId item = m_model.InsertItem(parent, previous, text, image, selImage, data);
    return item;
}

wxTreeItemId clTreeCtrl::AppendItem(
    const wxTreeItemId& parent, const wxString& text, int image, int selImage, wxTreeItemData* data)
{
    wxTreeItemId item = m_model.AppendItem(parent, text, image, selImage, data);
    return item;
}

wxTreeItemId clTreeCtrl::AddRoot(const wxString& text, int image, int selImage, wxTreeItemData* data)
{
    wxTreeItemId root = m_model.AddRoot(text, image, selImage, data);
    return root;
}

wxTreeItemId clTreeCtrl::GetRootItem() const { return m_model.GetRootItem(); }

void clTreeCtrl::Expand(const wxTreeItemId& item)
{
    if(!item.GetID()) return;
    clTreeCtrlNode* child = m_model.ToPtr(item);
    if(!child) return;
    child->SetExpanded(true);
    Refresh();
}

void clTreeCtrl::Collapse(const wxTreeItemId& item)
{
    if(!item.GetID()) return;
    clTreeCtrlNode* child = m_model.ToPtr(item);
    if(!child) return;
    child->SetExpanded(false);
    Refresh();
}

void clTreeCtrl::SelectItem(const wxTreeItemId& item, bool select)
{
    m_model.SelectItem(item, select);
    Refresh();
}

void clTreeCtrl::OnMouseLeftDown(wxMouseEvent& event)
{
    event.Skip();
    CHECK_ROOT_RET();

    int flags = 0;
    wxPoint pt = DoFixPoint(event.GetPosition());
    wxTreeItemId where = HitTest(pt, flags);
    if(where.IsOk()) {
        if(flags & wxTREE_HITTEST_ONITEMBUTTON) {
            if(IsExpanded(where)) {
                Collapse(where);
            } else {
                Expand(where);
            }
        } else {
            clTreeCtrlNode* pNode = m_model.ToPtr(where);
            if(event.ControlDown()) {
                // Add one to the current selections
                m_model.SelectItem(where, !pNode->IsSelected(), true);
            } else if(event.ShiftDown()) {
                // Range selection
                clTreeCtrlNode::Vec_t range;
                std::vector<std::pair<wxTreeItemId, bool>> itemsToSelect;
                m_model.GetRange(pNode, m_model.ToPtr(m_model.GetSingleSelection()), range);
                std::for_each(range.begin(), range.end(), [&](clTreeCtrlNode* p) {
                    itemsToSelect.push_back({ wxTreeItemId(p), true });
                });
                m_model.SelectItems(itemsToSelect);
            } else {
                // The default, single selection
                bool select_it = !pNode->IsSelected(); // Toggle is the default action
                bool force_selection = false;
                if(m_model.GetSelectionsCount() > 1) {
                    // Unless multiple selections, in that case, select it
                    select_it = true;
                    force_selection = true;
                }
                m_model.SelectItem(where, select_it, false, force_selection);
            }
        }
        Refresh();
    }
}

wxTreeItemId clTreeCtrl::HitTest(const wxPoint& point, int& flags) const
{
    if(!m_model.GetRoot()) {
        return wxTreeItemId();
    }

    flags = 0;
    for(size_t i = 0; i < m_model.GetOnScreenItems().size(); ++i) {
        const clTreeCtrlNode* item = m_model.GetOnScreenItems()[i];
        if(item->GetButtonRect().Contains(point)) {
            flags |= wxTREE_HITTEST_ONITEMBUTTON;
            return wxTreeItemId(const_cast<clTreeCtrlNode*>(item));
        }
        if(item->GetItemRect().Contains(point)) {
            return wxTreeItemId(const_cast<clTreeCtrlNode*>(item));
        }
    }
    return wxTreeItemId();
}

void clTreeCtrl::UnselectAll()
{
    if(!m_model.GetRoot()) {
        return;
    }
    m_model.UnselectAll();
    Refresh();
}

wxPoint clTreeCtrl::DoFixPoint(const wxPoint& pt)
{
    wxPoint point = pt;
    return point;
}

void clTreeCtrl::EnsureVisible(const wxTreeItemId& item)
{
    if(!item.IsOk()) {
        return;
    }
    // Make sure that all parents of ítem are expanded
    if(!m_model.ExpandToItem(item)) {
        return;
    }
    Refresh();
    CallAfter(&clTreeCtrl::DoEnsureVisible, item);
}

void clTreeCtrl::DoEnsureVisible(const wxTreeItemId& item)
{
    // scroll to the item
    if(!item.IsOk()) {
        return;
    }
    clTreeCtrlNode* pNode = m_model.ToPtr(item);
    if(IsItemVisible(pNode)) {
        return;
    }
    EnsureItemVisible(pNode, false); // make it visible at the bottom
    Refresh();
}

void clTreeCtrl::OnMouseLeftDClick(wxMouseEvent& event)
{
    event.Skip();
    CHECK_ROOT_RET();

    int flags = 0;
    wxPoint pt = DoFixPoint(event.GetPosition());
    wxTreeItemId where = HitTest(pt, flags);
    if(where.IsOk()) {
        SelectItem(where, true);

        // Let sublclasses handle this first
        wxTreeEvent evt(wxEVT_TREE_ITEM_ACTIVATED);
        evt.SetEventObject(this);
        evt.SetItem(where);
        if(GetEventHandler()->ProcessEvent(evt)) {
            return;
        }

        // Process the default action
        if(ItemHasChildren(where)) {
            if(IsExpanded(where)) {
                Collapse(where);
            } else {
                Expand(where);
            }
        }
    }
}

bool clTreeCtrl::IsExpanded(const wxTreeItemId& item) const
{
    if(!item.GetID()) return false;
    clTreeCtrlNode* child = m_model.ToPtr(item);
    if(!child) return false;
    return child->IsExpanded();
}

bool clTreeCtrl::ItemHasChildren(const wxTreeItemId& item) const
{
    if(!item.GetID()) return false;
    clTreeCtrlNode* child = m_model.ToPtr(item);
    if(!child) return false;
    return child->HasChildren();
}

void clTreeCtrl::SetIndent(int size)
{
    m_model.SetIndentSize(size);
    Refresh();
}

int clTreeCtrl::GetIndent() const { return m_model.GetIndentSize(); }

bool clTreeCtrl::IsEmpty() const { return m_model.IsEmpty(); }

size_t clTreeCtrl::GetChildrenCount(const wxTreeItemId& item, bool recursively) const
{
    if(!item.GetID()) return 0;
    clTreeCtrlNode* node = m_model.ToPtr(item);
    return node->GetChildrenCount(recursively);
}

void clTreeCtrl::DeleteChildren(const wxTreeItemId& item)
{
    if(!item.GetID()) return;
    clTreeCtrlNode* node = m_model.ToPtr(item);
    node->DeleteAllChildren();
}

wxTreeItemId clTreeCtrl::GetFirstChild(const wxTreeItemId& item, clTreeItemIdValue& cookie) const
{
    if(!item.GetID()) return wxTreeItemId();
    clTreeCtrlNode* node = m_model.ToPtr(item);
    const clTreeCtrlNode::Vec_t& children = node->GetChildren();
    if(children.empty()) return wxTreeItemId(); // No children
    cookie.nextItem = 1;                        // the next item
    return wxTreeItemId(children[0]);
}

wxTreeItemId clTreeCtrl::GetNextChild(const wxTreeItemId& item, clTreeItemIdValue& cookie) const
{
    if(!item.GetID()) return wxTreeItemId();
    clTreeCtrlNode* node = m_model.ToPtr(item);
    const clTreeCtrlNode::Vec_t& children = node->GetChildren();
    if((int)children.size() >= cookie.nextItem) return wxTreeItemId();
    wxTreeItemId child(children[cookie.nextItem]);
    cookie.nextItem++;
    return child;
}

wxString clTreeCtrl::GetItemText(const wxTreeItemId& item) const
{
    if(!item.GetID()) return "";
    clTreeCtrlNode* node = m_model.ToPtr(item);
    return node->GetLabel();
}

wxTreeItemData* clTreeCtrl::GetItemData(const wxTreeItemId& item) const
{
    if(!item.GetID()) return nullptr;
    clTreeCtrlNode* node = m_model.ToPtr(item);
    return node->GetClientObject();
}

void clTreeCtrl::OnMouseScroll(wxMouseEvent& event)
{
    CHECK_ROOT_RET();
    if(!GetFirstItemOnScreen()) {
        return;
    }
    const clTreeCtrlNode::Vec_t& onScreenItems = m_model.GetOnScreenItems();
    if(onScreenItems.empty()) {
        return;
    }
    clTreeCtrlNode* lastItem = onScreenItems.back();
    clTreeCtrlNode* firstItem = onScreenItems.front();

    // Can we scroll any further?
    wxTreeItemId nextItem;
    if(event.GetWheelRotation() > 0) { // Scrolling up
        nextItem = m_model.GetItemAfter(firstItem, true);
    } else {
        nextItem = m_model.GetItemAfter(lastItem, true);
    }
    if(!nextItem.IsOk()) {
        // No more items to draw
        return;
    }
    clTreeCtrlNode::Vec_t items;
    if(event.GetWheelRotation() > 0) { // Scrolling up
        m_model.GetPrevItems(GetFirstItemOnScreen(), m_scrollTick, items);
        SetFirstItemOnScreen(items.front()); // first item
    } else {
        m_model.GetNextItems(GetFirstItemOnScreen(), m_scrollTick, items);
        SetFirstItemOnScreen(items.back()); // the last item
    }
    Refresh();
}

const wxBitmap& clTreeCtrl::GetBitmap(size_t index) const
{
    if(index >= m_bitmaps.size()) {
        static wxBitmap emptyBitmap;
        return emptyBitmap;
    }
    return m_bitmaps[index];
}

void clTreeCtrl::SetBitmaps(const std::vector<wxBitmap>& bitmaps)
{
    m_bitmaps = bitmaps;
    int heighestBitmap = 0;
    for(size_t i = 0; i < m_bitmaps.size(); ++i) {
        heighestBitmap = wxMax(heighestBitmap, m_bitmaps[i].GetScaledHeight());
    }
    heighestBitmap += 2 * clTreeCtrlNode::Y_SPACER;
    m_lineHeight = wxMax(heighestBitmap, m_lineHeight);
    SetIndent(m_lineHeight);
    Refresh();
}

void clTreeCtrl::OnIdle(wxIdleEvent& event)
{
    CHECK_ROOT_RET();
    int flags = 0;
    wxPoint pt = ScreenToClient(::wxGetMousePosition());
    wxTreeItemId item = HitTest(pt, flags);
    if(item.IsOk()) {
        clTreeCtrlNode::Vec_t& items = m_model.GetOnScreenItems();
        clTreeCtrlNode* hoveredNode = m_model.ToPtr(item);
        bool refreshNeeded = false;
        for(size_t i = 0; i < items.size(); ++i) {
            bool new_state = hoveredNode == items[i];
            bool old_state = items[i]->IsHovered();
            if(!refreshNeeded) {
                refreshNeeded = (new_state != old_state);
            }
            items[i]->SetHovered(hoveredNode == items[i] && !HasCapture());
        }
        if(refreshNeeded) {
            Refresh();
        }
    }
}

void clTreeCtrl::OnLeaveWindow(wxMouseEvent& event)
{
    event.Skip();
    CHECK_ROOT_RET();
    clTreeCtrlNode::Vec_t& items = m_model.GetOnScreenItems();
    for(size_t i = 0; i < items.size(); ++i) {
        items[i]->SetHovered(false);
    }
    Refresh();
}

void clTreeCtrl::SetColours(const clTreeCtrlColours& colours)
{
    m_colours = colours;
    Refresh();
}

void clTreeCtrl::ExpandAllChildren(const wxTreeItemId& item)
{
    wxBusyCursor bc;
    m_model.ExpandAllChildren(item);
    Refresh();
}

void clTreeCtrl::CollapseAllChildren(const wxTreeItemId& item)
{
    wxBusyCursor bc;
    m_model.CollapseAllChildren(item);
    SetFirstItemOnScreen(m_model.ToPtr(item));
    SelectItem(item);
    Refresh();
}

wxTreeItemId clTreeCtrl::GetFirstVisibleItem() const
{
    const clTreeCtrlNode::Vec_t& items = m_model.GetOnScreenItems();
    if(items.empty()) {
        return wxTreeItemId();
    }
    return wxTreeItemId(items[0]);
}

wxTreeItemId clTreeCtrl::GetNextVisible(const wxTreeItemId& item) const { return DoGetSiblingVisibleItem(item, true); }
wxTreeItemId clTreeCtrl::GetPrevVisible(const wxTreeItemId& item) const { return DoGetSiblingVisibleItem(item, false); }

wxTreeItemId clTreeCtrl::DoGetSiblingVisibleItem(const wxTreeItemId& item, bool next) const
{
    if(!item.IsOk()) {
        return wxTreeItemId();
    }
    const clTreeCtrlNode::Vec_t& items = m_model.GetOnScreenItems();
    if(items.empty()) {
        return wxTreeItemId();
    }
    clTreeCtrlNode* from = m_model.ToPtr(item);
    clTreeCtrlNode::Vec_t::const_iterator iter =
        std::find_if(items.begin(), items.end(), [&](clTreeCtrlNode* p) { return p == from; });
    if(next && (iter == items.end())) {
        return wxTreeItemId();
    }
    if(!next && (iter == items.begin())) {
        return wxTreeItemId();
    }
    if(next) {
        ++iter;
        if(iter == items.end()) {
            return wxTreeItemId();
        }
    } else {
        --iter;
        if(iter == items.begin()) {
            return wxTreeItemId();
        }
    }
    return wxTreeItemId(*iter);
}

wxTreeItemId clTreeCtrl::GetSelection() const { return m_model.GetSingleSelection(); }

wxTreeItemId clTreeCtrl::GetFocusedItem() const { return GetSelection(); }

size_t clTreeCtrl::GetSelections(wxArrayTreeItemIds& selections) const
{
    const clTreeCtrlNode::Vec_t& items = m_model.GetSelections();
    if(items.empty()) {
        return 0;
    }
    std::for_each(items.begin(), items.end(), [&](clTreeCtrlNode* item) { selections.Add(wxTreeItemId(item)); });
    return selections.size();
}

void clTreeCtrl::OnKeyDown(wxKeyEvent& event)
{
    CHECK_ROOT_RET();
    wxTreeItemId selectedItem = GetSelection();
    if(!selectedItem.IsOk()) {
        return;
    }

    // Let the user chance to process this first
    wxTreeEvent evt(wxEVT_TREE_KEY_DOWN);
    evt.SetEventObject(this);
    evt.SetKeyEvent(event);
    evt.SetItem(selectedItem);
    if(GetEventHandler()->ProcessEvent(evt)) {
        return;
    }

    if(event.GetKeyCode() == WXK_UP) {
        selectedItem = m_model.GetItemBefore(selectedItem, true);
        if(selectedItem.IsOk()) {
            SelectItem(selectedItem);
            EnsureItemVisible(m_model.ToPtr(selectedItem), true);
        }
    } else if(event.GetKeyCode() == WXK_DOWN) {
        selectedItem = m_model.GetItemAfter(selectedItem, true);
        if(selectedItem.IsOk()) {
            SelectItem(selectedItem);
            EnsureItemVisible(m_model.ToPtr(selectedItem), false);
        }
    } else if(event.GetKeyCode() == WXK_PAGEDOWN) {
        clTreeCtrlNode::Vec_t items;
        m_model.GetNextItems(m_model.ToPtr(selectedItem), GetNumLineCanFitOnScreen(), items);
        if(!items.empty()) {
            selectedItem = wxTreeItemId(items.back());
            SelectItem(selectedItem);
            EnsureItemVisible(m_model.ToPtr(selectedItem), false);
        }
    } else if(event.GetKeyCode() == WXK_PAGEUP) {
        clTreeCtrlNode::Vec_t items;
        m_model.GetPrevItems(m_model.ToPtr(selectedItem), GetNumLineCanFitOnScreen(), items);
        if(!items.empty()) {
            selectedItem = wxTreeItemId(items[0]);
            SelectItem(selectedItem);
            EnsureItemVisible(m_model.ToPtr(selectedItem), true);
        }
    } else if(event.GetKeyCode() == WXK_LEFT) {
        if(m_model.ToPtr(selectedItem)->IsExpanded()) {
            Collapse(selectedItem);
        }
    } else if(event.GetKeyCode() == WXK_RIGHT) {
        if(!m_model.ToPtr(selectedItem)->IsExpanded()) {
            Expand(selectedItem);
        }
    } else if(event.GetKeyCode() == WXK_NUMPAD_DELETE || event.GetKeyCode() == WXK_DELETE) {
        // Delete the item (this will also fire
        // wxEVT_TREE_DELETE_ITEM
        Delete(selectedItem);
    } else if(event.GetKeyCode() == WXK_RETURN || event.GetKeyCode() == WXK_NUMPAD_ENTER) {
        wxTreeEvent evt(wxEVT_TREE_ITEM_ACTIVATED);
        evt.SetEventObject(this);
        evt.SetItem(selectedItem);
        GetEventHandler()->ProcessEvent(evt);
    }
}

bool clTreeCtrl::IsItemVisible(clTreeCtrlNode* item) const
{
    const clTreeCtrlNode::Vec_t& onScreenItems = m_model.GetOnScreenItems();
    return (std::find_if(onScreenItems.begin(), onScreenItems.end(), [&](clTreeCtrlNode* p) { return p == item; }) !=
        onScreenItems.end());
}

void clTreeCtrl::EnsureItemVisible(clTreeCtrlNode* item, bool fromTop)
{
    if(IsItemVisible(item)) {
        return;
    }
    if(fromTop) {
        SetFirstItemOnScreen(item);
    } else {
        int max_lines_on_screen = GetNumLineCanFitOnScreen();
        clTreeCtrlNode::Vec_t items;
        m_model.GetPrevItems(item, max_lines_on_screen, items);
        if(items.empty()) {
            return;
        }
        SetFirstItemOnScreen(items[0]);
    }
}

int clTreeCtrl::GetNumLineCanFitOnScreen() const
{
    wxRect clientRect = GetClientRect();
    int max_lines_on_screen = ceil(clientRect.GetHeight() / m_lineHeight);
    return max_lines_on_screen;
}

void clTreeCtrl::Delete(const wxTreeItemId& item)
{
    // delete the item + its children
    // fires event
    m_model.DeleteItem(item);
    Refresh();
}

void clTreeCtrl::SetItemData(const wxTreeItemId& item, wxTreeItemData* data)
{
    clTreeCtrlNode* node = m_model.ToPtr(item);
    CHECK_PTR_RET(node);
    node->SetClientData(data);
}

void clTreeCtrl::SetItemBackgroundColour(const wxTreeItemId& item, const wxColour& colour)
{
    clTreeCtrlNode* node = m_model.ToPtr(item);
    CHECK_PTR_RET(node);
    node->SetBgColour(colour);
    Refresh();
}

wxColour clTreeCtrl::GetItemBackgroudColour(const wxTreeItemId& item) const
{
    clTreeCtrlNode* node = m_model.ToPtr(item);
    if(!node) {
        return wxNullColour;
    }
    return node->GetBgColour();
}

void clTreeCtrl::SetItemTextColour(const wxTreeItemId& item, const wxColour& colour)
{
    clTreeCtrlNode* node = m_model.ToPtr(item);
    CHECK_PTR_RET(node);
    node->SetTextColour(colour);
    Refresh();
}

wxColour clTreeCtrl::GetItemTextColour(const wxTreeItemId& item) const
{
    clTreeCtrlNode* node = m_model.ToPtr(item);
    if(!node) {
        return wxNullColour;
    }
    return node->GetTextColour();
}

void clTreeCtrl::SetItemText(const wxTreeItemId& item, const wxString& text)
{
    clTreeCtrlNode* node = m_model.ToPtr(item);
    CHECK_PTR_RET(node);
    node->SetLabel(text);
    Refresh();
}

void clTreeCtrl::SetItemBold(const wxTreeItemId& item, bool bold)
{
    clTreeCtrlNode* node = m_model.ToPtr(item);
    CHECK_PTR_RET(node);
    wxFont f = node->GetFont();
    if(!f.IsOk()) {
        f = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    }
    f.SetWeight(bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
    node->SetFont(f);
    Refresh();
}

void clTreeCtrl::SetItemFont(const wxTreeItemId& item, const wxFont& font)
{
    clTreeCtrlNode* node = m_model.ToPtr(item);
    CHECK_PTR_RET(node);
    node->SetFont(font);
    Refresh();
}

wxFont clTreeCtrl::GetItemFont(const wxTreeItemId& item) const
{
    clTreeCtrlNode* node = m_model.ToPtr(item);
    if(!node) {
        return wxNullFont;
    }
    return node->GetFont();
}

void clTreeCtrl::OnContextMenu(wxContextMenuEvent& event)
{
    event.Skip();
    CHECK_ROOT_RET();
    int flags = 0;
    wxPoint pt = ScreenToClient(::wxGetMousePosition());
    wxTreeItemId item = HitTest(pt, flags);
    if(item.IsOk()) {
        SelectItem(item, true);
        wxTreeEvent evt(wxEVT_TREE_ITEM_MENU);
        evt.SetItem(item);
        evt.SetEventObject(this);
        GetEventHandler()->ProcessEvent(evt);
    }
}

void clTreeCtrl::OnRightDown(wxMouseEvent& event)
{
    event.Skip();
    CHECK_ROOT_RET();
    int flags = 0;
    wxPoint pt = DoFixPoint(event.GetPosition());
    wxTreeItemId where = HitTest(pt, flags);
    if(where.IsOk()) {
        wxTreeEvent evt(wxEVT_TREE_ITEM_RIGHT_CLICK);
        evt.SetEventObject(this);
        evt.SetItem(where);
        event.Skip(false);
        if(GetEventHandler()->ProcessEvent(evt)) {
            return;
        }
        event.Skip();
    }
}

clTreeCtrlNode* clTreeCtrl::GetFirstItemOnScreen() { return m_model.GetFirstItemOnScreen(); }

void clTreeCtrl::SetFirstItemOnScreen(clTreeCtrlNode* item) { m_model.SetFirstItemOnScreen(item); }

void clTreeCtrl::SetSortFunction(const std::function<bool(const wxTreeItemId&, const wxTreeItemId&)>& CompareFunc)
{
    m_model.SetSortFunction(CompareFunc);
}

void clTreeCtrl::UpdateScrollBar(wxDC& dc)
{
    wxRect rect = GetClientRect();
    int position = m_model.GetItemIndex(m_model.GetFirstItemOnScreen());
    m_scrollBar->SetScrollbar(position, rect.GetHeight() / m_lineHeight, m_model.GetExpandedLines(), m_lineHeight);
    m_scrollBar->Render(dc);
}

void clTreeCtrl::OnScroll(wxScrollEvent& event)
{
    int lines = event.GetPosition();
    if(lines == 0) {
        return;
    }

    wxTreeItemId cur(GetFirstVisibleItem());
    int counter = 0;
    int max_items = abs(lines);
    if(lines < 0) {
        // Moving up
        while(cur.IsOk()) {
            wxTreeItemId itemBefore = m_model.GetItemBefore(cur, true);
            if(!itemBefore.IsOk()) {
                break;
            }
            cur = itemBefore;
            counter++;
            if(max_items == counter) {
                break;
            }
        }
    } else {
        // Moving down
        while(cur.IsOk()) {
            wxTreeItemId itemAfter = m_model.GetItemAfter(cur, true);
            if(!itemAfter.IsOk()) {
                break;
            }
            cur = itemAfter;
            counter++;
            if(max_items == counter) {
                break;
            }
        }
    }
    if(!cur.IsOk()) {
        return;
    }
    SetFirstItemOnScreen(m_model.ToPtr(cur));
    Refresh();
}
