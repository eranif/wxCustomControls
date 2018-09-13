#ifndef CLDATAVIEWLISTCTRL_H
#define CLDATAVIEWLISTCTRL_H

#include "clControlWithItems.h"

class WXDLLIMPEXP_SDK clDataViewListCtrl : public clControlWithItems
{
protected:
    void OnPaint(wxPaintEvent& event);

public:
    clDataViewListCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~clDataViewListCtrl();

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

    void ProcessIdle();
};

#endif // CLDATAVIEWLISTCTRL_H
