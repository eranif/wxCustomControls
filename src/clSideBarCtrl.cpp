#include "clSideBarCtrl.hpp"
#include "wx/anybutton.h"
#include "wx/dcbuffer.h"
#include "wx/log.h"
#include <wx/sizer.h>

thread_local clSideBarCtrl::ButtonId BUTTON_ID = 0;

class SideBarButton : public wxControl
{
protected:
    clSideBarCtrl* m_sidebar = nullptr;
    wxBitmap m_bmp;
    bool m_selected = false;
    wxDateTime m_dragStartTime;
    wxPoint m_dragStartPos;
    bool m_dragging = false;

protected:
    void DoDrop()
    {
        wxPoint pt = ::wxGetMousePosition();
        SideBarButton* target_button = nullptr;
        bool move_after = false;
        for(auto [id, tab] : m_sidebar->m_button_id_map) {
            auto target_tab_rect = tab->GetScreenRect();

            auto target_tab_upper_rect = target_tab_rect;
            target_tab_upper_rect.SetHeight(target_tab_rect.GetHeight() / 2);

            auto target_tab_lower_rect = target_tab_rect;
            target_tab_lower_rect.SetHeight(target_tab_rect.GetHeight() / 2);
            target_tab_lower_rect.SetY(target_tab_rect.GetY() + (target_tab_rect.GetHeight() / 2));

            if(target_tab_upper_rect.Contains(pt)) {
                // this is our target tab
                if(tab != this) {
                    target_button = tab;
                    move_after = false;
                }
                break;
            } else if(target_tab_lower_rect.Contains(pt)) {
                // this is our target tab
                if(tab != this) {
                    target_button = tab;
                    move_after = true;
                }
                break;
            }
        }

        if(target_button) {
            if(move_after) {
                m_sidebar->CallAfter(&clSideBarCtrl::MoveAfter, this, target_button);
            } else {
                m_sidebar->CallAfter(&clSideBarCtrl::MoveBefore, this, target_button);
            }
        }
    }

    void DoCancelDrag()
    {
        if(m_dragging) {
            // locate the target tab
            DoDrop();
        }
        m_dragStartTime.Set((time_t)-1); // Reset the saved values
        m_dragStartPos = wxPoint();
        SetCursor(wxCURSOR_HAND);
        m_dragging = false;
        if(HasCapture()) {
            ReleaseMouse();
        }
    }

    void DoBeginDrag()
    {
        // Change the cursor indicating DnD in progress
        SetCursor(wxCURSOR_SIZING);
        CaptureMouse();
        m_dragging = true;
    }

public:
    explicit SideBarButton(clSideBarCtrl* parent, const wxBitmap& bmp)
        : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
        , m_sidebar(parent)
        , m_bmp(bmp)
    {
        m_dragStartTime = (time_t)-1;
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetCursor(wxCURSOR_HAND);

        wxRect rr = m_bmp.GetScaledSize();
        rr.Inflate(10);
        SetSizeHints(rr.GetWidth(), rr.GetHeight());
        SetSize(rr.GetWidth(), rr.GetHeight());

        Bind(wxEVT_PAINT, &SideBarButton::OnPaint, this);
        Bind(wxEVT_ERASE_BACKGROUND, &SideBarButton::OnEraseBg, this);
        Bind(wxEVT_LEFT_DOWN, &SideBarButton::OnLeftDown, this);
        Bind(wxEVT_MOTION, &SideBarButton::OnMotion, this);
        Bind(wxEVT_LEFT_UP, &SideBarButton::OnLeftUp, this);
    }

    virtual ~SideBarButton()
    {
        Unbind(wxEVT_PAINT, &SideBarButton::OnPaint, this);
        Unbind(wxEVT_ERASE_BACKGROUND, &SideBarButton::OnEraseBg, this);
        Unbind(wxEVT_LEFT_DOWN, &SideBarButton::OnLeftDown, this);
        Unbind(wxEVT_MOTION, &SideBarButton::OnMotion, this);
        Unbind(wxEVT_LEFT_UP, &SideBarButton::OnLeftUp, this);
    }

    void OnMotion(wxMouseEvent& event)
    {
        event.Skip();
        if(m_dragStartTime.IsValid() && event.LeftIsDown() &&
           !m_dragging) { // If we're tugging on the tab, consider starting D'n'D
            wxLogMessage("Moving");
            wxTimeSpan diff = wxDateTime::UNow() - m_dragStartTime;
            if(diff.GetMilliseconds() > 100 && // We need to check both x and y distances as tabs may be vertical
               ((abs(m_dragStartPos.x - event.GetX()) > 5) || (abs(m_dragStartPos.y - event.GetY()) > 5))) {
                DoBeginDrag(); // Sufficient time and distance since the LeftDown for a believable D'n'D start
            }
        }
    }

    void OnLeftUp(wxMouseEvent& event)
    {
        event.Skip();
        if(m_dragging) {
            DoCancelDrag();
        }
    }

