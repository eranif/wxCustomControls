#ifndef CLTREECTRLMODEL_H
#define CLTREECTRLMODEL_H

#include <wx/sharedptr.h>
#include <wx/string.h>
#include <wx/treebase.h>

class clTreeCtrl;
enum clTreeCtrlNodeFlags {
    kFontBold = (1 << 0),
    kFontItalic = (1 << 1),
    kSortItems = (1 << 2),
    kExpanded = (1 << 3),
    kSelected = (1 << 4),
};

class clTreeCtrlNode
{
public:
    typedef wxSharedPtr<clTreeCtrlNode> Ptr_t;

protected:
    wxEvtHandler* m_sink = nullptr;
    wxString m_label;
    int m_bitmapIndex = wxNOT_FOUND;
    size_t m_flags = 0;
    wxString m_colour;
    wxTreeItemData* m_clientData = nullptr;
    clTreeCtrlNode* m_parent = nullptr;
    std::vector<clTreeCtrlNode::Ptr_t> m_children;
    int m_indent = 0;
    wxRect m_rect;

protected:
    void SetFlag(clTreeCtrlNodeFlags flag, bool b)
    {
        if(b) {
            m_flags |= flag;
        } else {
            m_flags &= ~flag;
        }
    }

    bool HasFlag(clTreeCtrlNodeFlags flag) const { return m_flags & flag; }
    void RemoveChild(clTreeCtrlNode* child);
    /**
     * @brief return the nth visible item
     */
    clTreeCtrlNode* GetVisibleItem(int index);

public:
    clTreeCtrlNode(wxEvtHandler* sink);
    clTreeCtrlNode(wxEvtHandler* sink, const wxString& label, int bitmapIndex = wxNOT_FOUND);
    ~clTreeCtrlNode();

    void SetRect(const wxRect& rect) { m_rect = rect; }
    const wxRect& GetRect() const { return m_rect; }

    void AddChild(clTreeCtrlNode::Ptr_t child);

    bool IsBold() const { return HasFlag(kFontBold); }
    void SetBold(bool b) { SetFlag(kFontBold, b); }

    bool IsItalic() const { return HasFlag(kFontItalic); }
    void SetItalic(bool b) { SetFlag(kFontItalic, b); }

    bool IsExpanded() const { return HasFlag(kExpanded); }
    bool SetExpanded(bool b);

    void SetBitmapIndex(int bitmapIndex) { this->m_bitmapIndex = bitmapIndex; }
    int GetBitmapIndex() const { return m_bitmapIndex; }
    const std::vector<clTreeCtrlNode::Ptr_t>& GetChildren() const { return m_children; }
    wxTreeItemData* GetClientObject() const { return m_clientData; }
    void SetParent(clTreeCtrlNode* parent);
    clTreeCtrlNode* GetParent() const { return m_parent; }
    bool HasChildren() const { return !m_children.empty(); }
    void SetClientData(wxTreeItemData* clientData) { this->m_clientData = clientData; }
    void SetLabel(const wxString& label) { this->m_label = label; }
    const wxString& GetLabel() const { return m_label; }

    int GetExpandedLines() const;
    void GetItemsFromIndex(int start, int count, std::vector<clTreeCtrlNode*>& items);
    void SetIndent(int indent) { this->m_indent = indent; }
    int GetIndent() const { return m_indent; }

    bool IsSelected() const { return HasFlag(kSelected); }
    void SetSelected(bool b) { SetFlag(kSelected, b); }

    void UnselectAll();
    /**
     * @brief find the index of an item.
     * @param item the item to search for
     * @param onlyEpandedItems search in the expanded items only
     */
    int GetItemIndex(clTreeCtrlNode* item, bool onlyExpandedItems = true) const;
};

class clTreeCtrlModel
{
    clTreeCtrl* m_tree = nullptr;
    clTreeCtrlNode m_root;
    int m_nVisibleLines = wxNOT_FOUND;
    std::vector<clTreeCtrlNode*> m_selectedItems;
    std::vector<clTreeCtrlNode*> m_visibleItems;

public:
    clTreeCtrlModel(clTreeCtrl* tree);
    ~clTreeCtrlModel();

    void GetItemsFromIndex(int start, int count, std::vector<clTreeCtrlNode*>& items);
    wxTreeItemId AddRoot(const wxString& text, int image, int selImage, wxTreeItemData* data);
    wxTreeItemId AppendItem(
        const wxTreeItemId& parent, const wxString& text, int image, int selImage, wxTreeItemData* data);
    wxTreeItemId GetRootItem() const;

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

    void SetVisibleItems(const std::vector<clTreeCtrlNode*>& items);

    const std::vector<clTreeCtrlNode*>& GetVisibleItems() const { return m_visibleItems; }
    bool ExpandToItem(const wxTreeItemId& item);

    int GetItemIndex(const wxTreeItemId& item, bool visibleItemsOnly = true) const;
};

#endif // CLTREECTRLMODEL_H
