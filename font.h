#ifndef FONT_H
#define FONT_H

#include "common_types.h"

struct font_family_st;
struct font_face_st;
struct glyph_st;

enum font_style_en {
    FONT_STYLE_REGULAR,
    FONT_STYLE_BOLD,
    FONT_STYLE_ITALICS,
    FONT_STYLE_BOLD_ITALIC
};

struct font_face_metrics_st {
    int point_size;
    int ascent;
    int descent;
    struct point_st device_size;
    struct point_st glyph_size;
    struct point_st glyph_offset;
};

struct glyph_metrics_st {
    size_ty code;
    struct point_st scalable_size;
    struct point_st device_size;
    struct point_st size;
    struct point_st offset;
};

struct glyph_texture_atlas_st {
    const struct font_face_st* source_font_face;
    size_ty pixels_width;
    size_ty pixels_height;
    unsigned char* pixels;
    struct hash_tablei_st* glyph_entries;
};

struct glyph_texture_atlas_entry_st {
    const struct glyph_st* glyph;
    float uv_min_u;
    float uv_min_v;
    float uv_max_u;
    float uv_max_v;
};

struct font_family_st* font_family_load_from_file(const char* file_path);
void                   font_family_destroy(struct font_family_st** font_family);
const char*            font_family_get_name(const struct font_family_st* font_family);
struct font_face_st*   font_family_get_face(const struct font_family_st* font_family, enum font_style_en style);

struct font_face_metrics_st   font_face_get_metrics(const struct font_face_st* font_face);
const struct glyph_st*        font_face_get_glyph(const struct font_face_st* font_face, size_ty code);
struct glyph_texture_atlas_st font_face_generate_glyph_texture_atlas(const struct font_face_st* font_face);

struct glyph_metrics_st glyph_get_metrics(const struct glyph_st* glyph);

#endif