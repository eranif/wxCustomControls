#ifndef CLCAPTIONBAR_HPP
#define CLCAPTIONBAR_HPP

#include "wxCustomControls.hpp"
#include <clColours.h>
#include <codelite_exports.h>
#include <unordered_map>
#include <vector>
#include <wx/control.h>
#include <wx/dc.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/toplevel.h>

enum wxCaptionStyle {
    wxCAPTION_BOLD_FONT = (1 << 0),
    wxCAPTION_CLOSE_BUTTON = (1 << 1),
    wxCAPTION_MINIMIZE_BUTTON = (1 << 2),
    wxCAPTION_MAXIMIZE_BUTTON = (1 << 3),
};

enum wxCaptionHitTest {
    wxCAPTION_HT_NOWHERE = -1,
    wxCAPTION_HT_ICON,
    wxCAPTION_HT_CLOSEBUTTON,
    wxCAPTION_HT_MINIMIZEBUTTON,
    wxCAPTION_HT_MAXMIZEBUTTON,
    wxCAPTION_HT_MENUBUTTON,
};

enum wxCaptionButtonState {
    wxCAPTION_BUTTON_NORMAL,
    wxCAPTION_BUTTON_PRESSED,
    wxCAPTION_BUTTON_HOVER,
};

class clCaptionBar;
class WXDLLIMPEXP_SDK clCaptionButton
{
    wxRect m_rect;
    wxRect m_innerRect;
    size_t m_state = wxCAPTION_BUTTON_NORMAL;
    clCaptionBar* m_captionBar = nullptr;

public:
    clCaptionButton(clCaptionBar* captionBar) { m_captionBar = captionBar; }
    ~clCaptionButton() {}

    bool IsPressed() const { return m_state & wxCAPTION_BUTTON_PRESSED; }
    bool IsHover() const { return m_state & wxCAPTION_BUTTON_HOVER; }
    bool IsNormal() const { return m_state & wxCAPTION_BUTTON_NORMAL; }
    bool Contains(const wxPoint& pt) const { return m_rect.Contains(pt); }
    void SetRect(const wxRect& rect)
    {
        m_rect = rect;
        m_innerRect = rect;
        m_innerRect.SetWidth(m_innerRect.GetWidth() / 3);
        m_innerRect.SetHeight(m_innerRect.GetHeight() / 3);
        m_innerRect = m_innerRect.CenterIn(rect);
    }

    const wxRect& GetRect() const { return m_rect; }
    const wxRect& GetInnerRect() const { return m_innerRect; }

    void Clear() { m_rect = {}; }
    /**
     * @brief change the button state, return true if the state actually changed
     */
    bool ResetState()
    {
        auto old_state = m_state;
        m_state = wxCAPTION_BUTTON_NORMAL;
        return old_state != m_state;
    }

    /**
     * @brief do "hover". return true if the state was changed
     */
    bool DoHover(const wxPoint& pt)
    {
        auto old_state = m_state;
        if(m_rect.Contains(pt)) {
            m_state = wxCAPTION_BUTTON_HOVER;
        } else {
            m_state = wxCAPTION_BUTTON_NORMAL;
        }
        return old_state != m_state;
    }

    void LeftDown(wxCaptionHitTest where);
    void LeftUp(wxCaptionHitTest where);
    void Render(wxDC& dc, wxCaptionHitTest ht);
};

class WXDLLIMPEXP_SDK clCaptionBar : public wxWindow
{
    friend class clCaptionButton;
    typedef void (clCaptionButton::*MouseCallback)(wxCaptionHitTest);
    typedef std::unordered_map<wxCaptionHitTest, std::pair<clCaptionButton*, MouseCallback>> CallbackMap_t;

protected:
    clColours m_colours;
    wxPoint m_delta;
    wxTopLevelWindow* m_topLevelWindow = nullptr;
    wxString m_caption;
    wxBitmap m_bitmap;
    wxRect m_bitmapRect;

    // buttons
    clCaptionButton m_buttonMenu;
    clCaptionButton m_buttonClose;
    clCaptionButton m_buttonMinimize;
    clCaptionButton m_buttonMaximize;
    size_t m_flags = 0;

    CallbackMap_t m_leftDownCallbacks;
    CallbackMap_t m_leftUpCallbacks;

protected:
    wxCaptionHitTest HitTest(const wxPoint& pt) const;
    void DoSetBestSize();
    void ClearRects();
    bool ProcessCallback(const CallbackMap_t& map, wxCaptionHitTest where);

protected:
    void OnPaint(wxPaintEvent& e);
    void OnEraseBg(wxEraseEvent& e);
    void OnLeftDown(wxMouseEvent& e);
    void OnLeftUp(wxMouseEvent& e);
    void OnMotion(wxMouseEvent& e);
    void OnEnterWindow(wxMouseEvent& e);
    void OnLeaveWindow(wxMouseEvent& e);
    void OnSize(wxSizeEvent& e);
    void OnMouseDoubleClick(wxMouseEvent& e);

public:
    clCaptionBar(wxWindow* parent, wxTopLevelWindow* topLevelFrame);
    virtual ~clCaptionBar();

    void SetBitmap(const wxBitmap& bitmap);
    void SetCaption(const wxString& caption);

    const wxBitmap& GetBitmap() const { return m_bitmap; }
    const wxString& GetCaption() const { return m_caption; }
    // Customisation point: allow user to set colours
    void SetColours(const clColours& colours);
    const clColours& GetColours() const { return m_colours; }
    void SetOption(wxCaptionStyle option, bool enabled);
    /**
     * @brief set options from the wxCaptionStyle
     */
    void SetOptions(size_t options);
    bool HasOption(wxCaptionStyle option) const { return m_flags & option; }
};
#endif // CLCAPTIONBAR_HPP
