#include "clSideBarCtrl.hpp"
#include "wx/anybutton.h"
#include "wx/dcbuffer.h"
#include <wx/sizer.h>

thread_local clSideBarCtrl::ButtonId BUTTON_ID = 0;

class SideBarButton : public wxControl
{
protected:
    clSideBarCtrl* m_sidebar = nullptr;
    wxBitmap m_bmp;
    bool m_selected = false;

public:
    explicit SideBarButton(clSideBarCtrl* parent, const wxBitmap& bmp)
        : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
        , m_sidebar(parent)
        , m_bmp(bmp)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetCursor(wxCURSOR_HAND);

        wxRect rr = m_bmp.GetScaledSize();
        rr.Inflate(10);
        SetSizeHints(rr.GetWidth(), rr.GetHeight());
        SetSize(rr.GetWidth(), rr.GetHeight());

        Bind(wxEVT_PAINT, &SideBarButton::OnPaint, this);
        Bind(wxEVT_ERASE_BACKGROUND, &SideBarButton::OnEraseBg, this);
        Bind(wxEVT_LEFT_DOWN, &SideBarButton::OnLeftDown, this);
    }

    virtual ~SideBarButton()
    {
        Unbind(wxEVT_PAINT, &SideBarButton::OnPaint, this);
        Unbind(wxEVT_ERASE_BACKGROUND, &SideBarButton::OnEraseBg, this);
        Unbind(wxEVT_LEFT_DOWN, &SideBarButton::OnLeftDown, this);
    }

    void OnLeftDown(wxMouseEvent& event)
    {
        event.Skip();
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

        // TODO: fire event for selection changed
    }

    void OnPaint(wxPaintEvent& event)
    {
        wxUnusedVar(event);
        wxBufferedPaintDC dc(this);

        wxRect client_rect = GetClientRect();

        wxColour colour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
        if(IsSeleced()) {
            if(DrawingUtils::IsDark(colour)) {
                colour = colour.ChangeLightness(50);
            } else {
                colour = colour.ChangeLightness(160);
            }
        }
        dc.SetBrush(colour);
        dc.SetPen(colour);
        dc.DrawRectangle(client_rect);

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
