#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "font.h"
#include "gfx_mesh.h"
#include "gfx_text.h"
#include "hash_table.h"

#include "common_types.h"

struct gfx_mesh_data_st gfx_text_generate_mesh_data(const struct glyph_texture_atlas_st* glyph_texture_atlas, const char* text, float color[4]) {
    size_ty num_quads = strlen(text) * 4;
    float* vertices = malloc(num_quads * (4 * (3 + 2 + 4) * sizeof(float)));
    size_ty* indices = malloc(num_quads * 6 * sizeof(size_ty));
    num_quads = 0;

    struct font_face_metrics_st font_face_metrics = font_face_get_metrics(glyph_texture_atlas->source_font_face);

    float* vertex = vertices;
    size_ty* index = indices;
    const char* text_ptr = text;
    float pen_pos_x = 0.0f;
    float pen_pos_y = 0.0f;

    while (*text_ptr != '\0') {
        char ch = *text_ptr++;
        if (ch == ' ') {
            const struct glyph_texture_atlas_entry_st* glyph_texture_atlas_entry = hash_tablei_get(glyph_texture_atlas->glyph_entries, (size_ty)ch);
            struct glyph_metrics_st glyph_metrics = glyph_get_metrics(glyph_texture_atlas_entry->glyph);
            pen_pos_x += glyph_metrics.device_size.x;
            continue;
        }
        if (ch == '\n') {
            pen_pos_x = 0.0f;
            pen_pos_y += font_face_metrics.glyph_size.y;
            continue;
        }

        const struct glyph_texture_atlas_entry_st* glyph_texture_atlas_entry = hash_tablei_get(glyph_texture_atlas->glyph_entries, (size_ty)ch);
        const struct glyph_st* glyph = glyph_texture_atlas_entry->glyph;
        struct glyph_metrics_st glyph_metrics = glyph_get_metrics(glyph);
        float left = pen_pos_x + glyph_metrics.offset.x;
        float top = pen_pos_y - glyph_metrics.offset.y - glyph_metrics.size.y;

        *vertex++ = left;
        *vertex++ = top;
        *vertex++ = 0.0f;
        *vertex++ = glyph_texture_atlas_entry->uv_min_u;
        *vertex++ = glyph_texture_atlas_entry->uv_max_v;
        *vertex++ = color[0];
        *vertex++ = color[1];
        *vertex++ = color[2];
        *vertex++ = color[3];
        
        *vertex++ = left;
        *vertex++ = top + glyph_metrics.size.y;
        *vertex++ = 0.0f;
        *vertex++ = glyph_texture_atlas_entry->uv_min_u;
        *vertex++ = glyph_texture_atlas_entry->uv_min_v;
        *vertex++ = color[0];
        *vertex++ = color[1];
        *vertex++ = color[2];
        *vertex++ = color[3];
        
        *vertex++ = left + glyph_metrics.size.x;
        *vertex++ = top;
        *vertex++ = 0.0f;
        *vertex++ = glyph_texture_atlas_entry->uv_max_u;
        *vertex++ = glyph_texture_atlas_entry->uv_max_v;
        *vertex++ = color[0];
        *vertex++ = color[1];
        *vertex++ = color[2];
        *vertex++ = color[3];
        
        *vertex++ = left + glyph_metrics.size.x;
        *vertex++ = top + glyph_metrics.size.y;
        *vertex++ = 0.0f; // pos z
        *vertex++ = glyph_texture_atlas_entry->uv_max_u;
        *vertex++ = glyph_texture_atlas_entry->uv_min_v;
        *vertex++ = color[0];
        *vertex++ = color[1];
        *vertex++ = color[2];
        *vertex++ = color[3];

        *index++ = num_quads * 4 + 0;
        *index++ = num_quads * 4 + 1;
        *index++ = num_quads * 4 + 2;
        *index++ = num_quads * 4 + 2;
        *index++ = num_quads * 4 + 1;
        *index++ = num_quads * 4 + 3;
        
        ++num_quads;

        pen_pos_x += glyph_metrics.device_size.x;
    }

    // Shrink now we know exact number of vertices
    vertices = realloc(vertices, num_quads * (4 * (3 + 2 + 4) * sizeof(float)));
    indices = realloc(indices, num_quads * 6 * sizeof(size_ty));

    struct gfx_mesh_data_st mesh_data;

    mesh_data.attributes = array_create(sizeof(struct gfx_vertex_attribute_st));
    array_set_capacity(mesh_data.attributes, 3);
    struct gfx_vertex_attribute_st* attribute = array_push(mesh_data.attributes);
    attribute->b_is_integer_storage = b_FALSE;
    attribute->b_is_normalised = b_FALSE;
    attribute->num_components = 3;
    attribute->type = GFX_VERTEX_ATTRIBUTE_TYPE_FLOAT32;
    attribute = array_push(mesh_data.attributes);
    attribute->b_is_integer_storage = b_FALSE;
    attribute->b_is_normalised = b_FALSE;
    attribute->num_components = 2;
    attribute->type = GFX_VERTEX_ATTRIBUTE_TYPE_FLOAT32;
    attribute = array_push(mesh_data.attributes);
    attribute->b_is_integer_storage = b_FALSE;
    attribute->b_is_normalised = b_FALSE;
    attribute->num_components = 4;
    attribute->type = GFX_VERTEX_ATTRIBUTE_TYPE_FLOAT32;

    mesh_data.num_vertices = num_quads * 4;
    mesh_data.vertices = vertices;

    mesh_data.num_indices = num_quads * 6;
    mesh_data.indices = indices;

    return mesh_data;
}