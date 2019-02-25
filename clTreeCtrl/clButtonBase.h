#ifndef CLBUTTONBASE_H
#define CLBUTTONBASE_H

#include "codelite_exports.h"
#include <wx/panel.h>
#include <wx/validate.h>
#include "clColours.h"
#include "drawingutils.h"
#include <wx/control.h>

class WXDLLIMPEXP_SDK clButtonBase : public wxControl
{
    size_t m_buttonStyle = 0;
    clColours m_colours;
    wxString m_text;
    eButtonState m_state = eButtonState::kNormal;
    enum eDrawingState {
        kEnabled,
        kDisabled,
    };
    eDrawingState m_drawingState = kEnabled;

protected:
    void BindEvents();
    void UnBindEvents();
    void Initialise();
    wxSize GetBestSize() const;
    void PostClickEvent();
    clButtonBase::eDrawingState GetDrawingState() const;

protected:
    // Event handlers
    void OnPaint(wxPaintEvent& event);
    void OnErasebg(wxEraseEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnEnter(wxMouseEvent& event);
    void OnLeave(wxMouseEvent& event);
    void OnFocus(wxFocusEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnIdle(wxIdleEvent& event);

    // Overridable
    virtual void Render(wxDC& dc);

public:
    clButtonBase();
    clButtonBase(wxWindow* parent, wxWindowID id, const wxString& label = wxEmptyString,
                 const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
                 const wxValidator& validator = wxDefaultValidator, const wxString& name = "button");
    bool Create(wxWindow* parent, wxWindowID id, const wxString& label = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
                const wxValidator& validator = wxDefaultValidator, const wxString& name = "button");
    virtual ~clButtonBase();
    void SetColours(const clColours& colours);
    const clColours& GetColours() const { return m_colours; }
    void SetText(const wxString& text) { this->m_text = text; }
    const wxString& GetText() const { return m_text; }
};

#endif // CLBUTTONBASE_H
