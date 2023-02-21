#pragma once

#include "chai3d.h"

//------------------------------------------------------------------------------
#ifdef C_USE_OPENGL
#ifdef MACOSX
#include "OpenGL/glu.h"
#else
#include "GL/glu.h"
#endif
#endif
//------------------------------------------------------------------------------

class MultiCursor : public chai3d::cToolCursor
{
public:
	MultiCursor(chai3d::cWorld* a_parentWorld, const double& a_radius);
	~MultiCursor();

	// overload function to apply extra deformable force along with rigid force
	bool applyToDevice(chai3d::cVector3d deformableForce);

private:

};