#include <stdlib.h>

#include "gfx_shader.h"
#include "log.h"
#include "matrix.h"
#include "open_gl.h"

struct gfx_shader_st {
    GLuint handle;
    bool_ty b_is_compiled;
};

GLuint opengl_create_shader(GLenum stage, const char* source);

struct gfx_shader_st* gfx_shader_create() {
    struct gfx_shader_st* shader = malloc(sizeof(struct gfx_shader_st));
    shader->handle = glCreateProgram();
    if (shader->handle == 0) {
        free(shader);
        shader = NULL;
        log_error("OpenGL failed to create shader program.");
    }
    return shader;
}

void gfx_shader_destroy(struct gfx_shader_st** shader) {
    glDeleteShader((*shader)->handle);
    free(*shader);
    *shader = NULL;
}

bool_ty gfx_shader_compile(struct gfx_shader_st* shader, const char* vertex_stage_source, const char* fragment_stage_source) {
    GLuint vertex_shader_handle = opengl_create_shader(GL_VERTEX_SHADER, vertex_stage_source);
    GLuint fragment_shader_handle = opengl_create_shader(GL_FRAGMENT_SHADER, fragment_stage_source);
    if (vertex_shader_handle == 0 || fragment_shader_handle == 0)
        return b_FALSE;

    shader->b_is_compiled = b_FALSE;

    glAttachShader(shader->handle, vertex_shader_handle);
    glAttachShader(shader->handle, fragment_shader_handle);
    glLinkProgram(shader->handle);
    
    glDeleteShader(vertex_shader_handle);
    glDeleteShader(fragment_shader_handle);

    GLint link_status;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &link_status);
    if(link_status == GL_FALSE)
    {
        GLchar info_log[1024];
        glGetProgramInfoLog(shader->handle, 1024, NULL, info_log);
        log_error("OpenGL failed to link shader program: %s\n", info_log);
        return b_FALSE;
    }
    
    shader->b_is_compiled = b_TRUE;
    return b_TRUE;
}

bool_ty gfx_shader_is_compiled(const struct gfx_shader_st* shader) {
    return shader->b_is_compiled;
}

void gfx_shader_make_current(const struct gfx_shader_st* shader) {
    if(shader->b_is_compiled)
        glUseProgram(shader->handle);
}
void gfx_shader_set_uniform_int(const struct gfx_shader_st* shader, const char* uniform_id, int i) {
    GLint location = glGetUniformLocation(shader->handle, uniform_id);
    glUniform1i(location, i);
}

void gfx_shader_set_uniform_mat4(const struct gfx_shader_st* shader, const char* uniform_id, const struct mat4_st* matrix) {
    GLint location = glGetUniformLocation(shader->handle, uniform_id);
    glUniformMatrix4fv(location, 1, GL_FALSE, (GLfloat*)matrix->elements);
}

GLuint opengl_create_shader(GLenum stage, const char* source) {
    GLuint handle = glCreateShader(stage);
    if (handle == 0) {
        log_error("OpenGL failed to create shader. Stage: %i\n", stage);
        return 0;
    }

    glShaderSource(handle, 1, &source, NULL);
    glCompileShader(handle);

    GLint status;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLchar info_log[1024];
        glGetShaderInfoLog(handle, 1024, NULL, info_log);
        log_error("Failed to compile shader: %s\n", info_log);
        glDeleteShader(handle);
        return 0;
    };
    return handle;
}