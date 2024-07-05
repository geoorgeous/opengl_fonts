#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "array.h"
#include "file_io.h"
#include "gfx_mesh.h"
#include "gltf.h"
#include "json.h"
#include "log.h"

enum gltf_primitive_mode_en {
    GLTF_PRIMITIVE_MODE_POINTS,
    GLTF_PRIMITIVE_MODE_LINES,
    GLTF_PRIMITIVE_MODE_LINE_LOOP,
    GLTF_PRIMITIVE_MODE_LINE_STRIP,
    GLTF_PRIMITIVE_MODE_TRIANGLES,
    GLTF_PRIMITIVE_MODE_TRIANGLE_STRIP,
    GLTF_PRIMITIVE_MODE_TRIANGLE_FAN
};

// todo: refactor
size_t gltf_get_accessor_type_component_count(const char* accessor_type) {
    if (strncmp(accessor_type, "VEC", 3) == 0) {
        return accessor_type[3] - '0';
    } else if (strncmp(accessor_type, "MAT", 3) == 0) {
        return (accessor_type[3] - '0') * (accessor_type[3] - '0');
    } else if (strncmp(accessor_type, "SCALAR", 6) == 0) {
        return 1;
    }
    return 0;
}

size_t gltf_sizeof_accessor_component_type(unsigned int component_type) {
    switch (component_type) {
        case 5120:           // BYTE
        case 5121: return 1; // UNSIGNED_BYTE
        case 5122:           // SHORT
        case 5123: return 2; // UNSIGNED_SHORT
        case 5125:           // UNSIGNED_INT
        case 5126: return 4; // FLOAT
        default: return 0;
    }
}

enum gfx_vertex_attribute_type_en gltf_component_type_to_vertex_attribute_type(unsigned int component_type) {
    switch (component_type) {   
        case 5120: return GFX_VERTEX_ATTRIBUTE_TYPE_INT8;
        case 5121: return GFX_VERTEX_ATTRIBUTE_TYPE_UINT8;
        case 5122: return GFX_VERTEX_ATTRIBUTE_TYPE_INT16;
        case 5123: return GFX_VERTEX_ATTRIBUTE_TYPE_UINT16;
        case 5125: return GFX_VERTEX_ATTRIBUTE_TYPE_UINT32;
        case 5126: return GFX_VERTEX_ATTRIBUTE_TYPE_FLOAT32;
        default: return 0;
    }
}

void gltf_vertex_attribute_from_gltf_accessor(struct json_value_st* accessor, struct gfx_vertex_attribute_st* attribute) {
    struct json_value_st* accessor_normalized = json_object_get(accessor, "normalized");
    if (accessor_normalized != NULL)
        attribute->b_is_normalised = json_bool(accessor_normalized);
    else
        attribute->b_is_normalised = b_FALSE;

    unsigned int accessor_component_type = (unsigned int)json_number(json_object_get(accessor, "componentType"));
    if (accessor_component_type == 5126)
        attribute->b_is_integer_storage = b_FALSE;
    else
        attribute->b_is_integer_storage = b_TRUE;

    struct json_value_st* accessor_type = json_object_get(accessor, "type");
    attribute->num_components = gltf_get_accessor_type_component_count(json_string(accessor_type));

    attribute->type = gltf_component_type_to_vertex_attribute_type(accessor_component_type);
}

