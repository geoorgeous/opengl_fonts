#include <stdlib.h>

#include "array.h"
#include "gfx_mesh.h"
#include "open_gl.h"

struct gfx_mesh_st {
    GLuint vbo_handle;
    GLuint vio_handle;
    GLuint vao_handle;
    size_ty num_vertices;
    size_ty num_indices;
};

struct gfx_mesh_st* gfx_mesh_create() {
    struct gfx_mesh_st* mesh = malloc(sizeof(struct gfx_mesh_st));
    glGenVertexArrays(1, &mesh->vao_handle);
    mesh->vbo_handle = 0;
    mesh->vio_handle = 0;
    return mesh;
}

size_ty sizeof_vertex_attribute_type(enum gfx_vertex_attribute_type_en type);
GLenum vertex_attribute_type_to_open_gl_type(enum gfx_vertex_attribute_type_en type);

void gfx_mesh_destroy(struct gfx_mesh_st** mesh) {
    glDeleteBuffers(1, &(*mesh)->vbo_handle);
    glDeleteBuffers(1, &(*mesh)->vio_handle);
    glDeleteVertexArrays(1, &(*mesh)->vao_handle);
    free(*mesh);
    *mesh = NULL;
}

void gfx_mesh_set_data(struct gfx_mesh_st* mesh, const struct gfx_mesh_data_st* data) {
    size_ty vertex_size = 0;
    for (size_ty i = 0; i < array_length(data->attributes); ++i) {
        struct gfx_vertex_attribute_st* attribute = array_get(data->attributes, i);
        vertex_size += sizeof_vertex_attribute_type(attribute->type) * attribute->num_components;
    }
        
    glBindVertexArray(mesh->vao_handle);
    
    if (data->vertices == NULL) {
        glDeleteBuffers(1, &mesh->vbo_handle);
        mesh->num_vertices = 0;
    } else {
        if(mesh->vbo_handle == 0)
            glGenBuffers(1, &mesh->vbo_handle);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_handle);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(data->num_vertices * vertex_size), data->vertices, GL_STATIC_DRAW);
        mesh->num_vertices = data->num_vertices;
    }

    if (data->indices == NULL) {
        glDeleteBuffers(1, &mesh->vio_handle);
        mesh->num_indices = 0;
    } else {
        if (mesh->vio_handle == 0)
            glGenBuffers(1, &mesh->vio_handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vio_handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(sizeof(*data->indices) * data->num_indices), data->indices, GL_STATIC_DRAW);
        mesh->num_indices = data->num_indices;
    }

    size_ty attribute_offset = 0;
    for (size_ty i = 0; i < array_length(data->attributes); ++i) {
        struct gfx_vertex_attribute_st* attribute = array_get(data->attributes, i);
        GLuint index = i;
        GLint size = attribute->num_components;
        GLenum type = vertex_attribute_type_to_open_gl_type(attribute->type);
        GLsizei stride = vertex_size;
        void* offset = (void*)(size_t)attribute_offset;
        if (attribute->b_is_integer_storage)
            glVertexAttribIPointer(index, size, type, stride, offset);
        else
            glVertexAttribPointer(index, size, type, attribute->b_is_normalised ? GL_TRUE : GL_FALSE, vertex_size, offset);
        glEnableVertexAttribArray(index);
        attribute_offset += attribute->num_components * sizeof_vertex_attribute_type(attribute->type);
    }
}

void gfx_mesh_draw(const struct gfx_mesh_st* mesh) {
    glBindVertexArray(mesh->vao_handle);
    if (mesh->vio_handle == 0)
        glDrawArrays(GL_TRIANGLES, 0, mesh->num_vertices);
    else
        glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, 0);
}


size_ty sizeof_vertex_attribute_type(enum gfx_vertex_attribute_type_en type) {
    switch (type) {
        case GFX_VERTEX_ATTRIBUTE_TYPE_FLOAT32: return 4;
        default: return 0;
    }
}

GLenum vertex_attribute_type_to_open_gl_type(enum gfx_vertex_attribute_type_en type) {
    switch (type) {
        case GFX_VERTEX_ATTRIBUTE_TYPE_FLOAT32: return GL_FLOAT;
        default: return 0;
    }
}