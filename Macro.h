#pragma once

#include <string>

extern std::string resourceRoot;

//---------------------------------------------------------------------------
// DECLARED MACROS
//---------------------------------------------------------------------------

// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+std::string(p)).c_str())