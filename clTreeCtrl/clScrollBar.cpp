#include "clScrollBar.h"
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>

clScrollBar::clScrollBar(wxWindow* parent, wxOrientation orientation)
    : wxPanel(parent)
    , m_orientation(orientation)
{
    Bind(wxEVT_PAINT, &clScrollBar::OnPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, [&](wxEraseEvent& event) { wxUnusedVar(event); });
    if(m_orientation == wxVERTICAL) {
        SetSizeHints(16, -1);
    } else {
        SetSizeHints(-1, 16);
    }
}

clScrollBar::~clScrollBar() {}

void clScrollBar::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    wxGCDC gcdc(dc);
}

void clScrollBar::Update(int ticksCount, int tickSize, int firstTick)
{
    wxRect clientRect = GetClientRect();
    double thumbRatio = ((double)ticksCount / (double)visibleTicks);
    m_thumbSize = thumbRatio * clientRect.GetHeight();
    m_thumbPos = ((double)((double)firstTick / (double)ticksCount) * clientRect.GetHeight()) + clientRect.GetY();
    if(m_thumbSize < 0) {
        m_thumbSize = 0;
    }
    if(m_thumbPos < 0) {
        m_thumbPos = 0;
    }
    Refresh();
}
