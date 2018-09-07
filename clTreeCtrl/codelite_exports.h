#ifndef CODELITE_EXPORTS_H
#define CODELITE_EXPORTS_H

#ifndef CODELITE_EXPORTS
#define CODELITE_EXPORTS

#ifdef __WXMSW__

#include <wx/string.h>

//////////////////////////////////////
// SDK
//////////////////////////////////////
#ifdef WXDLLIMPEXP_SDK
#undef WXDLLIMPEXP_SDK
#endif

#ifdef WXMAKINGDLL_SDK
#define WXDLLIMPEXP_SDK __declspec(dllexport)
#elif defined(WXUSINGDLL_SDK)
#define WXDLLIMPEXP_SDK __declspec(dllimport)
#else // not making nor using DLL
#define WXDLLIMPEXP_SDK
#endif

#else // !MSW
#define WXDLLIMPEXP_CL
#define WXDLLIMPEXP_SDK
#endif

#endif // CODELITE_EXPORTS

#endif // CODELITE_EXPORTS_H
