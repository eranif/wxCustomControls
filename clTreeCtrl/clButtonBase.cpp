#include "clButtonBase.h"
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/buffer.h>
#include <wx/renderer.h>
#include <wx/settings.h>
#include <wx/anybutton.h>

#define TEXT_SPACER 10

#ifdef __WXMSW__
#define BUTTON_RADIUS 0.0
#else
#define BUTTON_RADIUS 3.0
#endif

clButtonBase::clButtonBase() {}

clButtonBase::clButtonBase(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos,
                           const wxSize& size, long style, const wxValidator& validator, const wxString& name)
    : wxControl(parent, id, pos, size, wxTAB_TRAVERSAL | wxNO_BORDER | wxWANTS_CHARS)
    , m_buttonStyle(style)
{
    SetText(label);
    Initialise();
}

bool clButtonBase::Create(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos,
                          const wxSize& size, long style, const wxValidator& validator, const wxString& name)
{
    m_buttonStyle = style;
    SetText(label);
    if(!wxControl::Create(parent, id, pos, size, wxTAB_TRAVERSAL | wxNO_BORDER | wxWANTS_CHARS)) { return false; }
    Initialise();
    return true;
}

clButtonBase::~clButtonBase() { UnBindEvents(); }

void clButtonBase::BindEvents()
{
    Bind(wxEVT_PAINT, &clButtonBase::OnPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, &clButtonBase::OnErasebg, this);
    Bind(wxEVT_LEFT_DOWN, &clButtonBase::OnLeftDown, this);
    Bind(wxEVT_LEFT_DCLICK, &clButtonBase::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &clButtonBase::OnLeftUp, this);
    Bind(wxEVT_ENTER_WINDOW, &clButtonBase::OnEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &clButtonBase::OnLeave, this);
    Bind(wxEVT_SET_FOCUS, &clButtonBase::OnFocus, this);
    Bind(wxEVT_KILL_FOCUS, &clButtonBase::OnFocus, this);
    Bind(wxEVT_KEY_DOWN, &clButtonBase::OnKeyDown, this);
    Bind(wxEVT_IDLE, &clButtonBase::OnIdle, this);
    Bind(wxEVT_SIZE, &clButtonBase::OnSize, this);
}

void clButtonBase::UnBindEvents()
{
    Unbind(wxEVT_PAINT, &clButtonBase::OnPaint, this);
    Unbind(wxEVT_ERASE_BACKGROUND, &clButtonBase::OnErasebg, this);
    Unbind(wxEVT_LEFT_DOWN, &clButtonBase::OnLeftDown, this);
    Unbind(wxEVT_LEFT_DCLICK, &clButtonBase::OnLeftDown, this);
    Unbind(wxEVT_LEFT_UP, &clButtonBase::OnLeftUp, this);
    Unbind(wxEVT_ENTER_WINDOW, &clButtonBase::OnEnter, this);
    Unbind(wxEVT_LEAVE_WINDOW, &clButtonBase::OnLeave, this);
    Unbind(wxEVT_SET_FOCUS, &clButtonBase::OnFocus, this);
    Unbind(wxEVT_KILL_FOCUS, &clButtonBase::OnFocus, this);
    Unbind(wxEVT_KEY_DOWN, &clButtonBase::OnKeyDown, this);
    Unbind(wxEVT_IDLE, &clButtonBase::OnIdle, this);
    Unbind(wxEVT_SIZE, &clButtonBase::OnSize, this);
}

void clButtonBase::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC abdc(this);
    wxGCDC dc(abdc);
    PrepareDC(dc);
    Render(dc);
    m_lastPaintFlags = GetDrawingFlags();
    if(HasFocus()) {
        wxRect clientRect = GetClientRect();
        wxRect focusRect = clientRect.Deflate(3);
        focusRect = focusRect.CenterIn(clientRect);
        wxRendererNative::GetGeneric().DrawFocusRect(this, abdc, focusRect, wxCONTROL_SELECTED);
    }
}

void clButtonBase::OnErasebg(wxEraseEvent& event) { wxUnusedVar(event); }

void clButtonBase::OnLeftDown(wxMouseEvent& event)
{
    event.Skip();
    if(!IsEnabled()) { return; }

    m_state = eButtonState::kPressed;
    CaptureMouse();
    Refresh();
}

