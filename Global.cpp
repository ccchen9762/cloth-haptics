//#include "Global.h"

#include "chai3d.h"

std::string resourceRoot = "";
//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
chai3d::cStereoMode kStereoMode = chai3d::C_STEREO_DISABLED;

// fullscreen mode
bool kFullscreen = false;

// mirrored display
bool kMirroredDisplay = false;