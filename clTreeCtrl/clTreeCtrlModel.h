#ifndef CLTREECTRLMODEL_H
#define CLTREECTRLMODEL_H

#include "clTreeCtrlNode.h"
#include <vector>
#include <wx/colour.h>
#include <wx/sharedptr.h>
#include <wx/string.h>
#include <wx/treebase.h>

class clTreeCtrl;
struct clTreeItemIdValue {
    int nextItem = 0;
};

class clTreeCtrlModel
{
    clTreeCtrl* m_tree = nullptr;
    clTreeCtrlNode* m_root = nullptr;
    clTreeCtrlNode::Vec_t m_selectedItems;
    clTreeCtrlNode::Vec_t m_onScreenItems;
    int m_indentSize = 16;
    bool m_shutdown = false;

protected:
    void DoExpandAllChildren(const wxTreeItemId& item, bool expand);
    bool IsSingleSelection() const;
    bool IsMultiSelection() const;
    bool SendEvent(wxEvent& event);

public:
    clTreeCtrlModel(clTreeCtrl* tree);
    ~clTreeCtrlModel();

    void ExpandAllChildren(const wxTreeItemId& item);
    void CollapseAllChildren(const wxTreeItemId& item);

    // Notifications from the node
    void NodeDeleted(clTreeCtrlNode* node);
    void NodeExpanded(clTreeCtrlNode* node, bool expanded);
    bool NodeExpanding(clTreeCtrlNode* node, bool expanding);

    void GetNextItems(clTreeCtrlNode* from, int count, clTreeCtrlNode::Vec_t& items) const;
    void GetPrevItems(clTreeCtrlNode* from, int count, clTreeCtrlNode::Vec_t& items) const;
    wxTreeItemId AddRoot(const wxString& text, int image, int selImage, wxTreeItemData* data);
    wxTreeItemId AppendItem(
        const wxTreeItemId& parent, const wxString& text, int image, int selImage, wxTreeItemData* data);
    wxTreeItemId GetRootItem() const;

    void SetIndentSize(int indentSize) { this->m_indentSize = indentSize; }
    int GetIndentSize() const { return m_indentSize; }

    /**
     * @brief clear the selection from all the items
     */
    void UnselectAll();

    wxTreeItemId GetItemBefore(const wxTreeItemId& item, bool visibleItem) const;
    wxTreeItemId GetItemAfter(const wxTreeItemId& item, bool visibleItem) const;
    clTreeCtrlNode* ToPtr(const wxTreeItemId& item) const
    {
        if(!m_root || !item.IsOk()) { return nullptr; }
        return reinterpret_cast<clTreeCtrlNode*>(item.GetID());
    }

    /**
     * @brief select a given item
     */
    void SelectItem(
        const wxTreeItemId& item, bool select = true, bool addSelection = false, bool clear_old_selection = false);
    /**
     * @brief plural version
     */
    void SelectItems(const std::vector<std::pair<wxTreeItemId, bool>>& items);

    void Clear();

    void SetOnScreenItems(const clTreeCtrlNode::Vec_t& items);

    const clTreeCtrlNode::Vec_t& GetOnScreenItems() const { return m_onScreenItems; }
    clTreeCtrlNode::Vec_t& GetOnScreenItems() { return m_onScreenItems; }
    const clTreeCtrlNode::Vec_t& GetSelections() const { return m_selectedItems; }
    //clTreeCtrlNode::Vec_t& GetSelections() { return m_selectedItems; }
    bool ExpandToItem(const wxTreeItemId& item);
    wxTreeItemId GetSingleSelection() const;
    size_t GetSelectionsCount() const { return m_selectedItems.size(); }
    
    /**
     * @brief do we have items in this tree? (root included)
     */
    bool IsEmpty() const { return m_root == nullptr; }

    clTreeCtrlNode* GetRoot() const { return m_root; }

    /**
     * @brief delete subtree starting from 'item', including item
     * fires event wxEVT_TREE_DELETE_ITEM
     * @param item
     */
    void DeleteItem(const wxTreeItemId& item);
    int GetItemIndex(clTreeCtrlNode* item) const;
    
    /**
     * @brief get range of items from -> to
     * Or from: to->from (incase 'to' has a lower index)
     */
    bool GetRange(clTreeCtrlNode* from, clTreeCtrlNode* to, clTreeCtrlNode::Vec_t& items) const;
};

#endif // CLTREECTRLMODEL_H
