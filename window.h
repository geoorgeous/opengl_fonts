#ifndef WINDOW_H
#define WINDOW_H

#include "common_types.h"

enum key_en {
	KEY_UNDEFINED = -1,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_NUMPAD_0,
	KEY_NUMPAD_1,
	KEY_NUMPAD_2,
	KEY_NUMPAD_3,
	KEY_NUMPAD_4,
	KEY_NUMPAD_5,
	KEY_NUMPAD_6,
	KEY_NUMPAD_7,
	KEY_NUMPAD_8,
	KEY_NUMPAD_9,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_ESC,
	KEY_GRAVE_ACCENT,
	KEY_TAB,
	KEY_CAPS_LOCK,
	KEY_LEFT_SHIFT,
	KEY_RIGHT_SHIFT,
	KEY_LEFT_CONTROL,
	KEY_RIGHT_CONTROL,
	KEY_ALT,
	KEY_SPACE,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,
	KEY_HOME,
	KEY_END,
	KEY_INSERT,
	KEY_DELETE,
	KEY_ENTER,
	KEY_BACKSPACE,
	KEY_DASH,
	KEY_EQUALS,
	KEY_LEFT_BRACE,
	KEY_RIGHT_BRACE,
	KEY_SEMI_COLON,
	KEY_SINGLE_QUOTE,
	KEY_TILDE,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_BACKWARD_SLASH,
	KEY_FORWARD_SLASH,
	KEY__MAX
};

enum mouse_button_en {
	MOUSE_BUTTON_UNDEFINED = -1,
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON__MAX
};

enum button_action_en {
	BUTTON_ACTION_UNDEFINED = -1,
	BUTTON_ACTION_PRESS,
	BUTTON_ACTION_RELEASE
};

enum cursor_style_en {
	CURSOR_STYLE_NORMAL,
	CURSOR_STYLE_TEXT_SELECT,
	CURSOR_STYLE_BUSY,
	CURSOR_STYLE_BUSY_BACKGROUND,
	CURSOR_STYLE_CROSS,
	CURSOR_STYLE_SIZE_HORIZONTAL,
	CURSOR_STYLE_SIZE_VERTICAL,
	CURSOR_STYLE_SIZE_DIAG_NWSE,
	CURSOR_STYLE_SIZE_DIAG_NESW,
	CURSOR_STYLE_MOVE,
	CURSOR_STYLE_HAND
};

enum cursor_mode_en {
	CURSOR_MODE_NORMAL,
	CURSOR_MODE_HIDDEN,
	CURSOR_MODE_DISABLED
};

struct gfx_window_surface_st;

struct window_st;

struct window_st* window_create(struct point_st size, const char* title);
void                       window_destroy(struct window_st** window);
void                       window_handle_system_events(struct window_st* window);
bool_ty                    window_is_closed(const struct window_st* window);
struct point_st            window_get_size(const struct window_st* window);
bool_ty                    window_is_key_down(const struct window_st* window, enum key_en key);
bool_ty                    window_is_mouse_button_down(const struct window_st* window, enum mouse_button_en mouse_button);
struct point_st            window_get_mouse_position(const struct window_st* window);
void				       window_set_title(struct window_st* window, const char* title);
void                       window_set_cursor_style(struct window_st* window, enum cursor_style_en cursor_style);
void                       window_set_cursor_mode(struct window_st* window, enum cursor_mode_en cursor_mode);
void                       window_set_is_resizable(struct window_st* window, bool_ty is_resizable);

struct gfx_window_surface_st* gfx_window_surface_create(const struct window_st* window);
void                          gfx_window_surface_destroy(struct gfx_window_surface_st** window_surface);
void                          gfx_window_surface_get_resolution(const struct gfx_window_surface_st*);
void                          gfx_window_surface_set_resolution(struct gfx_window_surface_st* window_surface);
void                          gfx_window_surface_present(struct gfx_window_surface_st* window_surface);

#endif