#ifndef CLCHOICE_H
#define CLCHOICE_H

#include <wx/choice.h>
#include "clButtonBase.h" // Base class: clButtonBase
#include <wx/arrstr.h>
#include <vector>
#include <string>

class WXDLLIMPEXP_SDK clChoice : public clButtonBase
{
    std::vector<wxString> m_choices;
    int m_selection = wxNOT_FOUND;

public:
    clChoice();
    clChoice(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, const wxArrayString& choices,
             long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxChoiceNameStr);
    bool Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, const wxArrayString& choices,
                long style = 0, const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxChoiceNameStr);
    virtual ~clChoice();

    /**
     * @brief Finds an item whose label matches the given string.
     */
    int FindString(const wxString& s, bool caseSensitive = false) const;
};

#endif // CLCHOICE_H
