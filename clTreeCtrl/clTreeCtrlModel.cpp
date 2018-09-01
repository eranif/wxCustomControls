#include "clTreeCtrl.h"
#include "clTreeCtrlModel.h"
#include <algorithm>
#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/treebase.h>

#define INDENT_SIZE 16

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

clTreeCtrlModel::clTreeCtrlModel(clTreeCtrl* tree)
    : m_tree(tree)
{
}

clTreeCtrlModel::~clTreeCtrlModel() { wxDELETE(m_root); }

void clTreeCtrlModel::GetNextItems(clTreeCtrlNode* from, int count, clTreeCtrlNode::Vec_t& items) const
{
    return from->GetNextItems(count, items);
}

void clTreeCtrlModel::GetPrevItems(clTreeCtrlNode* from, int count, clTreeCtrlNode::Vec_t& items) const
{
    return from->GetPrevItems(count, items);
}

wxTreeItemId clTreeCtrlModel::AddRoot(const wxString& text, int image, int selImage, wxTreeItemData* data)
{
    if(m_root) { return wxTreeItemId(m_root); }
    m_root = new clTreeCtrlNode(m_tree);
    m_root->SetLabel(text);
    m_root->SetBitmapIndex(image);
    m_root->SetBitmapSelectedIndex(selImage);
    m_root->SetClientData(data);
    return wxTreeItemId(m_root);
}

wxTreeItemId clTreeCtrlModel::GetRootItem() const
{
    if(!m_root) { return wxTreeItemId(); }
    return wxTreeItemId(const_cast<clTreeCtrlNode*>(m_root));
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

    if(m_tree->GetTreeStyle() & wxTR_MULTIPLE) {
        // If we are unselecting it, remove it from the array
        clTreeCtrlNode::Vec_t::iterator iter = std::find_if(
            m_selectedItems.begin(), m_selectedItems.end(), [&](clTreeCtrlNode* p) { return (p == child); });
        if(iter != m_selectedItems.end() && !select) { m_selectedItems.erase(iter); }
    } else {
        UnselectAll();
    }
    child->SetSelected(select);
    if(select) { m_selectedItems.push_back(child); }
}

void clTreeCtrlModel::Clear()
{
    m_selectedItems.clear();
    m_nVisibleLines = wxNOT_FOUND;
    for(size_t i = 0; i < m_onScreenItems.size(); ++i) { m_onScreenItems[i]->ClearRects(); }
    m_onScreenItems.clear();
}

void clTreeCtrlModel::SetOnScreenItems(const clTreeCtrlNode::Vec_t& items)
{
    // Clear the old visible items. But only, if the item does not appear in both lists
    for(size_t i = 0; i < m_onScreenItems.size(); ++i) {
        clTreeCtrlNode* visibleItem = m_onScreenItems[i];
        clTreeCtrlNode::Vec_t::const_iterator iter
            = std::find_if(items.begin(), items.end(), [&](clTreeCtrlNode* item) { return item == visibleItem; });
        if(iter == items.end()) { m_onScreenItems[i]->ClearRects(); }
    }
    m_onScreenItems = items;
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

wxTreeItemId clTreeCtrlModel::AppendItem(
    const wxTreeItemId& parent, const wxString& text, int image, int selImage, wxTreeItemData* data)
{
    clTreeCtrlNode* parentNode = nullptr;
    if(parent.IsOk()) { parentNode = reinterpret_cast<clTreeCtrlNode*>(parent.GetID()); }

    if(parentNode) {
        clTreeCtrlNode* child = new clTreeCtrlNode(m_tree, text, image, selImage);
        child->SetClientData(data);
        parentNode->AddChild(child);
        return wxTreeItemId(child);
    }
    return wxTreeItemId(nullptr);
}

void clTreeCtrlModel::ExpandAllChildren(const wxTreeItemId& item) { DoExpandAllChildren(item, true); }

void clTreeCtrlModel::CollapseAllChildren(const wxTreeItemId& item) { DoExpandAllChildren(item, false); }

void clTreeCtrlModel::DoExpandAllChildren(const wxTreeItemId& item, bool expand)
{
    clTreeCtrlNode* p = ToPtr(item);
    if(!p) { return; }
    while(p) {
        if(p->HasChildren()) {
            if(expand && !p->IsExpanded()) {
                p->SetExpanded(true);
            } else if(!expand && p->IsExpanded()) {
                p->SetExpanded(false);
            }
        }
        p = p->GetNext();
    }
    StateModified();
}

wxTreeItemId clTreeCtrlModel::GetItemBefore(const wxTreeItemId& item, bool visibleItem) const
{
    clTreeCtrlNode* p = ToPtr(item);
    if(!p) { return wxTreeItemId(); }
    if(visibleItem) {
        clTreeCtrlNode::Vec_t items;
        GetPrevItems(p, 2, items);
        if(items.size() != 2) { return wxTreeItemId(); }
        return wxTreeItemId(items[0]);
    } else {
        return wxTreeItemId(p->GetPrev());
    }
}

wxTreeItemId clTreeCtrlModel::GetItemAfter(const wxTreeItemId& item, bool visibleItem) const
{
    clTreeCtrlNode* p = ToPtr(item);
    if(!p) { return wxTreeItemId(); }
    if(visibleItem) {
        clTreeCtrlNode::Vec_t items;
        GetNextItems(p, 2, items);
        if(items.size() != 2) { return wxTreeItemId(); }
        return wxTreeItemId(items[1]);
    } else {
        return wxTreeItemId(p->GetNext());
    }
}
