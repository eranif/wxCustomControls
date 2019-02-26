#include "clChoice.h"

clChoice::clChoice() {}

clChoice::~clChoice() {}

bool clChoice::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                      const wxArrayString& choices, long style, const wxValidator& validator, const wxString& name)
{
    if(!clButtonBase::Create(parent, id, "TTTTTTTTpppppppppp", pos, size, 0, validator, name)) { return false; }
    return true;
}

clChoice::clChoice(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                   const wxArrayString& choices, long style, const wxValidator& validator, const wxString& name)
{
    Create(parent, id, pos, size, choices, 0, validator, name);
}

int clChoice::FindString(const wxString& s, bool caseSensitive) const
{
    for(size_t i = 0; i < m_choices.size(); ++i) {
        if(caseSensitive) {
            if(m_choices[i] == s) { return i; }
        } else {
            if(s.CmpNoCase(m_choices[i]) == 0) { return i; }
        }
    }
    return wxNOT_FOUND;
}
