#pragma once

#include "chai3d.h"
#include "GEL3D.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //for matrices
#include <glm/gtc/type_ptr.hpp>

struct Spring {
    int p1, p2;
    float rest_length;
    float Ks, Kd;
    int type;
};

//------------------------------------------------------------------------------
// DECLARED GRAPHICS FUNCTIONS
//------------------------------------------------------------------------------

// draw lines of the grid
void drawGrid();

// initialize scene
void initCloth();