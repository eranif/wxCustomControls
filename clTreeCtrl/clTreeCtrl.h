#ifndef CLTREECTRL_H
#define CLTREECTRL_H

#include "clTreeCtrlModel.h"
#include <wx/arrstr.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>

class clTreeCtrl : public wxPanel
{
    int m_lineHeight = 0;
    clTreeCtrlModel m_model;
    int m_firstVisibleLine = 0;
    int m_scrollTick = 2;
    std::vector<wxBitmap> m_bitmaps;
    clTreeCtrlColours m_colours;
    long m_treeStyle = 0;

private:
    int GetExpandedLines();
    wxPoint DoFixPoint(const wxPoint& pt);

public:
    clTreeCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~clTreeCtrl();

    void SetBitmaps(const std::vector<wxBitmap>& bitmaps);
    const std::vector<wxBitmap>& GetBitmaps() const { return m_bitmaps; }

    /**
     * @brief return the tree style
     */
    long GetTreeStyle() const { return m_treeStyle; }

    /**
     * @brief set the colours used for drawing items
     */
    void SetColours(const clTreeCtrlColours& colours);
    /**
     * @brief get the colours used for drawing items
     */
    const clTreeCtrlColours& GetColours() const { return m_colours; }

    /**
     * @brief return line height
     */
    int GetLineHeight() const { return m_lineHeight; }

    /**
     * @brief return bitmap at give index, return wxNullBitmap on error
     */
    const wxBitmap& GetBitmap(size_t index) const;
    /**
     * @brief Calculates which (if any) item is under the given point, returning the tree item id at this point plus
     *  extra information flags.
     *  flags is a bitlist of the following:
     *  wxTREE_HITTEST_NOWHERE: In the client area but below the last item.
     *  wxTREE_HITTEST_ONITEMBUTTON: On the button associated with an item.
     *  wxTREE_HITTEST_ONITEMICON: On the bitmap associated with an item.
     *  wxTREE_HITTEST_ONITEMLABEL: On the label (string) associated with an item.
     */
    wxTreeItemId HitTest(const wxPoint& point, int& flags) const;

    /**
     * @brief ppends an item to the end of the branch identified by parent, return a new item id.
     */
    wxTreeItemId AppendItem(const wxTreeItemId& parent, const wxString& text, int image = -1, int selImage = -1,
        wxTreeItemData* data = NULL);
    /**
     * @brief Adds the root node to the tree, returning the new item.
     */
    wxTreeItemId AddRoot(const wxString& text, int image = -1, int selImage = -1, wxTreeItemData* data = NULL);

    /**
     * @brief return the root item
     */
    wxTreeItemId GetRootItem() const;

    /**
     * @brief Expands the given item
     */
    void Expand(const wxTreeItemId& item);

    /**
     * @brief Collapses the given item
     */
    void Collapse(const wxTreeItemId& item);

    /**
     * @brief Selects the given item
     */
    void SelectItem(const wxTreeItemId& item, bool select = true);

    // Selection
    wxTreeItemId GetSelection() const;
    wxTreeItemId GetFocusedItem() const;
    size_t GetSelections(wxArrayTreeItemIds& selections) const;

    /**
     * @brief unselect all items
     */
    void UnselectAll();

    /**
     * @brief Scrolls and/or expands items to ensure that the given item is visible.
     */
    void EnsureVisible(const wxTreeItemId& item);

    /**
     * @brief is item expanded?
     */
    bool IsExpanded(const wxTreeItemId& item) const;

    /**
     * @brief Returns true if the item has children.
     */
    bool ItemHasChildren(const wxTreeItemId& item) const;

    /**
     * @brief set the item's indent size
     */
    void SetIndent(int size);

    /**
     * @brief return the current indent size
     */
    int GetIndent() const;

    /**
     * @brief is the tree has items? (root included)
     */
    bool IsEmpty() const;

    /**
     * @brief return the children count of this item
     */
    size_t GetChildrenCount(const wxTreeItemId& item, bool recursively = true) const;

    /**
     * @brief Delete all the item's children. No events are fired
     */
    void DeleteChildren(const wxTreeItemId& item);
    /**
     * @brief Returns the first child; call GetNextChild() for the next child.
     */
    wxTreeItemId GetFirstChild(const wxTreeItemId& item, clTreeItemIdValue& cookie) const;
    wxTreeItemId GetNextChild(const wxTreeItemId& item, clTreeItemIdValue& cookie) const;

    /**
     * @brief Returns the first visible item
     */
    wxTreeItemId GetFirstVisibleItem() const;

    /**
     * @brief Returns the next visible item or an invalid item if this item is the last visible one
     */
    wxTreeItemId GetNextVisible(const wxTreeItemId& item) const;

    /**
     * @brief return the item label
     */
    wxString GetItemText(const wxTreeItemId& item) const;

    /**
     * @brief return the associated item data
     */
    wxTreeItemData* GetItemData(const wxTreeItemId& item) const;

    /**
     * @brief expand this item and all its children
     */
    void ExpandAllChildren(const wxTreeItemId& item);

    /**
     * @brief expand the entire tree
     */
    void ExpandAll() { ExpandAllChildren(GetRootItem()); }

    /**
     * @brief expand this item and all its children
     */
    void CollapseAllChildren(const wxTreeItemId& item);

    /**
     * @brief expand the entire tree
     */
    void CollapAll() { CollapseAllChildren(GetRootItem()); }

    /**
     * @brief convert row to item
     * row can only be expanded line (and all of its parent)
     * i.e. the item is visible on screen OR if we scroll we can see it
     */
    wxTreeItemId RowToItem(int row) const;

protected:
    void DoEnsureVisible(const wxTreeItemId& item);
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftDClick(wxMouseEvent& event);
    void OnMouseScroll(wxMouseEvent& event);
    void OnIdle(wxIdleEvent& event);
    void OnLeaveWindow(wxMouseEvent& event);
};

#endif // CLTREECTRL_H
