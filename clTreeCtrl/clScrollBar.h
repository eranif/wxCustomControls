#ifndef CLSCROLLBAR_H
#define CLSCROLLBAR_H

#include <wx/panel.h>

class clScrollBar : public wxPanel
{
    int m_totalTicks = 0;
    int m_tickSize = 0;
    int m_firstTick = 0;
    wxOrientation m_orientation = wxVERTICAL;

public:
    clScrollBar(wxWindow* parent, wxOrientation orientation = wxVERTICAL);
    virtual ~clScrollBar();

    void OnPaint(wxPaintEvent& event);
    /**
     * @brief update the scrollbar
     * @param ticksCount number of total ticks (lines or columns) managed by this scrollbar
     * @param tickSize size of a tick (in pixels)
     * @param firstTick the index of the first visible tick. Must be in the range of 0 <= firstTick < ticksCount
     */
    void Update(int ticksCount, int tickSize, int firstTick);
};

#endif // CLSCROLLBAR_H
