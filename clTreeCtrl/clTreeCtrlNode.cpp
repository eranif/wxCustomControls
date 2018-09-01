#include "clTreeCtrl.h"
#include "clTreeCtrlNode.h"
#include "clTreeNodeVisitor.h"
#include <functional>
#include <wx/dc.h>

clTreeCtrlNode::clTreeCtrlNode(clTreeCtrl* tree)
    : m_tree(tree)
{
}

clTreeCtrlNode::clTreeCtrlNode(clTreeCtrl* tree, const wxString& label, int bitmapIndex, int bitmapSelectedIndex)
    : m_tree(tree)
    , m_label(label)
    , m_bitmapIndex(bitmapIndex)
    , m_bitmapSelectedIndex(bitmapSelectedIndex)
{
}

clTreeCtrlNode::~clTreeCtrlNode() {}

void clTreeCtrlNode::AddChild(clTreeCtrlNode::Ptr_t child)
{
    child->SetParent(this);
    child->SetIndentsCount(GetIndentsCount() + 1);
    m_children.push_back(child);
    if(HasFlag(kSortItems)) {
        // Sort the items
    }
}

void clTreeCtrlNode::SetParent(clTreeCtrlNode* parent)
{
    if(m_parent) { m_parent->RemoveChild(this); }
    m_parent = parent;
}

void clTreeCtrlNode::RemoveChild(clTreeCtrlNode* child)
{
    std::vector<clTreeCtrlNode::Ptr_t>::iterator iter = std::find_if(
        m_children.begin(), m_children.end(), [&](clTreeCtrlNode::Ptr_t node) { return node.get() == child; });
    if(iter != m_children.end()) { m_children.erase(iter); }
}

int clTreeCtrlNode::GetExpandedLines() const
{
    int counter = 0;
    std::function<bool(clTreeCtrlNode*, bool)> pCounterFunc = [&](clTreeCtrlNode* item, bool visible) {
        wxUnusedVar(visible);
        counter++;
        return true;
    };
    clTreeNodeVisitor V;
    V.Visit(const_cast<clTreeCtrlNode*>(this), true, pCounterFunc);
    return counter;
}

void clTreeCtrlNode::GetItemsFromIndex(int start, int count, clTreeCtrlNode::Vec_t& items)
{
    // find the first entry
    clTreeCtrlNode* startItem = GetVisibleItem(start);
    if(!startItem) return;

    std::function<bool(clTreeCtrlNode*, bool)> pFuncStopCond = [&](clTreeCtrlNode* item, bool visible) {
        wxUnusedVar(visible);
        items.push_back(item);
        if((int)items.size() == count) return false;
        return true;
    };
    clTreeNodeVisitor V;
    V.Visit(startItem, true, pFuncStopCond);
}

clTreeCtrlNode* clTreeCtrlNode::GetVisibleItem(int index)
{
    int counter = -1;
    clTreeCtrlNode* pMatch = nullptr;
    std::function<bool(clTreeCtrlNode*, bool)> pFuncStopCond = [&](clTreeCtrlNode* item, bool visible) {
        wxUnusedVar(visible);
        ++counter;
        if(counter == index) {
            pMatch = item;
            return false;
        }
        return true;
    };
    clTreeNodeVisitor V;
    V.Visit(this, true, pFuncStopCond);
    return pMatch;
}

void clTreeCtrlNode::UnselectAll()
{
    std::function<bool(clTreeCtrlNode*, bool)> pUnselectItem = [&](clTreeCtrlNode* item, bool visible) {
        wxUnusedVar(visible);
        item->SetSelected(false);
        return true;
    };
    clTreeNodeVisitor V;
    V.Visit(this, false, pUnselectItem);
}

int clTreeCtrlNode::GetItemIndex(clTreeCtrlNode* item, bool onlyExpandedItems) const
{
    int index = wxNOT_FOUND;
    int where = wxNOT_FOUND;
    std::function<bool(clTreeCtrlNode*, bool)> pCounterFunc = [&](clTreeCtrlNode* p, bool visible) {
        ++index;
        if(p == item) {
            where = index;
            return false;
        }
        return true;
    };
    clTreeNodeVisitor V;
    V.Visit(const_cast<clTreeCtrlNode*>(this), onlyExpandedItems, pCounterFunc);
    return where;
}

