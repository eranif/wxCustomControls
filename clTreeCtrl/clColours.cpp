#include "clColours.h"
#include <wx/gdicmn.h>
#include <wx/settings.h>

void clColours::InitDefaults()
{
    itemTextColour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    selItemTextColour = wxColour("#FDFEFE");
    selItemBgColour = wxColour("#79BAEC");
    selbuttonColour = selItemTextColour;
    buttonColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
    hoverBgColour = wxColour("rgb(219,221,224)");
    bgColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
    scrolBarButton = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
    itemBgColour = bgColour;
    alternateColourEven = bgColour;
    alternateColourOdd = bgColour.ChangeLightness(95);
    headerBgColour = wxColour("#FBFCFC");
    headerHBorderColour = wxColour("#D0D3D4");
    headerVBorderColour = wxColour("#D0D3D4");
}

void clColours::InitDarkDefaults()
{
    bgColour = wxColour("#5F6A6A");
    itemTextColour = wxColour("#F4F6F6");
    selItemTextColour = *wxWHITE;
    selItemBgColour = wxColour("#6D7B8D");
    selbuttonColour = selItemTextColour;
    buttonColour = itemTextColour;
    hoverBgColour = wxColour("#717D7E");
    itemBgColour = bgColour;
    scrolBarButton = selItemBgColour;
    alternateColourEven = bgColour;
    alternateColourOdd = bgColour.ChangeLightness(95);
    headerBgColour = wxColour("#4D5656");
    headerHBorderColour = wxColour("#839192");
    headerVBorderColour = wxColour("#BFC9CA");
}
