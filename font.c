#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "array.h"
#include "file_io.h"
#include "hash_table.h"
#include "font.h"
#include "log.h"













#include <stdio.h>

struct glyph_st {
    struct glyph_metrics_st metrics;
    unsigned char* bitmap;
};

struct font_family_st {
    char* name;
    struct array_st* faces;
};

struct font_face_st {
    char* name;
    enum font_style_en style;
    struct font_face_metrics_st metrics;
    struct hash_tablei_st* glyphs;
};

static bool_ty glyph_compare(const void* a, const void* b);

struct font_family_st* font_family_load_from_file(const char* file_path) {
    unsigned int buffer_size;
    char* buffer = file_io_read(file_path, &buffer_size);
    if (buffer == NULL)
        return NULL;
    
    int BDF_FILE_SECTION_GLOBAL = 0;
    int BDF_FILE_SECTION_PROPERTIES = 1;
    int BDF_FILE_SECTION_CHAR = 2;
    int BDF_FILE_SECTION_CHAR_BITMAT = 3;

    struct font_family_st* font_family = malloc(sizeof(struct font_family_st));
    font_family->name = NULL;
    font_family->faces = array_create(sizeof(struct font_face_st));
    
    // BDF file format only supports storing single font face.
    struct font_face_st* font_face = array_push(font_family->faces);
    font_face->name = NULL;
    font_face->style = FONT_STYLE_REGULAR;
    font_face->metrics = (struct font_face_metrics_st){ 0 };
    font_face->glyphs = hash_tablei_create(sizeof(struct glyph_st));

    struct glyph_st* glyph;

