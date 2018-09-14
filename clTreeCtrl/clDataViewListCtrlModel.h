#ifndef CLDATAVIEWLISTCTRLMODEL_H
#define CLDATAVIEWLISTCTRLMODEL_H

#include "clRowEntry.h"
#include "codelite_exports.h"
#include <wx/dataview.h>

class WXDLLIMPEXP_SDK clDataViewListCtrlModel : public clItemModelBase
{
    clRowEntry::Vec_t m_items;

public:
    virtual void NodeDeleted(clRowEntry* item);
    virtual void NodeExpanded(clRowEntry* item, bool expanded);
    virtual bool NodeExpanding(clRowEntry* item, bool expanding);

    clDataViewListCtrlModel();
    virtual ~clDataViewListCtrlModel();

    int ItemToRow(const wxDataViewItem& item) const;
    wxDataViewItem RowToItem(int row) const;
    
    clRowEntry* ToPtr(const wxDataViewItem& item) const;
    size_t size() const { return m_items.size(); }
    bool empty() const { return m_items.empty(); }
    bool IsEmpty() const { return empty(); }
    void Add(clRowEntry* item) { m_items.push_back(item); }
    /**
     * @brief return count items from a given position
     */
    size_t GetItems(int from, int count, clRowEntry::Vec_t& items) const;
};

#endif // CLDATAVIEWLISTCTRLMODEL_H
