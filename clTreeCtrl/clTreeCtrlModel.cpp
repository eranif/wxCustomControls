#include "clTreeCtrl.h"
#include "clTreeCtrlModel.h"
#include <algorithm>
#include <queue>
#include <wx/treebase.h>

#define INDENT_SIZE 16

clTreeCtrlNode::clTreeCtrlNode(wxEvtHandler* sink)
    : m_sink(sink)
{
}

clTreeCtrlNode::clTreeCtrlNode(wxEvtHandler* sink, const wxString& label, int bitmapIndex)
    : m_sink(sink)
    , m_label(label)
    , m_bitmapIndex(bitmapIndex)
{
}

clTreeCtrlNode::~clTreeCtrlNode() {}

void clTreeCtrlNode::AddChild(clTreeCtrlNode::Ptr_t child)
{
    child->SetParent(this);
    child->SetIndent(GetIndent() + INDENT_SIZE);
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
    std::queue<const clTreeCtrlNode*> Q;
    Q.push(this);
    int count = 0;

    // Now count all the visible children, i.e. the non collapsed ones
    while(!Q.empty()) {
        const clTreeCtrlNode* p = Q.front();
        Q.pop();
        ++count;
        if(p->HasChildren() && p->IsExpanded()) {
            for(size_t i = 0; i < p->GetChildren().size(); ++i) { Q.push(p->GetChildren()[i].get()); }
        }
    }
    return count;
}

void clTreeCtrlNode::GetItemsFromIndex(int start, int count, std::vector<clTreeCtrlNode*>& items)
{
    // find the first entry
    clTreeCtrlNode* startItem = GetVisibleItem(start);
    if(!startItem) return;

    std::queue<clTreeCtrlNode*> Q;
    Q.push(startItem);

    // Now count all the visible children, i.e. the non collapsed ones
    while(!Q.empty()) {
        clTreeCtrlNode* p = Q.front();
        Q.pop();
        items.push_back(p);
        if((int)items.size() == count) { return; }

        // Add the children
        if(p->HasChildren() && p->IsExpanded()) {
            for(size_t i = 0; i < p->GetChildren().size(); ++i) { Q.push(p->GetChildren()[i].get()); }
        }

        bool foundSelf = false;
        if((p == startItem) && p->GetParent()) {
            // Add the top level item siblings
            const std::vector<clTreeCtrlNode::Ptr_t>& siblings = p->GetParent()->GetChildren();
            std::for_each(siblings.begin(), siblings.end(), [&](clTreeCtrlNode::Ptr_t brother) {
                if(foundSelf) {
                    Q.push(brother.get());
                } else if(brother.get() == p) {
                    // from the next iteration, start collecting
                    foundSelf = true;
                }
            });
        }
    }
}

clTreeCtrlNode* clTreeCtrlNode::GetVisibleItem(int index)
{
    std::queue<clTreeCtrlNode*> Q;
    Q.push(this);
    int counter = -1;

    // Now count all the visible children, i.e. the non collapsed ones
    while(!Q.empty()) {
        clTreeCtrlNode* p = Q.front();
        Q.pop();
        ++counter;

        if(counter == index) { return p; }
        if(p->HasChildren() && p->IsExpanded()) {
            for(size_t i = 0; i < p->GetChildren().size(); ++i) { Q.push(p->GetChildren()[i].get()); }
        }
    }
    return nullptr;
}

void clTreeCtrlNode::UnselectAll()
{
    std::queue<clTreeCtrlNode*> Q;
    Q.push(this);

    // Now count all the visible children, i.e. the non collapsed ones
    while(!Q.empty()) {
        clTreeCtrlNode* p = Q.front();
        Q.pop();
        p->SetSelected(false);
        if(p->HasChildren()) {
            for(size_t i = 0; i < p->GetChildren().size(); ++i) { Q.push(p->GetChildren()[i].get()); }
        }
    }
}

int clTreeCtrlNode::GetItemIndex(clTreeCtrlNode* item, bool onlyExpandedItems) const
{
    std::queue<const clTreeCtrlNode*> Q;
    Q.push(this);

    int index = 0;
    // Now count all the visible children, i.e. the non collapsed ones
    while(!Q.empty()) {
        const clTreeCtrlNode* p = Q.front();
        Q.pop();
        if(p == item) { return index; }
        ++index;
        if(p->HasChildren()) {
            if((onlyExpandedItems && p->IsExpanded()) || !onlyExpandedItems) {
                for(size_t i = 0; i < p->GetChildren().size(); ++i) { Q.push(p->GetChildren()[i].get()); }
            }
        }
    }
    return wxNOT_FOUND;
}

bool clTreeCtrlNode::SetExpanded(bool b)
{
    // Already expanded?
    if(b && IsExpanded()) { return true; }
    // Already collapsed?
    if(!b && !IsExpanded()) { return true; }
    
    wxTreeEvent before(b ? wxEVT_TREE_ITEM_EXPANDING : wxEVT_TREE_ITEM_COLLAPSING);
    before.SetItem(wxTreeItemId(this));
    before.SetEventObject(m_sink);
    m_sink->ProcessEvent(before);
    if(!before.IsAllowed()) { return false; }

    SetFlag(kExpanded, b);
    wxTreeEvent after(b ? wxEVT_TREE_ITEM_EXPANDED : wxEVT_TREE_ITEM_COLLAPSED);
    after.SetItem(wxTreeItemId(this));
    after.SetEventObject(m_sink);
    m_sink->ProcessEvent(after);
    return true;
}

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

clTreeCtrlModel::clTreeCtrlModel(clTreeCtrl* tree)
    : m_tree(tree)
    , m_root(tree)
{
}

clTreeCtrlModel::~clTreeCtrlModel() {}

void clTreeCtrlModel::GetItemsFromIndex(int start, int count, std::vector<clTreeCtrlNode*>& items)
{
    m_root.GetItemsFromIndex(start, count, items);
}

wxTreeItemId clTreeCtrlModel::AddRoot(const wxString& text, int image, int selImage, wxTreeItemData* data)
{
    wxUnusedVar(selImage);
    m_root.SetLabel(text);
    m_root.SetBitmapIndex(image);
    m_root.SetClientData(data);
    return wxTreeItemId(&m_root);
}

wxTreeItemId clTreeCtrlModel::GetRootItem() const { return wxTreeItemId(const_cast<clTreeCtrlNode*>(&m_root)); }

int clTreeCtrlModel::GetExpandedLines()
{
    if(m_nVisibleLines == wxNOT_FOUND) { m_nVisibleLines = m_root.GetExpandedLines(); }
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
    for(size_t i = 0; i < m_visibleItems.size(); ++i) { m_visibleItems[i]->SetRect(wxRect()); }
    m_visibleItems.clear();
}

void clTreeCtrlModel::SetVisibleItems(const std::vector<clTreeCtrlNode*>& items)
{
    // Clear the old visible items. But only, if the item does not appear in both lists
    for(size_t i = 0; i < m_visibleItems.size(); ++i) {
        clTreeCtrlNode* visibleItem = m_visibleItems[i];
        std::vector<clTreeCtrlNode*>::const_iterator iter
            = std::find_if(items.begin(), items.end(), [&](clTreeCtrlNode* item) { return item == visibleItem; });
        if(iter == items.end()) { m_visibleItems[i]->SetRect(wxRect()); }
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
    clTreeCtrlNode* child = reinterpret_cast<clTreeCtrlNode*>(item.GetID());
    if(!child) return wxNOT_FOUND;
    return m_root.GetItemIndex(child, visibleItemsOnly);
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
