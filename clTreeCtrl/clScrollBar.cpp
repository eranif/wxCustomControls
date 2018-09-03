#include "clScrollBar.h"
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>

clScrollBar::clScrollBar(wxWindow* parent, wxOrientation orientation)
    : wxPanel(parent)
    , m_orientation(orientation)
{
    Bind(wxEVT_PAINT, &clScrollBar::OnPaint, this);
    Bind(wxEVT_SIZE, [&](wxSizeEvent& event) {
        event.Skip();
        Refresh();
    });
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
    m_tickSize = tickSize;
    m_totalTicks = ticksCount;
    m_firstTick = firstTick;
    Refresh();
}