uint8_t* gltf_get_accessor_buffer_ptr(struct json_value_st* accessor, struct json_value_st* buffer_views, uint8_t* buffer_data) {
    struct json_value_st* accessor_buffer_view_index = json_object_get(accessor, "bufferView");
    struct json_value_st* accessor_byte_offset = json_object_get(accessor, "byteOffset");
    struct json_value_st* accessor_component_type = json_object_get(accessor, "componentType");
    struct json_value_st* accessor_count = json_object_get(accessor, "count");
    struct json_value_st* accessor_type = json_object_get(accessor, "type");
    
    struct json_value_st* buffer_view = json_array_get(buffer_views, (size_ty)json_number(accessor_buffer_view_index));
    struct json_value_st* buffer_view_buffer = json_object_get(buffer_view, "buffer");
    struct json_value_st* buffer_view_byte_offset = json_object_get(buffer_view, "byteOffset");
    struct json_value_st* buffer_view_byte_length = json_object_get(buffer_view, "byteLength");

    size_t component_count = gltf_get_accessor_type_component_count(json_string(accessor_type));
    size_t component_size = gltf_sizeof_accessor_component_type((unsigned int)json_number(accessor_component_type));

    size_t buffer_offset = (buffer_view_byte_offset == NULL ? 0 : (size_t)json_number(buffer_view_byte_offset)) + (accessor_byte_offset == NULL ? 0 : (size_t)json_number(accessor_byte_offset));
    size_t data_size = component_size * component_count * (size_t)json_number(accessor_count);

    return buffer_data + buffer_offset;
}

void gltf_import(const char* file_path) {
    size_ty buffer_size;
    char* buffer_start = file_io_read(file_path, &buffer_size);

    if (buffer_start == NULL || buffer_size < 12) // Buffer must be 12 bytes to read header
        return;

    uint32_t* header = (uint32_t*)buffer_start;
    if (header[0] != 0x46546C67) {
        // todo: error
        return;
    }

    uint32_t* json_chunk = header + 3;
    uint32_t json_chunk_length = json_chunk[0];
    char* json_chunk_data = (char*)&json_chunk[2];

    uint32_t* binary_chunk = (uint32_t*)(json_chunk_data + (size_t)(json_chunk_length));
    uint32_t binary_chunk_length = binary_chunk[0];
    uint8_t* binary_chunk_data = (uint8_t*)&binary_chunk[2];

    struct json_value_st* json = json_parse(json_chunk_data, (size_ty)json_chunk_length);
    if (json == NULL) {
        // todo: error
        return;
    }
    
    struct json_value_st* asset = json_object_get(json, "asset");

    struct json_value_st* accessors = json_object_get(json, "accessors");
    struct json_value_st* buffer_views = json_object_get(json, "bufferViews");
    struct json_value_st* meshes = json_object_get(json, "meshes");

    for (size_ty i = 0; i < json_array_length(meshes); ++i) {
        struct json_value_st* mesh = json_array_get(meshes, i);
        struct json_value_st* primitives = json_object_get(mesh, "primitives");
        for (size_ty j = 0; j < json_array_length(primitives); ++j) {
            struct json_value_st* primitive = json_array_get(primitives, j);
            struct json_value_st* attributes = json_object_get(primitive, "attributes");
            
            struct json_value_st* position = json_object_get(attributes, "POSITION");
            struct json_value_st* accessor = json_array_get(accessors, (size_ty)json_number(position));

            // TODO: NORMAL
            // TODO: TANGENT
            // TODO: TEXCOORD_n
            // TODO: COLOR_n
            // TODO: JOINTS_n
            // TODO: WEIGHTS_n

            // TODO: primitive.indices
            // TODO: primitive.material
            // TODO: primitive.mode
            struct gfx_mesh_data_st mesh_data;
            mesh_data.attributes = array_create(sizeof(struct gfx_vertex_attribute_st));
            mesh_data.num_vertices = 0;
            mesh_data.vertices = NULL;
            mesh_data.num_indices = 0;
            mesh_data.indices = NULL;

            gltf_vertex_attribute_from_gltf_accessor(accessor, array_get(mesh_data.attributes, 0));
            mesh_data.vertices = gltf_get_accessor_buffer_ptr(accessor, buffer_views, binary_chunk_data);
            mesh_data.num_vertices = (size_ty)json_number(json_object_get(accessor, "count"));

            struct gfx_mesh_st* mesh = gfx_mesh_create();

            gfx_mesh_set_data(mesh, &mesh_data);
            gfx_mesh_destroy(&mesh);
        }
    }
}