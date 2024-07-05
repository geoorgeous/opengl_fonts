#ifndef GFX_MESH_H
#define GFX_MESH_H

#include "common_types.h"

enum gfx_vertex_attribute_type_en {
    GFX_VERTEX_ATTRIBUTE_TYPE_INT8,
    GFX_VERTEX_ATTRIBUTE_TYPE_UINT8,
    GFX_VERTEX_ATTRIBUTE_TYPE_INT16,
    GFX_VERTEX_ATTRIBUTE_TYPE_UINT16,
    GFX_VERTEX_ATTRIBUTE_TYPE_UINT32,
    GFX_VERTEX_ATTRIBUTE_TYPE_FLOAT32
};

struct gfx_vertex_attribute_st {
    bool_ty b_is_integer_storage;
    bool_ty b_is_normalised;
    size_ty num_components;
    enum gfx_vertex_attribute_type_en type;
};

struct array_st;

struct gfx_mesh_data_st {
    struct array_st* attributes;
    size_ty num_vertices;
    void* vertices;
    size_ty num_indices;
    size_ty* indices;
};

struct gfx_mesh_st;

struct gfx_mesh_st* gfx_mesh_create();
void                gfx_mesh_destroy(struct gfx_mesh_st** mesh);
void                gfx_mesh_set_data(struct gfx_mesh_st* mesh, const struct gfx_mesh_data_st* data);
void                gfx_mesh_draw(const struct gfx_mesh_st* mesh);

#endif