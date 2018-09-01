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
    clTreeCtrlNode::Ptr_t m_root;
    int m_nVisibleLines = wxNOT_FOUND;
    clTreeCtrlNode::Vec_t m_selectedItems;
    clTreeCtrlNode::Vec_t m_onScreenItems;
    int m_indentSize = 16;

protected:
    void DoExpandAllChildren(const wxTreeItemId& item, bool expand);

public:
    clTreeCtrlModel(clTreeCtrl* tree);
    ~clTreeCtrlModel();

    void ExpandAllChildren(const wxTreeItemId& item);
    void CollapseAllChildren(const wxTreeItemId& item);
    
    void GetItemsFromIndex(int start, int count, clTreeCtrlNode::Vec_t& items);
    /**
     * @brief return item from index. The item must be expanded (and all its parent)
     */
    wxTreeItemId GetItemFromIndex(int index) const;
    wxTreeItemId AddRoot(const wxString& text, int image, int selImage, wxTreeItemData* data);
    wxTreeItemId AppendItem(
        const wxTreeItemId& parent, const wxString& text, int image, int selImage, wxTreeItemData* data);
    wxTreeItemId GetRootItem() const;

    void SetIndentSize(int indentSize) { this->m_indentSize = indentSize; }
    int GetIndentSize() const { return m_indentSize; }

    int GetExpandedLines();

    /**
     * @brief called by the view to indicate that the tree state was modified
     */
    void StateModified();

    /**
     * @brief clear the selection from all the items
     */
    void UnselectAll();

    /**
     * @brief select a given item
     */
    void SelectItem(const wxTreeItemId& item, bool select = true);

    void Clear();

    void SetOnScreenItems(const clTreeCtrlNode::Vec_t& items);

    const clTreeCtrlNode::Vec_t& GetOnScreenItems() const { return m_onScreenItems; }
    clTreeCtrlNode::Vec_t& GetOnScreenItems() { return m_onScreenItems; }
    const clTreeCtrlNode::Vec_t& GetSelections() const { return m_selectedItems; }
    clTreeCtrlNode::Vec_t& GetSelections() { return m_selectedItems; }
    bool ExpandToItem(const wxTreeItemId& item);

    int GetItemIndex(const wxTreeItemId& item, bool visibleItemsOnly = true) const;

    /**
     * @brief do we have items in this tree? (root included)
     */
    bool IsEmpty() const { return m_root == nullptr; }
};

#endif // CLTREECTRLMODEL_H
