#include "clCaptionBar.hpp"
#include "drawingutils.h"
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>

clCaptionBar::clCaptionBar() {}

clCaptionBar::clCaptionBar(wxWindow* parent, wxTopLevelWindow* topLevelFrame)
    : wxWindow(parent, wxID_ANY)
    , m_topLevelWindow(topLevelFrame)
{
    m_colours.InitDefaults();

    Bind(wxEVT_PAINT, &clCaptionBar::OnPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, &clCaptionBar::OnEraseBg, this);
    Bind(wxEVT_LEFT_DOWN, &clCaptionBar::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &clCaptionBar::OnLeftUp, this);
    Bind(wxEVT_MOTION, &clCaptionBar::OnMotion, this);
    Bind(wxEVT_ENTER_WINDOW, &clCaptionBar::OnEnterWindow, this);
    Bind(wxEVT_LEAVE_WINDOW, &clCaptionBar::OnLeaveWindow, this);
    Bind(wxEVT_SIZE, &clCaptionBar::OnSize, this);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    DoSetBestSize();
    topLevelFrame->SetWindowStyle(wxRESIZE_BORDER);
    topLevelFrame->PostSizeEvent();
}

clCaptionBar::~clCaptionBar()
{
    Unbind(wxEVT_PAINT, &clCaptionBar::OnPaint, this);
    Unbind(wxEVT_ERASE_BACKGROUND, &clCaptionBar::OnEraseBg, this);
    Unbind(wxEVT_LEFT_DOWN, &clCaptionBar::OnLeftDown, this);
    Unbind(wxEVT_LEFT_UP, &clCaptionBar::OnLeftUp, this);
    Unbind(wxEVT_MOTION, &clCaptionBar::OnMotion, this);
    Unbind(wxEVT_ENTER_WINDOW, &clCaptionBar::OnEnterWindow, this);
    Unbind(wxEVT_LEAVE_WINDOW, &clCaptionBar::OnLeaveWindow, this);
    Unbind(wxEVT_SIZE, &clCaptionBar::OnSize, this);
}

int clCaptionBar::HitTest(const wxPoint& pt) const
{
    wxUnusedVar(pt);
    return wxNOT_FOUND;
}

void clCaptionBar::OnPaint(wxPaintEvent& e)
{
    wxUnusedVar(e);
    wxAutoBufferedPaintDC abdc(this);
    wxGCDC dc(abdc);
    PrepareDC(dc);

    wxRect rect = GetClientRect();
    rect.Inflate(1);
    dc.SetBrush(m_colours.GetBgColour());
    dc.SetPen(m_colours.GetBgColour());
    dc.DrawRectangle(rect);

    dc.SetTextForeground(m_colours.GetItemTextColour());
    dc.SetFont(DrawingUtils::GetDefaultGuiFont());

    int xx = FromDIP(5);

    // draw the bitmap
    if(GetBitmap().IsOk()) {
        int width = GetBitmap().GetScaledWidth();
        int height = GetBitmap().GetScaledHeight();
        wxRect bound_rect(xx, 0, width, height);
        bound_rect = bound_rect.CenterIn(rect, wxVERTICAL);
        dc.DrawBitmap(GetBitmap(), bound_rect.GetTopLeft());
        xx += bound_rect.GetWidth() + FromDIP(5);
    }

    // draw the text
    {
        wxSize textSize = dc.GetTextExtent(GetCaption());
        int width = textSize.GetWidth();
        int height = textSize.GetHeight();

        wxRect bound_rect(xx, 0, width, height);
        bound_rect = bound_rect.CenterIn(rect, wxVERTICAL);
        bound_rect.SetX(bound_rect.GetX());
        dc.DrawText(GetCaption(), bound_rect.GetTopLeft());
    }
}

void clCaptionBar::OnEraseBg(wxEraseEvent& e) { wxUnusedVar(e); }

void clCaptionBar::OnLeftDown(wxMouseEvent& e)
{
    wxUnusedVar(e);
    if(!m_topLevelWindow) {
        return;
    }
    CaptureMouse();
    wxPoint pos = m_topLevelWindow->ClientToScreen(e.GetPosition());
    wxPoint origin = m_topLevelWindow->GetPosition();
    int dx = pos.x - origin.x;
    int dy = pos.y - origin.y;
    m_delta = wxPoint(dx, dy);
}

void clCaptionBar::OnLeftUp(wxMouseEvent& e)
{
    wxUnusedVar(e);
    if(HasCapture()) {
        ReleaseMouse();
    }
}

void clCaptionBar::OnMotion(wxMouseEvent& e)
{
    wxUnusedVar(e);
    if(!m_topLevelWindow) {
        return;
    }

    if(e.Dragging() && e.LeftIsDown()) {
        wxPoint pt = e.GetPosition();
        wxPoint pos = m_topLevelWindow->ClientToScreen(pt);
        m_topLevelWindow->Move(wxPoint(pos.x - m_delta.x, pos.y - m_delta.y));
    }
}

void clCaptionBar::OnEnterWindow(wxMouseEvent& e) { wxUnusedVar(e); }

void clCaptionBar::OnLeaveWindow(wxMouseEvent& e) { wxUnusedVar(e); }

void clCaptionBar::SetColours(const clColours& colours)
{
    m_colours = colours;
    Refresh();
}

void clCaptionBar::DoSetBestSize()
{
    wxBitmap bmp(1, 1);
    wxMemoryDC memDC(bmp);
    wxGCDC dc(memDC);
    dc.SetFont(DrawingUtils::GetDefaultGuiFont());

    int buttonHeight = 0;
    {
        wxRect r = dc.GetTextExtent("Tp");
        r.Inflate(FromDIP(8));
        buttonHeight = r.GetHeight();
    }
    SetSizeHints(wxSize(-1, buttonHeight));
}

void clCaptionBar::OnSize(wxSizeEvent& e)
{
    e.Skip();
    Refresh();
}

void clCaptionBar::SetBitmap(const wxBitmap& bitmap)
{
    m_bitmap = bitmap;
    Refresh();
}

void clCaptionBar::SetCaption(const wxString& caption)
{
    m_caption = caption;
    Refresh();
}