void clButtonBase::OnLeftUp(wxMouseEvent& event)
{
    event.Skip();
    if(HasCapture()) { ReleaseMouse(); }
    if(!IsEnabled()) { return; }

    wxRect rect = GetClientRect();
    if(rect.Contains(event.GetPosition())) {
        m_state = eButtonState::kHover;
        PostClickEvent();
    } else {
        m_state = eButtonState::kNormal;
    }
    Refresh();
}

void clButtonBase::Initialise()
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    BindEvents();
    m_colours.InitFromColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    SetSizeHints(GetBestSize());
}

void clButtonBase::Render(wxDC& dc)
{
    wxRect clientRect = GetClientRect();
    wxRect rect = clientRect;
#ifdef __WXOSX__
    clientRect.Inflate(1);
#endif

    dc.SetBrush(m_colours.GetBgColour());
    dc.SetPen(m_colours.GetBgColour());
    dc.DrawRectangle(clientRect);

    bool isDisabled = !IsEnabled();
    bool isDark = DrawingUtils::IsDark(m_colours.GetBgColour());
    wxColour bgColour = m_colours.GetBgColour();
    wxColour borderColour = m_colours.GetBorderColour();

    switch(m_state) {
    case eButtonState::kNormal:
    case eButtonState::kHover:
        bgColour = bgColour.ChangeLightness(isDark ? 80 : 95);
        break;
    case eButtonState::kPressed:
        bgColour = bgColour.ChangeLightness(isDark ? 70 : 80);
        break;
    }

    if(isDark) {
#ifdef __WXOSX__
        borderColour = bgColour.ChangeLightness(80);
#else
        borderColour = bgColour.ChangeLightness(50);
#endif
    }

    if(isDisabled) {
        bgColour = bgColour.ChangeLightness(110);
        borderColour = borderColour.ChangeLightness(110);
    }

    // Draw the background
    if(!isDisabled) {
        if(IsNormal() || IsHover()) {
            // fill the button bg colour with gradient
            dc.GradientFillLinear(rect, bgColour, m_colours.GetBgColour(), wxNORTH);
            // draw the border
            dc.SetPen(borderColour);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRoundedRectangle(rect, BUTTON_RADIUS);
        } else if(m_state == eButtonState::kPressed) {
            // pressed button is drawns with flat bg colour and border
            wxColour pressedBgColour = bgColour.ChangeLightness(90);
            dc.SetPen(borderColour);
            dc.SetBrush(pressedBgColour);
            dc.DrawRoundedRectangle(rect, BUTTON_RADIUS);
        }
    } else {
        // Draw the button border
        dc.SetPen(borderColour);
        dc.SetBrush(bgColour);
        dc.DrawRoundedRectangle(rect, BUTTON_RADIUS);
    }

    if(!isDisabled && !IsPressed()) {
        wxColour topLineColour = bgColour.ChangeLightness(130);
        wxRect rr = rect;
        rr.Deflate(2);
        dc.SetPen(topLineColour);
        dc.DrawLine(rr.GetTopLeft(), rr.GetTopRight());
    }

    // Draw the text
    wxRect textBoundingRect = rect;
    wxRect arrowRect;
    const int arrowSize = GetSize().GetHeight();
    if(HasDropDownMenu()) {
        arrowRect.SetX(rect.GetRight() - arrowSize);
        arrowRect.SetY(rect.GetY());
        arrowRect.SetWidth(arrowSize);
        arrowRect.SetHeight(arrowSize);
        arrowRect = arrowRect.CenterIn(rect, wxVERTICAL);
        textBoundingRect.SetWidth(textBoundingRect.GetWidth() - arrowRect.GetWidth());
    }
    if(!GetText().IsEmpty()) {
        dc.SetFont(DrawingUtils::GetDefaultGuiFont());
        dc.SetTextForeground(isDisabled ? m_colours.GetGrayText() : m_colours.GetItemTextColour());
        wxRect textRect = dc.GetTextExtent(GetText());
        textRect.x += TEXT_SPACER;
        textRect = textRect.CenterIn(textBoundingRect);

        // Adjust the text rect so it wont overflow the button
        if(textRect.GetLeft() < TEXT_SPACER) { textRect.SetX(TEXT_SPACER); }
        if(textRect.GetRight() > textBoundingRect.GetRight()) { textRect.SetRight(textBoundingRect.GetRight()); }

        // Set a clipping region
        wxString fixedText;
        DrawingUtils::TruncateText(GetText(), textRect.GetWidth(), dc, fixedText);
        dc.SetClippingRegion(textBoundingRect);

        // Truncate the text to fit the drawing area
        dc.DrawText(fixedText, textRect.GetTopLeft());
        dc.DestroyClippingRegion();
    }

    if(HasDropDownMenu()) {
        // Draw an arrow
        wxRect drawArrowRect = arrowRect;
        arrowRect.SetHeight(10);
        arrowRect.SetWidth(16);
        arrowRect = arrowRect.CenterIn(drawArrowRect);
        dc.SetPen(wxPen(isDisabled ? m_colours.GetGrayText() : m_colours.GetDarkBorderColour(), 3));
        wxPoint downCenterPoint =
            wxPoint(arrowRect.GetBottomLeft().x + arrowRect.GetWidth() / 2, arrowRect.GetBottom());
        dc.DrawLine(arrowRect.GetTopLeft(), downCenterPoint);
        dc.DrawLine(arrowRect.GetTopRight(), downCenterPoint);
    }
}

