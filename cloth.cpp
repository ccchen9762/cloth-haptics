//------------------------------------------------------------------------------
// DECLARED GRAPHICS VARIABLES
//------------------------------------------------------------------------------

#include "cloth.h"

//------------------------------------------------------------------------------
// DECLARED GRAPHICS VARIABLES
//------------------------------------------------------------------------------

// grid size
int numX = 20, numY = 20;
size_t total_points = (numX + 1) * (numY + 1);
float fullsize = 4.0f;
float halfsize = fullsize / 2.0f;

int selected_index = -1;

std::vector<GLushort> indices;

std::vector<glm::vec3> X;

int oldX = 0, oldY = 0;

GLint viewport[4];
GLdouble MV[16];
GLdouble P[16];

//------------------------------------------------------------------------------

void initCloth() {

    int i = 0, j = 0, count = 0;
    int v = numY + 1;
    int u = numX + 1;

    indices.resize(numX * numY * 2 * 3);
    X.resize(total_points);

    // fill in X
    for (j = 0; j < v; j++) {
        for (i = 0; i < u; i++) {
            X[count++] = glm::vec3(((float(i) / (u - 1)) * 2 - 1) * halfsize / 5, -0.2f, ((float(j) / (v - 1)) * 2 - 1) * halfsize / 5);
        }
    }

    // fill in indices
    GLushort* id = &indices[0];
    for (i = 0; i < numY; i++) {
        for (j = 0; j < numX; j++) {
            int i0 = i * (numX + 1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (numX + 1);
            int i3 = i2 + 1;
            if ((j + i) % 2) {
                *id++ = i0; *id++ = i2; *id++ = i1;
                *id++ = i1; *id++ = i2; *id++ = i3;
            }
            else {
                *id++ = i0; *id++ = i2; *id++ = i3;
                *id++ = i0; *id++ = i3; *id++ = i1;
            }
        }
    }
}