bool clTreeCtrlNode::SetExpanded(bool b)
{
    // Already expanded?
    if(b && IsExpanded()) { return true; }
    // Already collapsed?
    if(!b && !IsExpanded()) { return true; }

    wxTreeEvent before(b ? wxEVT_TREE_ITEM_EXPANDING : wxEVT_TREE_ITEM_COLLAPSING);
    before.SetItem(wxTreeItemId(this));
    before.SetEventObject(m_tree);
    m_tree->GetEventHandler()->ProcessEvent(before);
    if(!before.IsAllowed()) { return false; }

    SetFlag(kExpanded, b);
    wxTreeEvent after(b ? wxEVT_TREE_ITEM_EXPANDED : wxEVT_TREE_ITEM_COLLAPSED);
    after.SetItem(wxTreeItemId(this));
    after.SetEventObject(m_tree);
    m_tree->GetEventHandler()->ProcessEvent(after);
    return true;
}

void clTreeCtrlNode::ClearRects()
{
    m_buttonRect = wxRect();
    m_itemRect = wxRect();
}

void clTreeCtrlNode::Render(wxDC& dc, const clTreeCtrlColours& colours)
{
    wxRect itemRect = GetItemRect();
    if(IsSelected() || IsHovered()) {
        dc.SetBrush(IsSelected() ? colours.selItemBgColour : colours.hoverBgColour);
        dc.SetPen(IsSelected() ? colours.selItemBgColour : colours.hoverBgColour);
        dc.DrawRoundedRectangle(itemRect, 1.5);
    }

    wxSize textSize = dc.GetTextExtent(GetLabel());
    int textY = itemRect.GetY() + (m_tree->GetLineHeight() - textSize.GetHeight()) / 2;
    // Draw the button
    int textXOffset = 0;
    if(HasChildren()) {
        wxPoint pts[3];
        wxRect buttonRect = GetButtonRect();
        textXOffset += buttonRect.GetWidth();
        buttonRect.Deflate((buttonRect.GetWidth() / 3), (buttonRect.GetHeight() / 3));
        if(IsExpanded()) {
            pts[0] = buttonRect.GetTopRight();
            pts[1] = buttonRect.GetBottomRight();
            pts[2] = buttonRect.GetBottomLeft();
            dc.SetBrush(colours.buttonColour);
            dc.SetPen(colours.buttonColour);
            dc.DrawPolygon(3, pts);
        } else {
            pts[0] = buttonRect.GetTopLeft();
            pts[1] = buttonRect.GetBottomLeft();
            pts[2].x = buttonRect.GetRight();
            pts[2].y = (buttonRect.GetY() + (buttonRect.GetHeight() / 2));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.SetPen(colours.buttonColour);
            dc.DrawPolygon(3, pts);
        }
    } else {
        textXOffset += itemRect.GetHeight();
    }

    int itemIndent = (GetIndentsCount() * m_tree->GetIndent());
    int bitmapIndex = GetBitmapIndex();
    if(IsExpanded() && HasChildren() && GetBitmapSelectedIndex() != wxNOT_FOUND) {
        bitmapIndex = GetBitmapSelectedIndex();
    }

    if(bitmapIndex != wxNOT_FOUND) {
        const wxBitmap& bmp = m_tree->GetBitmap(bitmapIndex);
        if(bmp.IsOk()) {
            textXOffset += X_SPACER;
            int bitmapY = itemRect.GetY() + ((itemRect.GetHeight() - bmp.GetScaledHeight()) / 2);
            dc.DrawBitmap(bmp, itemIndent + textXOffset, bitmapY);
            textXOffset += bmp.GetScaledWidth();
            textXOffset += X_SPACER;
        }
    }
    dc.SetTextForeground(IsSelected() ? colours.selItemTextColour : colours.textColour);
    dc.DrawText(GetLabel(), itemIndent + textXOffset, textY);
}

size_t clTreeCtrlNode::GetChildrenCount(bool recurse) const
{
    if(!recurse) {
        return m_children.size();
    } else {
        size_t count = m_children.size();
        for(size_t i = 0; i < count; ++i) { count += m_children[i]->GetChildrenCount(recurse); }
        return count;
    }
}