void clButtonBase::OnEnter(wxMouseEvent& event)
{
    event.Skip();
    if(!HasCapture()) {
        m_state = eButtonState::kHover;
        Refresh();
    }
}

void clButtonBase::OnLeave(wxMouseEvent& event)
{
    event.Skip();
    if(!HasCapture()) {
        m_state = eButtonState::kNormal;
        Refresh();
    }
}

wxSize clButtonBase::GetBestSize() const
{
    wxString sampleText;
    if(m_buttonStyle & wxBU_EXACTFIT) {
        sampleText = m_text.IsEmpty() ? "TTTppp" : m_text;
    } else {
        sampleText = "TTTpppTTTpp";
    }

    wxBitmap bmp(1, 1);
    wxMemoryDC memDC(bmp);
    wxGCDC dc(memDC);
    dc.SetFont(DrawingUtils::GetDefaultGuiFont());
    wxRect rectSize = dc.GetTextExtent(sampleText);
    wxRect textSize = dc.GetTextExtent(m_text);

    // If the text does not fit into the default button size, increase it
    if(textSize.GetWidth() > rectSize.GetWidth()) { rectSize = textSize; }
    rectSize.Inflate(8);

    // the arrow size is the same as the height
    if(HasDropDownMenu()) { rectSize.SetWidth(textSize.GetWidth() + rectSize.GetHeight()); }
    if(rectSize.GetWidth() < GetSize().GetWidth()) { rectSize.SetWidth(GetSize().GetWidth()); }
    rectSize.width += TEXT_SPACER;
    return rectSize.GetSize();
}

void clButtonBase::SetColours(const clColours& colours)
{
    this->m_colours = colours;
    Refresh();
}

void clButtonBase::OnFocus(wxFocusEvent& event)
{
    event.Skip();
    Refresh();
}

void clButtonBase::OnKeyDown(wxKeyEvent& event)
{
    event.Skip();
    if((event.GetKeyCode() == WXK_SPACE) || (event.GetKeyCode() == WXK_NUMPAD_ENTER) ||
       (event.GetKeyCode() == WXK_RETURN)) {
        PostClickEvent();
        event.Skip(false);
    } else if(event.GetKeyCode() == WXK_TAB) {
        Navigate(event.ShiftDown() ? wxNavigationKeyEvent::IsBackward : wxNavigationKeyEvent::IsForward);
    }
}

void clButtonBase::PostClickEvent()
{
    wxCommandEvent eventClick(wxEVT_BUTTON);
    eventClick.SetEventObject(this);
    GetEventHandler()->AddPendingEvent(eventClick);
}

void clButtonBase::OnIdle(wxIdleEvent& event)
{
    event.Skip();
    size_t flags = GetDrawingFlags();
    if(flags != m_lastPaintFlags) {
        // We need to refresh the window
        Refresh();
    }
}

size_t clButtonBase::GetDrawingFlags() const
{
    size_t flags = 0;
    if(IsEnabled()) { flags |= kDrawingFlagEnabled; }
    return flags;
}

void clButtonBase::SetText(const wxString& text)
{
    // strip menemonics, not very efficient...
    wxString tmp = text;
    tmp.Replace("&&", "@@");
    tmp.Replace("&", "");
    tmp.Replace("@@", "&");
    m_text = tmp;
}

void clButtonBase::SetDefault() {}

void clButtonBase::OnSize(wxSizeEvent& event)
{
    event.Skip();
    Refresh();
}

void clButtonBase::SetHasDropDownMenu(bool hasDropDownMenu)
{
    m_hasDropDownMenu = hasDropDownMenu;
    Refresh();
}
