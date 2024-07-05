// Seperation of ENGINE and TOOLS
// TOOLS should not be shipped with final executable.
// font file loading
// data driven file.georges_engine_data
// TOOLS for authoring georges_engine_data files:
// 3d models
// animations
// fonts
// scenes
// sounds
// textures

// todo:
// open window - use .ini file for settings?
// boot engine - use .ini?
// load resources
// load scene
// user input
// on-screen text
// console

// Todo: misc and icks
// dont like how mesh_data has an array but not constructor...
// dont like using low-level includes (e.g stdlib) in high-level source files
// hash table support string and int keys...
// C string char -> glyph code proper conversion??

// Todo: Text rendering
// 1. [x] Process font file: Read in a font file and store each separate glyph's metrics individually.
// 2. [x] Generate texture: Rasterize font glyphs using glyph metrics in to a font-face glyph texture-atlas.
// 3. [x] Generate mesh: Generate text mesh based on some string to display and a given font-face glyph texture-atlas.
// 4. [x] Render the mesh.
// 5. [ ] Advanced: Display formatted text, i.e. different colors, styles. Display at different sizes.
// 6. [ ] Better memory management
// 7. [ ] Better algorithm for texture growth

// Todo: UI rendering:
// 1. [ ] Draw transparent background
// 2. [ ] Overlay text on top of transparent background
// 3. [ ] Log history window
// 3. [ ] Stats window
// 3. [ ] CLI window

#include <stdlib.h>

#include "array.h"
#include "font.h"
#include "gfx_mesh.h"
#include "gfx_renderer.h"
#include "gfx_shader.h"
#include "gfx_text.h"
#include "gfx_texture.h"
#include "gltf.h"
#include "log.h"
#include "matrix.h"
#include "window.h"

bool_ty int_compare(const void* a, const void* b) {
    return (*(int*)a) < (*(int*)b) ? b_TRUE : b_FALSE;
}

int main(int argc, const char* argv[]) {
    struct window_st* window = window_create((struct point_st){ 960, 540 }, "George's Engine");
    struct gfx_window_surface_st* window_surface = gfx_window_surface_create(window);

    struct gfx_renderer_st* renderer = gfx_renderer_create();

    struct mat4_st camera_matrix = mat4_orthographic(0, 960, 0, 540, -1, 1);

    // Load font and generate texture atlas
    struct font_family_st* font_family = font_family_load_from_file("im9x14u.bdf");
    struct font_face_st* font_face = font_family_get_face(font_family, FONT_STYLE_REGULAR);
    struct glyph_texture_atlas_st glyph_texture_atlas = font_face_generate_glyph_texture_atlas(font_face);

    
    float color[4] = { 0.2f, 0.75f, 0.2f, 1.0f };

    // Generate text mesh
    struct gfx_mesh_data_st text_mesh_data = gfx_text_generate_mesh_data(&glyph_texture_atlas, 
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor\n"
        "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud\n"
        "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute\n"
        "irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla\n"
        "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia\n"
        "deserunt mollit anim id est laborum.", color);
    struct gfx_mesh_st* text_mesh = gfx_mesh_create();
    gfx_mesh_set_data(text_mesh, &text_mesh_data);
    array_destroy(&text_mesh_data.attributes);
    free(text_mesh_data.vertices);
    free(text_mesh_data.indices);

    // Create texture front texture atlas data
    struct gfx_texture_st* font_texture = gfx_texture_create();
    gfx_texture_set_data(font_texture, glyph_texture_atlas.pixels_width, glyph_texture_atlas.pixels_height, glyph_texture_atlas.pixels);
    free(glyph_texture_atlas.pixels);

    // gltf_import("Fox.glb");

    while(!window_is_closed(window)) {
        window_handle_system_events(window);

        struct gfx_renderer_command_st command;
        command.mesh = text_mesh;
        command.texture = font_texture;
        command.transform = mat4_transform_t(10, 20, 0);
        gfx_renderer_push_command(renderer, &command);

        gfx_renderer_submit_commands(renderer, &camera_matrix);

        gfx_window_surface_present(window_surface);
    };

    gfx_texture_destroy(&font_texture);
    gfx_mesh_destroy(&text_mesh);
    font_family_destroy(&font_family);
    gfx_renderer_destroy(&renderer);
    window_destroy(&window);

    return 0;
}