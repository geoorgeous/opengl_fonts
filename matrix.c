#include "matrix.h"

struct mat4_st mat4_identity() {
    return (struct mat4_st) { {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    } };
}

struct mat4_st mat4_transform_t(float x, float y, float z) {
    return (struct mat4_st) { {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
    } };
}

struct mat4_st mat4_transform_s(float x, float y, float z) {
    return (struct mat4_st) { {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    } };
}

struct mat4_st mat4_orthographic(float left, float right, float top, float bottom, float z_near, float z_far) {
    struct mat4_st mat4 = { 0 };
	mat4.elements[0]  =  2.0f / (right - left);
	mat4.elements[5]  =  2.0f / (top - bottom);
	mat4.elements[10] = -2.0f / (z_far - z_near);
	mat4.elements[12] = -((right + left) / (right - left));
	mat4.elements[13] = -((top + bottom) / (top - bottom));
	mat4.elements[14] = -((z_far + z_near) / (z_far - z_near));
	mat4.elements[15] =  1.0f;
    return mat4;
}