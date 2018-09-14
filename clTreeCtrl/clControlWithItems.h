#ifndef CLCONTROLWITHITEMS_H
#define CLCONTROLWITHITEMS_H

#include "clColours.h"
#include "clHeaderBar.h"
#include "clRowEntry.h"
#include "clScrolledPanel.h"

#ifdef __WXOSX__
#define SCROLL_TICK 2
#else
#define SCROLL_TICK 3
#endif

class clItemModelBase;
class WXDLLIMPEXP_SDK clControlWithItems : public clScrolledPanel
{
private:
    clHeaderBar m_viewHeader;
    clColours m_colours;
    int m_firstColumn = 0;
    int m_lineHeight = 0;
    int m_indent = 0;
    std::vector<wxBitmap> m_bitmaps;
    int m_scrollTick = SCROLL_TICK;
    int m_controlStyle = 0;

protected:
    int GetNumLineCanFitOnScreen() const;
    void RenderHeader(wxDC& dc);
    void RenderItems(wxDC& dc, const clRowEntry::Vec_t& items);
    void OnSize(wxSizeEvent& event);
    void DoUpdateHeader(clRowEntry* row);
    wxSize GetTextSize(const wxString& label) const;
    virtual void OnMouseScroll(wxMouseEvent& event);

public:
    clControlWithItems(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~clControlWithItems();

    virtual int GetIndent() const { return m_indent; }

    virtual void SetFirstColumn(int firstColumn) { this->m_firstColumn = firstColumn; }
    virtual int GetFirstColumn() const { return m_firstColumn; }

    virtual void SetLineHeight(int lineHeight) { this->m_lineHeight = lineHeight; }
    virtual int GetLineHeight() const { return m_lineHeight; }

    virtual void SetBitmaps(const std::vector<wxBitmap>& bitmaps) { this->m_bitmaps = bitmaps; }
    virtual const std::vector<wxBitmap>& GetBitmaps() const { return m_bitmaps; }

    void SetScrollTick(int scrollTick) { this->m_scrollTick = scrollTick; }
    int GetScrollTick() const { return m_scrollTick; }

    /**
     * @brief return the tree style
     */
    long GetControlStyle() const { return m_controlStyle; }
    void SetControlStyle(int style) { m_controlStyle = style; }
    
    bool HasControlStyle(int style) const { return m_controlStyle & style; }

    void EnableControlStyle(int style, bool set)
    {
        if(set) {
            m_controlStyle |= style;
        } else {
            m_controlStyle &= ~style;
        }
    }
    /**
     * @brief return bitmap at a given index
     */
    const wxBitmap& GetBitmap(size_t index) const;

    /**
     * @brief set the item's indent size
     */
    virtual void SetIndent(int size) { m_indent = size; }

    /**
     * @brief return the items rect area, excluding header
     */
    wxRect GetItemsRect() const;

    /**
     * @brief draw the header + items
     */
    void Render(wxDC& dc);

    /**
     * @brief return the header bar (relevant when using columns)
     */
    const clHeaderBar& GetHeader() const { return m_viewHeader; }
    /**
     * @brief return the header bar (relevant when using columns)
     */
    clHeaderBar& GetHeader() { return m_viewHeader; }

    /**
     * @param header
     */
    void SetHeader(const clHeaderBar& header);

    /**
     * @brief should we show the header bar?
     */
    void SetShowHeader(bool b);
    /**
     * @brief is the heaer bar visible?
     */
    bool IsHeaderVisible() const;

    /**
     * @brief update the scrollbar with the current view status
     * subclass should call this method whenver the view changes (re-sized, items are expanding, collapsing etc)
     */
    void UpdateScrollBar();

    void SetColours(const clColours& colours)
    {
        this->m_colours = colours;
        Refresh();
    }

    const clColours& GetColours() const { return m_colours; }
    clColours& GetColours() { return m_colours; }

    // Horizontal scrolling implementation
    void ScollToColumn(int firstColumn);
    void ScrollColumns(int steps, wxDirection direction);

    /**
     * @brief Calculates which (if any) item is under the given point, returning the tree item id at this point plus
     *  extra information flags.
     *  flags is a bitlist of the following:
     *  wxTREE_HITTEST_NOWHERE: In the client area but below the last item.
     *  wxTREE_HITTEST_ONITEMBUTTON: On the button associated with an item.
     *  wxTREE_HITTEST_ONITEMICON: On the bitmap associated with an item.
     *  wxTREE_HITTEST_ONITEMLABEL: On the label (string) associated with an item.
     * wxTREE_HITTEST_ONITEM
     */
    clRowEntry* HitTest(const wxPoint& point, int& flags) const;

    //===-----------------------------------------
    //===-----------------------------------------

    /**
     * @brief override this
     */
    virtual bool IsEmpty() const = 0;
    /**
     * @brief return the total of numbers of items we can scroll
     * If the view has collpased items, the range _excludes_ them
     */
    virtual int GetRange() const = 0;

    /**
     * @brief return the row number of the first visible item in the view
     */
    virtual int GetFirstItemPosition() const = 0;

    /**
     * @brief return list of all visible items
     */
    virtual clRowEntry::Vec_t GetOnScreenItems() const = 0;
    
    /**
     * @brief return the model associated with this control
     */
    virtual clItemModelBase* GetModel() = 0;
};

#endif // CLCONTROLWITHITEMS_H
