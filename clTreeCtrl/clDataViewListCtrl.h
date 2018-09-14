#ifndef CLDATAVIEWLISTCTRL_H
#define CLDATAVIEWLISTCTRL_H

#include "clControlWithItems.h"
#include "clDataViewListCtrlModel.h"
#include <vector>
#include <wx/dataview.h>

class WXDLLIMPEXP_SDK clDataViewListCtrl : public clControlWithItems
{
    clDataViewListCtrlModel m_model;
    int m_firstItemOnScreen = wxNOT_FOUND;

protected:
    void OnPaint(wxPaintEvent& event);
    clRowEntry* ToPtr(const wxDataViewItem& item) const { return m_model.ToPtr(item); }

public:
    virtual int GetFirstItemPosition() const;
    virtual clRowEntry::Vec_t GetOnScreenItems() const;
    virtual int GetRange() const;
    virtual bool IsEmpty() const;

    clDataViewListCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~clDataViewListCtrl();

    clItemModelBase* GetModel() { return &m_model; }

    /**
     * @brief override this method to scroll the view
     * @param steps number of lines to scroll. If 'steps' is set to 0, then scroll to top or bottom of the view
     * depending on the direction. otherwise, scroll 'steps' into the correct 'direction'
     * @param direction direction to scroll
     */
    virtual void ScrollRows(int steps, wxDirection direction);

    /**
     * @brief scroll to set 'firstLine' as the first visible line in the view
     */
    virtual void ScrollToRow(int firstLine);

    /**
     * @brief return item's index
     */
    int ItemToRow(const wxDataViewItem& item) const;

    /**
     * @brief return item from row
     */
    wxDataViewItem RowToItem(int row) const;

    /**
     * @brief Appends an item (i.e. a row) to the control.
     */
    void AppendItem(const wxVector<wxVariant>& values, wxUIntPtr data = 0);

    wxDataViewColumn* AppendIconTextColumn(const wxString& label, wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                                           int width = -1, wxAlignment align = wxALIGN_LEFT,
                                           int flags = wxDATAVIEW_COL_RESIZABLE);

    wxDataViewColumn* AppendProgressColumn(const wxString& label, wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                                           int width = -1, wxAlignment align = wxALIGN_LEFT,
                                           int flags = wxDATAVIEW_COL_RESIZABLE);

    wxDataViewColumn* AppendTextColumn(const wxString& label, wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
                                       int width = -1, wxAlignment align = wxALIGN_LEFT,
                                       int flags = wxDATAVIEW_COL_RESIZABLE);
    void ProcessIdle();
};

#endif // CLDATAVIEWLISTCTRL_H