    void OnLeftDown(wxMouseEvent& event)
    {
        event.Skip();
        if(!IsSeleced()) {
            for(auto [id, button] : m_sidebar->m_button_id_map) {
                wxUnusedVar(id);
                if(button->IsSeleced()) {
                    if(button == this) {
                        return;
                    }
                    button->SetSeleced(false);
                    button->Refresh();
                    break;
                }
            }
            SetSeleced(true);
            Refresh();
        } else {
            // Prepare to DnDclTreeCtrl_DnD
            if(event.LeftIsDown()) {
                m_dragStartTime = wxDateTime::UNow();
                m_dragStartPos = wxPoint(event.GetX(), event.GetY());
            }
        }
        // TODO: fire event for selection changed
    }

    void OnPaint(wxPaintEvent& event)
    {
        wxUnusedVar(event);
        wxBufferedPaintDC dc(this);

        wxRect client_rect = GetClientRect();

        wxColour colour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
        bool is_dark = DrawingUtils::IsDark(colour);
        if(IsSeleced()) {
            if(is_dark) {
                colour = colour.ChangeLightness(50);
            } else {
                colour = colour.ChangeLightness(160);
            }
        }
        dc.SetBrush(colour);
        dc.SetPen(colour);
        dc.DrawRectangle(client_rect);

        if(IsSeleced()) {
            auto pen_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
            dc.SetPen(pen_colour);
            dc.DrawLine(client_rect.GetTopLeft(), client_rect.GetTopRight());
            dc.DrawLine(client_rect.GetBottomLeft(), client_rect.GetBottomRight());

            if(m_sidebar->IsOrientationOnTheRight()) {
                dc.DrawLine(client_rect.GetTopRight(), client_rect.GetBottomRight());
            } else {
                dc.DrawLine(client_rect.GetTopLeft(), client_rect.GetBottomLeft());
            }
        }

        wxRect bmp_rect = wxRect(m_bmp.GetSize()).CenterIn(client_rect);
        dc.DrawBitmap(m_bmp, bmp_rect.GetTopLeft());
    }

    void OnEraseBg(wxEraseEvent& event) { wxUnusedVar(event); }

    void SetSeleced(bool b) { m_selected = b; }
    bool IsSeleced() const { return m_selected; }
};

clSideBarCtrl::clSideBarCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxControl(parent, id, pos, size, style | wxBORDER_NONE)
{
    m_mainSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_mainSizer);
}

clSideBarCtrl::~clSideBarCtrl() {}

clSideBarCtrl::ButtonId clSideBarCtrl::AddButton(const wxBitmap& bmp, bool select, const wxString& tooltip)
{
    SideBarButton* btn = new SideBarButton(this, bmp);
    btn->SetSeleced(select);
    btn->SetToolTip(tooltip);
    m_mainSizer->Add(btn, wxSizerFlags().CenterHorizontal());
    auto button_id = ++BUTTON_ID;
    m_button_id_map.insert({ button_id, btn });

    SetSizeHints(btn->GetSize().GetWidth(), wxNOT_FOUND);
    SetSize(btn->GetSize().GetWidth(), wxNOT_FOUND);
    GetParent()->GetSizer()->Layout();
    return button_id;
}

void clSideBarCtrl::DeleteButton(clSideBarCtrl::ButtonId button_id)
{
    if(m_button_id_map.count(button_id) == 0) {
        return;
    }
    auto button = m_button_id_map[button_id];
    m_mainSizer->Detach(button);
    button->Destroy();
    m_mainSizer->Layout();
}

void clSideBarCtrl::MoveAfter(SideBarButton* src, SideBarButton* target)
{
    if(src == target) {
        return;
    }
    m_mainSizer->Detach(src);
    // find the target button index
    size_t target_index = wxString::npos;
    for(size_t i = 0; i < m_mainSizer->GetItemCount(); ++i) {
        if(m_mainSizer->GetItem(i)->GetWindow() == target) {
            target_index = i;
            break;
        }
    }

    if(target_index == wxString::npos) {
        return;
    }

    if(target_index + 1 >= m_mainSizer->GetItemCount()) {
        m_mainSizer->Add(src);
    } else {
        m_mainSizer->Insert(target_index + 1, src);
    }
    m_mainSizer->Layout();
}

void clSideBarCtrl::MoveBefore(SideBarButton* src, SideBarButton* target)
{
    if(src == target) {
        return;
    }
    m_mainSizer->Detach(src);
    // find the target button index
    size_t target_index = wxString::npos;
    for(size_t i = 0; i < m_mainSizer->GetItemCount(); ++i) {
        if(m_mainSizer->GetItem(i)->GetWindow() == target) {
            target_index = i;
            break;
        }
    }

    if(target_index == wxString::npos) {
        return;
    }

    m_mainSizer->Insert(target_index, src);
    m_mainSizer->Layout();
}