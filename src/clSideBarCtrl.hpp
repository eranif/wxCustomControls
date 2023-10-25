#ifndef CLSIDEBARCTRL_HPP
#define CLSIDEBARCTRL_HPP

#include "clButton.h"
#include "codelite_exports.h"

#include <unordered_map>
#include <wx/bitmap.h>
#include <wx/control.h>

class SideBarButton;
class WXDLLIMPEXP_SDK clSideBarCtrl : public wxControl
{
    friend class SideBarButton;

public:
    typedef size_t ButtonId;

protected:
    wxSizer* m_mainSizer = nullptr;
    std::unordered_map<size_t, SideBarButton*> m_button_id_map;
    wxDirection m_orientation = wxLEFT;

protected:
    void MoveAfter(SideBarButton* src, SideBarButton* target);
    void MoveBefore(SideBarButton* src, SideBarButton* target);

public:
    clSideBarCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~clSideBarCtrl();

    /// Does the control placed on the left or right?
    void SetOrientationOnTheRight(bool b) { m_orientation = b ? wxRIGHT : wxLEFT; }
    bool IsOrientationOnTheRight() const { return m_orientation == wxRIGHT; }

    /// Add new button at the end, returns its unique ID
    ButtonId AddButton(const wxBitmap& bmp, bool select = false, const wxString& tooltip = wxEmptyString);

    /// Remove a button by ID
    void DeleteButton(ButtonId button_id);
};

#endif // CLSIDEBARCTRL_HPP