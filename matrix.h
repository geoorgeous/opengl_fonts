#ifndef MATRIX_H
#define MATRIX_H

struct mat4_st {
    float elements[16];
};

struct mat4_st mat4_identity();

struct mat4_st mat4_transform_t(float x, float y, float z);

struct mat4_st mat4_transform_s(float x, float y, float z);

struct mat4_st mat4_orthographic(float left, float right, float top, float bottom, float z_near, float z_far);

struct mat4_st mat4_perspective(float vertical_fov_radians, float aspect_ratio, float z_near, float z_far);

#endif