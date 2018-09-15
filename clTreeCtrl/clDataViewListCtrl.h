#ifndef CLDATAVIEWLISTCTRL_H
#define CLDATAVIEWLISTCTRL_H

#include "clTreeCtrl.h"
#include <wx/dataview.h>

/**
 * @brief a thin wrapper around clTreeCtrl which provides basic compatiblity API (such as adding columns)
 * This is mainly for code generators like wxCrafter
 */
class WXDLLIMPEXP_SDK clDataViewListCtrl : public clTreeCtrl
{
    bool m_needToClearDefaultHeader = true;
    wxDataViewListCtrl m_dummy; // Needed for generating wxDV compatible events

protected:
    void DoAddHeader(const wxString& label, int width);
    void OnConvertEvent(wxTreeEvent& event);
    bool SendDataViewEvent(const wxEventType& type, wxTreeEvent& treeEvent);

public:
    clDataViewListCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~clDataViewListCtrl();

    ///===--------------------
    /// wxDV compatilibty API
    ///===--------------------

    /**
     * @brief appends an item to the end of the list
     */
    wxDataViewItem AppendItem(const wxString& text, int image = -1, int selImage = -1, wxUIntPtr data = 0);

    /**
     * @brief insert item after 'previous'
     */
    wxDataViewItem InsertItem(const wxDataViewItem& previous, const wxString& text, int image = -1, int selImage = -1,
                              wxUIntPtr data = 0);

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

    wxDataViewItem GetSelection() const { return wxDataViewItem(GetSelection().GetID()); }
    wxDataViewItem GetCurrentItem() const { return wxDataViewItem(GetFocusedItem().GetID()); }
    int GetSelections(wxDataViewItemArray& sel) const;
    int GetSelectedItemsCount() const;
    void DeleteAllItems();

    wxUIntPtr GetItemData(const wxDataViewItem& item) const;
    void SetItemData(const wxDataViewItem& item, wxUIntPtr data);

    void SetItemBackgroundColour(const wxDataViewItem& item, const wxColour& colour, size_t col = 0);
    wxColour GetItemBackgroundColour(const wxDataViewItem& item, size_t col = 0) const;

    void SetItemTextColour(const wxDataViewItem& item, const wxColour& colour, size_t col = 0);
    wxColour GetItemTextColour(const wxDataViewItem& item, size_t col = 0) const;

    void SetItemText(const wxDataViewItem& item, const wxString& text, size_t col = 0);
    wxString GetItemText(const wxDataViewItem& item, size_t col = 0) const;

    void SetItemBold(const wxDataViewItem& item, bool bold, size_t col = 0);

    void SetItemFont(const wxDataViewItem& item, const wxFont& font, size_t col = 0);
    wxFont GetItemFont(const wxDataViewItem& item, size_t col = 0) const;
};

#endif // CLDATAVIEWLISTCTRL_H
