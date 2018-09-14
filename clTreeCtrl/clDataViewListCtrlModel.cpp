#include "clDataViewListCtrlModel.h"
#include <algorithm>

clDataViewListCtrlModel::clDataViewListCtrlModel() {}
clDataViewListCtrlModel::~clDataViewListCtrlModel() {}

void clDataViewListCtrlModel::NodeDeleted(clRowEntry* item)
{
    clRowEntry::Vec_t::iterator iter =
        std::find_if(m_items.begin(), m_items.end(), [&](clRowEntry* p) { return p == item; });
    if(iter != m_items.end()) { m_items.erase(iter); }
}

void clDataViewListCtrlModel::NodeExpanded(clRowEntry* item, bool expanded)
{
    wxUnusedVar(item);
    wxUnusedVar(expanded);
}

bool clDataViewListCtrlModel::NodeExpanding(clRowEntry* item, bool expanding)
{
    wxUnusedVar(item);
    wxUnusedVar(expanding);
    return true;
}

wxDataViewItem clDataViewListCtrlModel::RowToItem(int row) const
{
    if((row >= (int)m_items.size()) || (row < 0)) { return wxDataViewItem(); }
    return wxDataViewItem(m_items[row]);
}

int clDataViewListCtrlModel::ItemToRow(const wxDataViewItem& item) const
{
    int index = 0;
    clRowEntry::Vec_t::const_iterator iter = std::find_if(m_items.begin(), m_items.end(), [&](clRowEntry* p) {
        if(p == item.GetID()) { return true; }
        ++index;
        return false;
    });
    return iter == m_items.end() ? wxNOT_FOUND : index;
}

clRowEntry* clDataViewListCtrlModel::ToPtr(const wxDataViewItem& item) const
{
    if(!item.IsOk()) { return nullptr; }
    return reinterpret_cast<clRowEntry*>(item.GetID());
}

size_t clDataViewListCtrlModel::GetItems(int from, int count, clRowEntry::Vec_t& items) const
{
    items.clear();
    if(from == wxNOT_FOUND) { return 0; }
    for(int i = from; (i < count) && (i < (int)m_items.size()); ++i) {
        items.push_back(m_items[i]);
    }
    return items.size();
}
