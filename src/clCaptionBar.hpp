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

class WXDLLIMPEXP_SDK clCaptionBar : public wxWindow
{
protected:
    clColours m_colours;
    wxPoint m_delta;
    wxTopLevelWindow* m_topLevelWindow = nullptr;
    wxString m_caption;
    wxBitmap m_bitmap;

protected:
    int HitTest(const wxPoint& pt) const;
    void DoSetBestSize();

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
    clCaptionBar();
    clCaptionBar(wxWindow* parent, wxTopLevelWindow* topLevelFrame);
    virtual ~clCaptionBar();

    void SetBitmap(const wxBitmap& bitmap);
    void SetCaption(const wxString& caption);

    const wxBitmap& GetBitmap() const { return m_bitmap; }
    const wxString& GetCaption() const { return m_caption; }
    // Customisation point: allow user to set colours
    void SetColours(const clColours& colours);
    const clColours& GetColours() const { return m_colours; }
};
#endif // CLCAPTIONBAR_HPP
