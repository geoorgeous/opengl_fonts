#ifndef GFX_TEXTURE_H
#define GFX_TEXTURE_H

#include "common_types.h"

struct gfx_texture_st;

struct gfx_texture_st* gfx_texture_create();
void                   gfx_texture_destroy(struct gfx_texture_st** texture);
void                   gfx_texture_bind_to_device(const struct gfx_texture_st* texture);
void                   gfx_texture_set_data(struct gfx_texture_st* texture, size_ty width, size_ty height, void* data);

#endif