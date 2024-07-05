#include <stdlib.h>

#include "array.h"
#include "common_types.h"
#include "gfx_mesh.h"
#include "gfx_renderer.h"
#include "gfx_shader.h"
#include "gfx_texture.h"
#include "open_gl.h"

struct gfx_renderer_st {
    struct gfx_shader_st* shader;
    struct array_st* commands;
};

struct gfx_renderer_st* gfx_renderer_create() {
    struct gfx_renderer_st* renderer = malloc(sizeof(struct gfx_renderer_st));
    renderer->shader = gfx_shader_create();
    gfx_shader_compile(renderer->shader,
        "#version 330 core\n"
        "uniform mat4 u_camera_matrix;\n"
        "uniform mat4 u_model_matrix;\n"
        "layout (location = 0) in vec3 a_position;\n"
        "layout (location = 1) in vec2 a_texcoord;\n"
        "layout (location = 2) in vec4 a_color;\n"
        "out vec2 v_texcoord;\n"
        "out vec4 v_color;\n"
        "void main() {\n"
        "   v_texcoord = a_texcoord;\n"
        "   v_color = a_color;\n"
        "   gl_Position = u_camera_matrix * u_model_matrix * vec4(a_position.xyz, 1.0);\n"
        "}",
        "#version 330 core\n"
        "uniform sampler2D u_texture;\n"
        "in vec2 v_texcoord;\n"
        "in vec4 v_color;\n"
        "out vec4 f_color;\n"
        "void main() {\n"
            "f_color = texture(u_texture, v_texcoord) * v_color;\n"
        "}");
    renderer->commands = array_create(sizeof(struct gfx_renderer_command_st));
    return renderer;
}

void gfx_renderer_destroy(struct gfx_renderer_st** renderer) {
    array_destroy(&(*renderer)->commands);
    free(*renderer);
    *renderer = NULL;
}

void gfx_renderer_push_command(struct gfx_renderer_st* renderer, const struct gfx_renderer_command_st* command) {
    struct gfx_renderer_command_st* new_command = array_push(renderer->commands);
    *new_command = *command;
}

void gfx_renderer_submit_commands(struct gfx_renderer_st* renderer, const struct mat4_st* camera_matrix) {
    glClear(GL_COLOR_BUFFER_BIT);

    gfx_shader_make_current(renderer->shader);
    gfx_shader_set_uniform_mat4(renderer->shader, "u_camera_matrix", camera_matrix);
    gfx_shader_set_uniform_int(renderer->shader, "u_texture", 0);
    glActiveTexture(GL_TEXTURE0);
    for (size_ty i = 0; i < array_length(renderer->commands); ++i) {
        struct gfx_renderer_command_st* command = array_get(renderer->commands, i);
        gfx_texture_bind_to_device(command->texture);
        gfx_shader_set_uniform_mat4(renderer->shader, "u_model_matrix", &command->transform);
        gfx_mesh_draw(command->mesh);
    }
    array_clear(renderer->commands);
}
