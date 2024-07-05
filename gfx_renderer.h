#ifndef GFX_RENDERER_H
#define GFX_RENDERER_H

#include "matrix.h"

struct gfx_mesh_st;
struct gfx_renderer_st;
struct gfx_texture_st;

struct gfx_renderer_command_st {
    const struct gfx_texture_st* texture;
    const struct gfx_mesh_st* mesh;
    struct mat4_st transform;
};

struct gfx_renderer_st* gfx_renderer_create();
void                    gfx_renderer_destroy(struct gfx_renderer_st** renderer);
void                    gfx_renderer_push_command(struct gfx_renderer_st* renderer, const struct gfx_renderer_command_st* command);
void                    gfx_renderer_submit_commands(struct gfx_renderer_st* renderer, const struct mat4_st* camera_matrix);

#endif