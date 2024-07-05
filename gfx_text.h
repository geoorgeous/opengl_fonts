#ifndef GFX_TEXT_H
#define GFX_TEXT_H

#include "gfx_mesh.h"

struct font_face_st;

struct gfx_mesh_data_st gfx_text_generate_mesh_data(const struct glyph_texture_atlas_st* glyph_texture_atlas, const char* text, float color[4]);

#endif