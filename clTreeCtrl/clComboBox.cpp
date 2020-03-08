#include "clComboBox.hpp"
#include <wx/sizer.h>
#include <wx/button.h>

clComboBox::clComboBox(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size,
                       size_t n, const wxString choices[], long style, const wxValidator& validator,
                       const wxString& name)
    : wxControl(parent, id, pos, size, style)
{
    wxUnusedVar(validator);
    wxUnusedVar(name);
    Create(parent, id, value, pos, size, n, choices, style, validator, name);
}

clComboBox::clComboBox(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size,
                       const wxArrayString& choices, long style, const wxValidator& validator, const wxString& name)
    : wxControl(parent, id, pos, size, style)
{
    wxUnusedVar(validator);
    wxUnusedVar(name);
    Create(parent, id, value, pos, size, choices, style, validator, name);
}

clComboBox::clComboBox() {}

clComboBox::~clComboBox() {}

bool clComboBox::Create(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size,
                        size_t n, const wxString choices[], long style, const wxValidator& validator,
                        const wxString& name)
{
    wxUnusedVar(validator);
    wxUnusedVar(name);
    bool res = wxControl::Create(parent, id, pos, size, style);
    m_choices.reserve(n);
    for(size_t i = 0; i < n; ++i) {
        m_choices.push_back(choices[i]);
    }
    m_value = value;
    DoCreate();
    return res;
}

bool clComboBox::Create(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size,
                        const wxArrayString& choices, long style, const wxValidator& validator, const wxString& name)
{
    wxUnusedVar(validator);
    wxUnusedVar(name);
    bool res = wxControl::Create(parent, id, pos, size, style);
    m_choices = choices;
    m_value = value;
    DoCreate();
    return res;
}

void clComboBox::OnButtonClicked(wxCommandEvent& event) { wxUnusedVar(event); }

void clComboBox::DoCreate()
{
    SetSizer(new wxBoxSizer(wxHORIZONTAL));
    m_textCtrl = new wxTextCtrl(this, wxID_ANY, m_value);
    GetSizer()->Add(m_textCtrl, 1, wxEXPAND, 0);
    m_button = new clButton(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    m_button->SetHasDropDownMenu(true);
    m_button->SetText(""); // this will force size calculation
    Bind(wxEVT_BUTTON, &clComboBox::OnButtonClicked, this);
    GetSizer()->Add(m_button);
    GetSizer()->Fit(this);
}

void clComboBox::SetHint(const wxString& hint) { m_textCtrl->SetHint(hint); }

void clComboBox::SetSelection(size_t sel)
{
    if(m_choices.GetCount() >= sel) { return; }
    m_value = m_choices.Item(sel);
}
