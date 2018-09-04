#include "clScrollBar.h"
#include <wx/dc.h>
#include <wx/log.h>
#include <wx/settings.h>

clScrollBar::clScrollBar(wxWindow* parent, wxOrientation orientation)
    : m_parent(parent)
    , m_orientation(orientation)
{
    m_buttonColour = wxColour("rgb(133,146,158)");
    m_bgColour = wxColour("rgb(52,73,94)");
    m_parent->Bind(wxEVT_MOUSE_CAPTURE_LOST, [&](wxMouseCaptureLostEvent& e) {
        e.Skip();
        DoCancelScrolling();
    });
    m_parent->Bind(wxEVT_LEFT_DOWN, &clScrollBar::OnMouseLeftDown, this);
    m_parent->Bind(wxEVT_LEFT_UP, &clScrollBar::OnMouseLeftUp, this);
    m_parent->Bind(wxEVT_MOTION, &clScrollBar::OnMouseMotion, this);
}

clScrollBar::~clScrollBar()
{
    m_parent->Unbind(wxEVT_LEFT_DOWN, &clScrollBar::OnMouseLeftDown, this);
    m_parent->Unbind(wxEVT_LEFT_UP, &clScrollBar::OnMouseLeftUp, this);
    m_parent->Unbind(wxEVT_MOTION, &clScrollBar::OnMouseMotion, this);
}

void clScrollBar::Render(wxDC& dc)
{
    if(m_thumbSize == 0 || m_range == 0 || m_pageSize == 0 || m_lineInPixels == 0) { return; }

    wxRect rect = GetClientRect();
    m_thumbRect = wxRect();
    wxRect parentRect = GetParent()->GetClientRect();

    // Get the button size
    int buttonSize = DoGetButtonSize();

    // Calculate the button position
    int buttonPos = DoGetButtonPosition();
    if(m_orientation == wxVERTICAL) {
        m_thumbRect = wxRect((parentRect.GetWidth() - rect.GetWidth()), buttonPos, rect.GetWidth(), buttonSize);
        if((m_thumbRect.GetHeight() + m_thumbRect.GetY()) > rect.GetHeight()) {
            m_thumbRect.SetY(rect.GetHeight() - m_thumbRect.GetHeight());
        }
        // Do we need to show the bar?
        if(m_thumbRect.GetHeight() >= rect.GetHeight()) { return; }
    } else {
        m_thumbRect = wxRect(buttonPos, (parentRect.GetHeight() - rect.GetHeight()), buttonSize, rect.GetHeight());
        if((m_thumbRect.GetWidth() + m_thumbRect.GetX()) > rect.GetWidth()) {
            m_thumbRect.SetX(rect.GetWidth() - m_thumbRect.GetWidth());
        }
        // Do we need to show the bar?
        if(m_thumbRect.GetWidth() >= rect.GetWidth()) { return; }
    }

    // Draw the background
    dc.SetPen(m_bgColour);
    dc.SetBrush(m_bgColour);
    dc.DrawRectangle(rect);

    // Draw the thumb
    dc.SetPen(m_buttonColour);
    dc.SetBrush(m_buttonColour);
    dc.DrawRoundedRectangle(m_thumbRect, 2.0);
}

void clScrollBar::SetScrollbar(int position, int thumbSize, int range, int lineInPixels)
{
    m_position = position;
    m_thumbSize = thumbSize;
    m_range = range;
    m_pageSize = thumbSize - 1;
    m_lineInPixels = lineInPixels;
}

wxRect clScrollBar::GetClientRect()
{
    wxRect parentSize = GetParent()->GetClientRect();
    wxRect rect;
    if(m_orientation == wxVERTICAL) {
        rect.SetWidth(GetSize());
        rect.SetX(parentSize.GetWidth() - rect.GetWidth());
        rect.SetY(0);
        rect.SetHeight(parentSize.GetHeight());
    } else {
        rect.SetHeight(GetSize());
        rect.SetX(0);
        rect.SetY(parentSize.GetHeight() - rect.GetHeight());
        rect.SetWidth(parentSize.GetWidth());
    }
    return rect;
}

void clScrollBar::OnMouseLeftDown(wxMouseEvent& event)
{
    if(m_thumbRect.Contains(event.GetPosition())) {
        m_scrolling = true;
        m_anchorPoint = event.GetPosition();
        m_parent->CaptureMouse();
    } else {
        event.Skip();
    }
}

void clScrollBar::OnMouseMotion(wxMouseEvent& event)
{
    event.Skip();
    if(m_parent->HasCapture()) {
        wxRect clientRect = GetClientRect();
        int maxjorAxis = m_orientation == wxVERTICAL ? clientRect.GetHeight() : clientRect.GetWidth();
        double pixelsPerLine = (double)maxjorAxis / (double)m_range;
        if(pixelsPerLine == 0.0) { return; }
        
        if(clientRect.GetHeight() <= 0) {
            DoCancelScrolling();
            return;
        }
        wxPoint currpoint = event.GetPosition();
        int buttonSize = DoGetButtonSize();
        // int buttonPos = DoGetButtonPosition();
        if(m_orientation == wxVERTICAL) {
            if((currpoint.y + buttonSize) >= clientRect.GetHeight()) { return; }

            if(currpoint.y < 0) {
                currpoint.y = 0;
            } else if(currpoint.y >= clientRect.GetHeight()) {
                currpoint.y = clientRect.GetHeight();
            }

            int diff = abs((m_anchorPoint.y - currpoint.y));
            if(diff <= pixelsPerLine) { return; }
            int lines = (m_anchorPoint.y - currpoint.y) / pixelsPerLine;

            // Calculate the position and fire the event
            // int pos = (currpoint.y * m_range) / clientRect.GetHeight();
            wxScrollEvent scrollEvent(wxEVT_SCROLL_THUMBTRACK);
            scrollEvent.SetEventObject(m_parent);
            scrollEvent.SetPosition(lines); // Negative number means we moved up
            ProcessEvent(scrollEvent);
            m_anchorPoint = currpoint;
        } else if(m_orientation == wxHORIZONTAL) {
            if(currpoint.x < 0) {
                currpoint.x = 0;
            } else if(currpoint.x >= clientRect.GetWidth()) {
                currpoint.x = clientRect.GetWidth();
            }
        }
    }
}

void clScrollBar::OnMouseLeftUp(wxMouseEvent& event)
{
    event.Skip();
    DoCancelScrolling();
}

void clScrollBar::DoCancelScrolling()
{
    if(m_parent->HasCapture()) {
        wxLogMessage("Scrollbar released the mouse capture");
        m_parent->ReleaseMouse();
    }
    m_anchorPoint = wxPoint();
    m_scrolling = false;
}

int clScrollBar::DoGetButtonSize()
{
    wxRect clientRect = GetClientRect();
    int majorAxis = m_orientation == wxVERTICAL ? clientRect.GetHeight() : clientRect.GetWidth();
    int buttonSize = ((double)m_thumbSize / (double)m_range) * majorAxis;

    // Dont let the button size to be too small
    if(buttonSize < GetMinButtonSize()) { buttonSize = GetMinButtonSize(); }
    return buttonSize;
}

int clScrollBar::DoGetButtonPosition()
{
    wxRect clientRect = GetClientRect();
    int majorAxis = m_orientation == wxVERTICAL ? clientRect.GetHeight() : clientRect.GetWidth();
    int pos = ((double)m_position / (double)m_range) * majorAxis;
    return pos;
}