    char* line_start = buffer;
    char* line_pos = line_start;
    int section = 0;
    while(*line_pos != '\0') {
        if (*line_pos == '\n') {
            *line_pos = '\0';
            char* tok = strtok(line_start, " ");
            char hex[3] = {0};
            if (section == BDF_FILE_SECTION_CHAR_BITMAT) {
                // process hex
                // 60   01100000
                // F0   11110000
                // F0   11110000  
                // F0   11110000
                // 60   01100000
                // 60   01100000
                // 00   00000000
                // 60   01100000
                // 60   01100000

                size_ty bitmap_width_bytes = (size_ty)ceilf((float)glyph->metrics.size.x / 8);
                for (size_ty row = 0; row < (size_ty)glyph->metrics.size.y; ++row) {
                    for (size_ty bytei = 0; bytei < bitmap_width_bytes; ++bytei) {
                        hex[0] = *(tok + 0);
                        hex[1] = *(tok + 1);
                        unsigned char byte_value = strtol(hex, NULL, 16);
                        glyph->bitmap[bitmap_width_bytes * row + bytei] = byte_value;
                        tok += 2;
                        line_pos = tok;
                    }
                    while(*tok != '\0' && *tok != '\n')
                        tok++;
                    line_pos = tok++;
                }
                section = BDF_FILE_SECTION_CHAR;
            } else if (section == BDF_FILE_SECTION_CHAR) {
                if (strcmp(tok, "STARTCHAR") == 0) {
                    char* glyph_name = strtok(NULL, " ");
                    // log_debug("GLYPH %s\n", glyph_name);
                    glyph = hash_tablei_add(font_face->glyphs, glyph->metrics.code);
                    glyph->bitmap = NULL;
                } else if (strcmp(tok, "ENCODING") == 0) {
                    char* code = strtok(NULL, " ");
                    // log_debug("GLYPH | CODE: %s\n", code);
                    glyph->metrics.code = (size_ty)atoi(code);
                } else if (strcmp(tok, "SWIDTH") == 0) {
                    char* swidth = strtok(NULL, " ");
                    char* sheight = strtok(NULL, " ");
                    // log_debug("GLYPH | SCALABLE SIZE: %s x %s\n", swidth, sheight);
                    glyph->metrics.scalable_size.x = atoi(swidth);
                    glyph->metrics.scalable_size.y = atoi(sheight);
                } else if (strcmp(tok, "DWIDTH") == 0) {
                    char* dwidth = strtok(NULL, " ");
                    char* dheight = strtok(NULL, " ");
                    // log_debug("GLYPH | DEVICE SIZE: %s x %s\n", dwidth, dheight);
                    glyph->metrics.device_size.x = atoi(dwidth);
                    glyph->metrics.device_size.y = atoi(dheight);
                } else if (strcmp(tok, "BBX") == 0) {
                    char* bb_width = strtok(NULL, " ");
                    char* bb_height = strtok(NULL, " ");
                    char* bb_offset_x = strtok(NULL, " ");
                    char* bb_offset_y = strtok(NULL, " ");
                    // log_debug("GLYPH | BOUNDS: Size: %s x %s, Offset: %s x %s\n", bb_width, bb_height, bb_offset_x, bb_offset_y);
                    glyph->metrics.size.x = atoi(bb_width);
                    glyph->metrics.size.y = atoi(bb_height);
                    glyph->metrics.offset.x = atoi(bb_offset_x);
                    glyph->metrics.offset.y = atoi(bb_offset_y);
                } else if (strcmp(tok, "BITMAP") == 0) {
                    size_ty bitmap_width_bytes = (size_ty)ceilf((float)glyph->metrics.size.x / 8);
                    glyph->bitmap = malloc(sizeof(unsigned char) * bitmap_width_bytes * glyph->metrics.size.y);
                    section = BDF_FILE_SECTION_CHAR_BITMAT;
                } else if (strcmp(tok, "ENDFONT") == 0) {
                    break;
                }
            } else if (section == BDF_FILE_SECTION_PROPERTIES) {
                if (strcmp(tok, "FONT_ASCENT") == 0) {
                    char* ascent = strtok(NULL, " ");
                    log_debug("FONT_ASCENT: %s\n", ascent);

                } else if (strcmp(tok, "FONT_DESCENT") == 0) {
                    char* descent = strtok(NULL, " ");
                    log_debug("FONT_DESCENT: %s\n", descent);

                } else if (strcmp(tok, "ENDPROPERTIES") == 0) {
                    section = BDF_FILE_SECTION_GLOBAL;
                }
            } else if (section == BDF_FILE_SECTION_GLOBAL) {
                if (strcmp(tok, "STARTFONT") == 0) {
                    char* bdf_version = strtok(NULL, " ");
                    log_debug("BDF VERSION: %s\n", bdf_version);
                } else if (strcmp(tok, "FONT") == 0) {
                    char* font_name = strtok(NULL, " ");
                    log_debug("FONT: %s\n", font_name);
                    font_family->name = strdup(font_name);
                } else if (strcmp(tok, "SIZE") == 0) {
                    char* point_size = strtok(NULL, " ");
                    char* res_x = strtok(NULL, " ");
                    char* res_y = strtok(NULL, " ");
                    log_debug("SIZE: Point size: %s, Res: %s x %s\n", point_size, res_x, res_y);
                    font_face->metrics.point_size = atoi(point_size);
                    font_face->metrics.device_size.x = atoi(res_x);
                    font_face->metrics.device_size.y = atoi(res_y);
                } else if (strcmp(tok, "FONTBOUNDINGBOX") == 0) {
                    char* bb_width = strtok(NULL, " ");
                    char* bb_height = strtok(NULL, " ");
                    char* bb_offset_x = strtok(NULL, " ");
                    char* bb_offset_y = strtok(NULL, " ");
                    log_debug("FONTBOUNDINGBOX: Size: %s x %s, Offset: %s x %s\n", bb_width, bb_height, bb_offset_x, bb_offset_y);
                    font_face->metrics.glyph_size.x = atoi(bb_width);
                    font_face->metrics.glyph_size.y = atoi(bb_height);
                    font_face->metrics.glyph_offset.x = atoi(bb_offset_x);
                    font_face->metrics.glyph_offset.y = atoi(bb_offset_y);
                } else if (strcmp(tok, "STARTPROPERTIES") == 0) {
                    section = BDF_FILE_SECTION_PROPERTIES;
                } else if (strcmp(tok, "CHARS") == 0) {
                    char* num_glyphs = strtok(NULL, " ");
                    log_debug("CHARS: %s\n", num_glyphs);
                    section = BDF_FILE_SECTION_CHAR;
                }
            }
            line_start = line_pos + 1;
        }
        line_pos++;
    }

    return font_family;
}

static void free_glyph(size_ty key, void* value, void* user_ptr) {
    struct glyph_st* glyph = value;
    free(glyph->bitmap);
}

