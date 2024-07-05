#ifndef GFX_SHADER_H
#define GFX_SHADER_H

#include "common_types.h"

struct gfx_shader_st;
struct mat4_st;

struct gfx_shader_st* gfx_shader_create();
void                  gfx_shader_destroy(struct gfx_shader_st** shader);
bool_ty               gfx_shader_compile(struct gfx_shader_st* shader, const char* vertex_stage_source, const char* fragment_stage_source);
bool_ty               gfx_shader_is_compiled(const struct gfx_shader_st* shader);
void                  gfx_shader_make_current(const struct gfx_shader_st* shader);
void                  gfx_shader_set_uniform_int(const struct gfx_shader_st* shader, const char* uniform_id, int i);
void                  gfx_shader_set_uniform_mat4(const struct gfx_shader_st* shader, const char* uniform_id, const struct mat4_st* matrix);

#endif