#ifndef CLCOMBOBOX_HPP
#define CLCOMBOBOX_HPP

#include "codelite_exports.h"
#include <wx/control.h>
#include <wx/combobox.h>
#include "clButton.h"

class WXDLLIMPEXP_SDK clComboBox : public wxControl
{
    wxArrayString m_choices;
    wxTextCtrl* m_textCtrl = nullptr;
    clButton* m_button = nullptr;
    wxString m_value;

protected:
    void DoCreate();

protected:
    void OnButtonClicked(wxCommandEvent& event);

public:
    clComboBox();
    clComboBox(wxWindow* parent, wxWindowID id, const wxString& value = wxEmptyString,
               const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, size_t n = 0,
               const wxString choices[] = NULL, long style = 0, const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxComboBoxNameStr);
    clComboBox(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size,
               const wxArrayString& choices, long style = 0, const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxComboBoxNameStr);
    bool Create(wxWindow* parent, wxWindowID id, const wxString& value = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, size_t n = 0,
                const wxString choices[] = NULL, long style = 0, const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxComboBoxNameStr);
    bool Create(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size,
                const wxArrayString& choices, long style = 0, const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxComboBoxNameStr);
    virtual ~clComboBox();
    
    /**
     * @brief set text hint
     */
    void SetHint(const wxString& hint);
    
    /**
     * @brief select item from the list of choices
     */
    void SetSelection(size_t sel);
};

#endif // CLCOMBOBOX_HPP