void font_family_destroy(struct font_family_st** font_family) {
    free((*font_family)->name);
    for (size_ty i = 0; i < array_length((*font_family)->faces); ++i) {
        struct font_face_st* face = array_get((*font_family)->faces, i);
        free(face->name);
        hash_tablei_iterate(face->glyphs, free_glyph, NULL);
        hash_tablei_destroy(&face->glyphs);
    }
    array_destroy(&(*font_family)->faces);
    free(*font_family);
    *font_family = NULL;
}

const char* font_family_get_name(const struct font_family_st* font_family) {
    return font_family->name;
}

struct font_face_st* font_family_get_face(const struct font_family_st* font_family, enum font_style_en style) {
    for (size_ty i = 0; i < array_length(font_family->faces); ++i) {
        struct font_face_st* face = array_get(font_family->faces, i);
        if (face->style == style)
            return face;
    }
    return NULL;
}

struct font_face_metrics_st font_face_get_metrics(const struct font_face_st* font_face) {
    return font_face->metrics;
}

const struct glyph_st* font_face_get_glyph(const struct font_face_st* font_face, size_ty code) {
    return hash_tablei_get(font_face->glyphs, code);
}

struct glyph_temp_st {
    const struct glyph_st* glyph;
    struct point_st atlas_position;
};

struct tree_node_st {
    struct tree_node_st* children[2];
    struct point_st min;
    struct point_st max;
    const struct glyph_temp_st* glyph;
};

struct tree_node_st* find_node_for_glyph(struct tree_node_st* node, const struct glyph_temp_st* glyph_temp) {
    if (node->children[0] != NULL) {
        struct tree_node_st* glyph_node = find_node_for_glyph(node->children[0], glyph_temp);
        if (glyph_node != NULL) return glyph_node;
        return find_node_for_glyph(node->children[1], glyph_temp);
    }

    if (node->glyph != NULL)
        return NULL;

    int node_width = (node->max.x - node->min.x) + 1;
    int node_height = (node->max.y - node->min.y) + 1;
    if (node_width < glyph_temp->glyph->metrics.size.x || node_height < glyph_temp->glyph->metrics.size.y)
        return NULL;

    if (node_width == glyph_temp->glyph->metrics.size.x && node_height == glyph_temp->glyph->metrics.size.y)
        return node;

    node->children[0] = malloc(sizeof(struct tree_node_st));
    node->children[0]->children[0] = NULL;
    node->children[0]->children[1] = NULL;
    node->children[0]->glyph = NULL;
    node->children[1] = malloc(sizeof(struct tree_node_st));
    node->children[1]->children[0] = NULL;
    node->children[1]->children[1] = NULL;
    node->children[1]->glyph = NULL;

    int width_difference = node_width - glyph_temp->glyph->metrics.size.x;
    int height_difference = node_height - glyph_temp->glyph->metrics.size.y;
    if (width_difference > height_difference) {
        node->children[0]->min = node->min;
        node->children[0]->max.x = node->min.x + glyph_temp->glyph->metrics.size.x - 1;
        node->children[0]->max.y = node->max.y;
        node->children[1]->min.x = node->min.x + glyph_temp->glyph->metrics.size.x;
        node->children[1]->min.y = node->min.y;
        node->children[1]->max = node->max;
    } else {
        node->children[0]->min = node->min;
        node->children[0]->max.x = node->max.x;
        node->children[0]->max.y = node->min.y + glyph_temp->glyph->metrics.size.y - 1;
        node->children[1]->min.x = node->min.x;
        node->children[1]->min.y = node->min.y + glyph_temp->glyph->metrics.size.y;
        node->children[1]->max = node->max;
    }

    return find_node_for_glyph(node->children[0], glyph_temp);
}

void add_glyph_to_list(size_ty key, void* glyph, void* user_ptr) {
    struct array_st* glyphs = user_ptr;
    struct glyph_temp_st* glyph_temp = array_push(glyphs);
    glyph_temp->glyph = glyph;
    glyph_temp->atlas_position = (struct point_st){ 0 };
}

