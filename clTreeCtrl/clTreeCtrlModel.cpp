#include "clTreeCtrl.h"
#include "clTreeCtrlModel.h"
#include "clTreeNodeVisitor.h"
#include <algorithm>
#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/treebase.h>

#define INDENT_SIZE 16

clTreeCtrlNode::clTreeCtrlNode(clTreeCtrl* tree)
    : m_tree(tree)
{
}

clTreeCtrlNode::clTreeCtrlNode(clTreeCtrl* tree, const wxString& label, int bitmapIndex)
    : m_tree(tree)
    , m_label(label)
    , m_bitmapIndex(bitmapIndex)
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
    std::function<bool(clTreeCtrlNode*)> pCounterFunc = [&](clTreeCtrlNode* item) {
        counter++;
        return true;
    };
    clTreeNodeVisitor V;
    V.Visit(const_cast<clTreeCtrlNode*>(this), true, pCounterFunc);
    return counter;
}

void clTreeCtrlNode::GetItemsFromIndex(int start, int count, std::vector<clTreeCtrlNode*>& items)
{
    // find the first entry
    clTreeCtrlNode* startItem = GetVisibleItem(start);
    if(!startItem) return;

    std::function<bool(clTreeCtrlNode*)> pFuncStopCond = [&](clTreeCtrlNode* item) {
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
    std::function<bool(clTreeCtrlNode*)> pFuncStopCond = [&](clTreeCtrlNode* item) {
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
    std::function<bool(clTreeCtrlNode*)> pUnselectItem = [&](clTreeCtrlNode* item) {
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
    std::function<bool(clTreeCtrlNode*)> pCounterFunc = [&](clTreeCtrlNode* p) {
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
    if(IsSelected()) {
        dc.SetBrush(colours.selItemBgColour);
        dc.SetPen(colours.selItemBgColour);
        dc.DrawRoundedRectangle(itemRect, 1.5);
    }
    int textY = itemRect.GetY() + clTreeCtrlNode::Y_SPACER;
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
    }

    dc.SetTextForeground(IsSelected() ? colours.selItemTextColour : colours.textColour);
    dc.DrawText(GetLabel(), (GetIndentsCount() * m_tree->GetIndent()) + textXOffset, textY);
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

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

clTreeCtrlModel::clTreeCtrlModel(clTreeCtrl* tree)
    : m_tree(tree)
{
}

clTreeCtrlModel::~clTreeCtrlModel() { m_root.reset(nullptr); }

void clTreeCtrlModel::GetItemsFromIndex(int start, int count, std::vector<clTreeCtrlNode*>& items)
{
    if(!m_root) return;
    m_root->GetItemsFromIndex(start, count, items);
}

wxTreeItemId clTreeCtrlModel::AddRoot(const wxString& text, int image, int selImage, wxTreeItemData* data)
{
    if(m_root) { return wxTreeItemId(m_root.get()); }
    m_root.reset(new clTreeCtrlNode(m_tree));
    wxUnusedVar(selImage);
    m_root->SetLabel(text);
    m_root->SetBitmapIndex(image);
    m_root->SetClientData(data);
    return wxTreeItemId(m_root.get());
}

wxTreeItemId clTreeCtrlModel::GetRootItem() const
{
    if(!m_root) { return wxTreeItemId(); }
    return wxTreeItemId(const_cast<clTreeCtrlNode*>(m_root.get()));
}

int clTreeCtrlModel::GetExpandedLines()
{
    if(!m_root) { return 0; }
    if(m_nVisibleLines == wxNOT_FOUND) { m_nVisibleLines = m_root->GetExpandedLines(); }
    return m_nVisibleLines;
}

void clTreeCtrlModel::StateModified() { m_nVisibleLines = wxNOT_FOUND; }

void clTreeCtrlModel::UnselectAll()
{
    for(size_t i = 0; i < m_selectedItems.size(); ++i) { m_selectedItems[i]->SetSelected(false); }
    m_selectedItems.clear();
}

void clTreeCtrlModel::SelectItem(const wxTreeItemId& item, bool select)
{
    clTreeCtrlNode* child = reinterpret_cast<clTreeCtrlNode*>(item.GetID());
    if(!child) return;
    child->SetSelected(select);

    // If we are unselecting it, remove it from the array
    std::vector<clTreeCtrlNode*>::iterator iter
        = std::find_if(m_selectedItems.begin(), m_selectedItems.end(), [&](clTreeCtrlNode* p) { return (p == child); });
    if(iter != m_selectedItems.end() && !select) { m_selectedItems.erase(iter); }

    if(select) { m_selectedItems.push_back(child); }
}

void clTreeCtrlModel::Clear()
{
    m_selectedItems.clear();
    m_nVisibleLines = wxNOT_FOUND;
    for(size_t i = 0; i < m_visibleItems.size(); ++i) { m_visibleItems[i]->ClearRects(); }
    m_visibleItems.clear();
}

void clTreeCtrlModel::SetVisibleItems(const std::vector<clTreeCtrlNode*>& items)
{
    // Clear the old visible items. But only, if the item does not appear in both lists
    for(size_t i = 0; i < m_visibleItems.size(); ++i) {
        clTreeCtrlNode* visibleItem = m_visibleItems[i];
        std::vector<clTreeCtrlNode*>::const_iterator iter
            = std::find_if(items.begin(), items.end(), [&](clTreeCtrlNode* item) { return item == visibleItem; });
        if(iter == items.end()) { m_visibleItems[i]->ClearRects(); }
    }
    m_visibleItems = items;
}

bool clTreeCtrlModel::ExpandToItem(const wxTreeItemId& item)
{
    clTreeCtrlNode* child = reinterpret_cast<clTreeCtrlNode*>(item.GetID());
    if(!child) { return false; }

    clTreeCtrlNode* parent = child->GetParent();
    while(parent) {
        if(!parent->SetExpanded(true)) { return false; }
        parent = parent->GetParent();
    }
    StateModified();
    return true;
}

int clTreeCtrlModel::GetItemIndex(const wxTreeItemId& item, bool visibleItemsOnly) const
{
    if(!m_root) { return wxNOT_FOUND; }
    clTreeCtrlNode* child = reinterpret_cast<clTreeCtrlNode*>(item.GetID());
    if(!child) return wxNOT_FOUND;
    return m_root->GetItemIndex(child, visibleItemsOnly);
}

wxTreeItemId clTreeCtrlModel::AppendItem(
    const wxTreeItemId& parent, const wxString& text, int image, int selImage, wxTreeItemData* data)
{
    clTreeCtrlNode* parentNode = nullptr;
    if(parent.IsOk()) { parentNode = reinterpret_cast<clTreeCtrlNode*>(parent.GetID()); }

    if(parentNode) {
        clTreeCtrlNode::Ptr_t child(new clTreeCtrlNode(m_tree, text, image));
        child->SetClientData(data);
        parentNode->AddChild(child);
        return wxTreeItemId(child.get());
    }
    return wxTreeItemId(nullptr);
}
