#include <stdlib.h>

#include "gfx_texture.h"
#include "open_gl.h"

struct gfx_texture_st {
    GLuint handle;
};

struct gfx_texture_st* gfx_texture_create() {
    struct gfx_texture_st* texture = malloc(sizeof(struct gfx_texture_st));
    glGenTextures(1, &texture->handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

void gfx_texture_destroy(struct gfx_texture_st** texture) {
    glDeleteTextures(1, &(*texture)->handle);
    free(*texture);
    *texture = NULL;
}

void gfx_texture_bind_to_device(const struct gfx_texture_st* texture) {
    glBindTexture(GL_TEXTURE_2D, texture->handle);
}

void gfx_texture_set_data(struct gfx_texture_st* texture, size_ty width, size_ty height, void* data) {
    glBindTexture(GL_TEXTURE_2D, texture->handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)width, (GLsizei)height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}