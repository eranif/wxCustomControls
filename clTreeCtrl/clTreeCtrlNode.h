#ifndef CLTREECTRLNODE_H
#define CLTREECTRLNODE_H

#include <wx/colour.h>
#include <wx/sharedptr.h>
#include <wx/string.h>
#include <wx/treebase.h>
#include <wx/gdicmn.h>

class clTreeCtrl;
enum clTreeCtrlNodeFlags {
    kFontBold = (1 << 0),
    kFontItalic = (1 << 1),
    kSortItems = (1 << 2),
    kExpanded = (1 << 3),
    kSelected = (1 << 4),
};

struct clTreeCtrlColours {
    wxColour textColour;
    wxColour selItemTextColour;
    wxColour selItemBgColour;
    wxColour buttonColour;
};

class clTreeCtrlNode
{
public:
    typedef wxSharedPtr<clTreeCtrlNode> Ptr_t;
    static const int Y_SPACER = 2;
    static const int X_SPACER = 2;

protected:
    clTreeCtrl* m_tree = nullptr;
    wxString m_label;
    int m_bitmapIndex = wxNOT_FOUND;
    int m_bitmapSelectedIndex = wxNOT_FOUND;
    size_t m_flags = 0;
    wxString m_colour;
    wxTreeItemData* m_clientData = nullptr;
    clTreeCtrlNode* m_parent = nullptr;
    std::vector<clTreeCtrlNode::Ptr_t> m_children;
    int m_indentsCount = 0;
    wxRect m_itemRect;
    wxRect m_buttonRect;

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
    clTreeCtrlNode(clTreeCtrl* tree);
    clTreeCtrlNode(
        clTreeCtrl* tree, const wxString& label, int bitmapIndex = wxNOT_FOUND, int bitmapSelectedIndex = wxNOT_FOUND);
    ~clTreeCtrlNode();

    void Render(wxDC& dc, const clTreeCtrlColours& colours);

    void ClearRects();
    void SetRects(const wxRect& rect, const wxRect& buttonRect)
    {
        m_itemRect = rect;
        m_buttonRect = buttonRect;
    }
    const wxRect& GetItemRect() const { return m_itemRect; }
    const wxRect& GetButtonRect() const { return m_buttonRect; }

    void AddChild(clTreeCtrlNode::Ptr_t child);

    bool IsBold() const { return HasFlag(kFontBold); }
    void SetBold(bool b) { SetFlag(kFontBold, b); }

    bool IsItalic() const { return HasFlag(kFontItalic); }
    void SetItalic(bool b) { SetFlag(kFontItalic, b); }

    bool IsExpanded() const { return HasFlag(kExpanded); }
    bool SetExpanded(bool b);

    void SetBitmapIndex(int bitmapIndex) { this->m_bitmapIndex = bitmapIndex; }
    int GetBitmapIndex() const { return m_bitmapIndex; }
    void SetBitmapSelectedIndex(int bitmapIndex) { this->m_bitmapSelectedIndex = bitmapIndex; }
    int GetBitmapSelectedIndex() const { return m_bitmapSelectedIndex; }

    const std::vector<clTreeCtrlNode::Ptr_t>& GetChildren() const { return m_children; }
    std::vector<clTreeCtrlNode::Ptr_t>& GetChildren() { return m_children; }
    wxTreeItemData* GetClientObject() const { return m_clientData; }
    void SetParent(clTreeCtrlNode* parent);
    clTreeCtrlNode* GetParent() const { return m_parent; }
    bool HasChildren() const { return !m_children.empty(); }
    void SetClientData(wxTreeItemData* clientData) { this->m_clientData = clientData; }
    void SetLabel(const wxString& label) { this->m_label = label; }
    const wxString& GetLabel() const { return m_label; }
    size_t GetChildrenCount(bool recurse) const;
    int GetExpandedLines() const;
    void GetItemsFromIndex(int start, int count, std::vector<clTreeCtrlNode*>& items);
    void SetIndentsCount(int count) { this->m_indentsCount = count; }
    int GetIndentsCount() const { return m_indentsCount; }

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

#endif // CLTREECTRLNODE_H