static bool_ty glyph_compare(const void* a, const void* b) {
    const struct glyph_temp_st* glyph_a = a;
    const struct glyph_temp_st* glyph_b = b;
    if ((glyph_a->glyph->metrics.size.x * glyph_a->glyph->metrics.size.y) > (glyph_b->glyph->metrics.size.x * glyph_b->glyph->metrics.size.y))
        return b_TRUE;
    return b_FALSE;
}

struct glyph_texture_atlas_st font_face_generate_glyph_texture_atlas(const struct font_face_st* font_face) {
    struct array_st* glyph_temps = array_create(sizeof(struct glyph_temp_st));
    array_set_capacity(glyph_temps, hash_tablei_length(font_face->glyphs));
    hash_tablei_iterate(font_face->glyphs, add_glyph_to_list, glyph_temps);
    array_sort(glyph_temps, glyph_compare);

    struct glyph_texture_atlas_st glyph_texture_atlas;
    glyph_texture_atlas.source_font_face = font_face;
    glyph_texture_atlas.pixels_width = 512;
    glyph_texture_atlas.pixels_height = 512;
    glyph_texture_atlas.glyph_entries = hash_tablei_create(sizeof(struct glyph_texture_atlas_entry_st));

    struct tree_node_st root;
    root.children[0] = NULL;
    root.children[1] = NULL;
    root.glyph = NULL;
    root.min.x = 0;
    root.min.y = 0;
    root.max.x = glyph_texture_atlas.pixels_width - 1;
    root.max.y = glyph_texture_atlas.pixels_height - 1;
    
    for (size_ty i = 0; i < array_length(glyph_temps); ++i) {
        struct glyph_temp_st* glyph_temp = array_get(glyph_temps, i);
        struct tree_node_st* node = find_node_for_glyph(&root, glyph_temp);
        if (node == NULL) {
            log_error("Error packing glyphs in to texture\n");
        }
        node->glyph = glyph_temp;
        glyph_temp->atlas_position = node->min;
    }

    // todo: free tree nodes

    glyph_texture_atlas.pixels = calloc(glyph_texture_atlas.pixels_width * glyph_texture_atlas.pixels_height, sizeof(unsigned char));

    for (size_ty i = 0; i < array_length(glyph_temps); ++i) {
        struct glyph_temp_st* glyph_temp = array_get(glyph_temps, i);
        const struct glyph_st* glyph = glyph_temp->glyph;
        size_ty bitmap_width_bytes = (size_ty)ceilf((float)glyph->metrics.size.x / 8);

        for (size_ty row = 0; row < (size_ty)glyph->metrics.size.y; ++row) {
            size_ty texture_row = glyph_texture_atlas.pixels_height - glyph_temp->atlas_position.y - (row + 1); // OpenGL expects pixel data rows to be flipped vertically
            size_ty glyph_pixel_row_offset = texture_row * glyph_texture_atlas.pixels_width + glyph_temp->atlas_position.x;
            unsigned char* pixel = glyph_texture_atlas.pixels + glyph_pixel_row_offset;
            for (size_ty bytei = 0; bytei < bitmap_width_bytes; bytei++) {
                unsigned char byte = glyph->bitmap[bitmap_width_bytes * row + bytei];
                for (size_ty i = 0; i < 8; i++) {
                    if (bytei * 8 + i >= (size_ty)glyph->metrics.size.x)
                        break;
                    unsigned char pixel_value = (byte & (0x80 >> i)) ? 0xFF : 0;
                    *pixel++ = pixel_value;
                }
            }
        }


        struct glyph_texture_atlas_entry_st* entry = hash_tablei_add(glyph_texture_atlas.glyph_entries, glyph->metrics.code);
        entry->glyph = glyph;
        entry->uv_min_u = (float)glyph_temp->atlas_position.x / glyph_texture_atlas.pixels_width;
        entry->uv_max_u = entry->uv_min_u + (float)glyph->metrics.size.x / glyph_texture_atlas.pixels_width;
        entry->uv_max_v = 1.0f - (float)glyph_temp->atlas_position.y / glyph_texture_atlas.pixels_height;
        entry->uv_min_v = entry->uv_max_v - (float)glyph->metrics.size.y / glyph_texture_atlas.pixels_height;
    }

    return glyph_texture_atlas;
}

struct glyph_metrics_st glyph_get_metrics(const struct glyph_st* glyph) {
    return glyph->metrics;
}