#include "clTreeCtrl.h"
#include "clTreeCtrlModel.h"
#include "clTreeNodeVisitor.h"
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
    m_root->SetLabel(text);
    m_root->SetBitmapIndex(image);
    m_root->SetBitmapSelectedIndex(selImage);
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
    for(size_t i = 0; i < m_onScreenItems.size(); ++i) { m_onScreenItems[i]->ClearRects(); }
    m_onScreenItems.clear();
}

void clTreeCtrlModel::SetOnScreenItems(const std::vector<clTreeCtrlNode*>& items)
{
    // Clear the old visible items. But only, if the item does not appear in both lists
    for(size_t i = 0; i < m_onScreenItems.size(); ++i) {
        clTreeCtrlNode* visibleItem = m_onScreenItems[i];
        std::vector<clTreeCtrlNode*>::const_iterator iter
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
        clTreeCtrlNode::Ptr_t child(new clTreeCtrlNode(m_tree, text, image, selImage));
        child->SetClientData(data);
        parentNode->AddChild(child);
        return wxTreeItemId(child.get());
    }
    return wxTreeItemId(nullptr);
}
