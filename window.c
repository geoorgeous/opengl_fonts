#define WIN32_LEAN_AND_MEAN
#include <dwmapi.h>
#include <shellapi.h>
#include <windows.h>

#include "open_gl.h"
#include "log.h"
#include "window.h"

#define TARGET_OPENGL_VERSION_MAJOR 3
#define TARGET_OPENGL_VERSION_MINOR 3

struct gfx_window_surface_st {
    const struct window_st* window;
};

struct key_state_st {
	bool_ty is_down;
};

struct mouse_button_state_st {
	bool_ty is_down;
};

struct mouse_state_st {
	struct point_st client_position;
	struct mouse_button_state_st buttons[MOUSE_BUTTON__MAX];
};

struct window_input_state_st {
	struct key_state_st keys[KEY__MAX];
	struct mouse_state_st mouse;
};

struct window_st {
	HWND hwnd;
    HDC hdc;
	HGLRC hglrc;
	bool_ty is_closed;
	struct point_st size;
	struct window_input_state_st input_state;
	enum cursor_style_en cursor_style;
	enum cursor_mode_en cursor_mode;
};

typedef HGLRC WINAPI wgl_proc_wglCreateContextAttribsARB(HDC, HGLRC, const int*);
wgl_proc_wglCreateContextAttribsARB* wgl_create_context_attribs_arb;

typedef BOOL WINAPI wgl_proc_wglChoosePixelFormatARB(HDC, const int*, const FLOAT*, UINT, int*, UINT*);
wgl_proc_wglChoosePixelFormatARB* wgl_choose_pixel_format_arb;

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

#define WINDOW_CLASS_NAME TEXT("WGLWINDOW")

void print_wnd_error(const char* message) {
	DWORD error_code = GetLastError();
	if (error_code == 0)
		return;
	TCHAR error_message_buffer[512];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), error_message_buffer, 512, NULL);
	log_error("Win32 error: %s: %s\n", message, error_message_buffer);
}

bool_ty init_wgl_extensions() {
	static bool_ty has_completed = b_FALSE;

	if (has_completed)
		return b_TRUE;

	WNDCLASSA temp_wnd_class = {
		.style = CS_OWNDC,
		.lpfnWndProc = DefWindowProcA,
		.hInstance = GetModuleHandle(NULL),
		.lpszClassName = TEXT("WGLTEMP"),
	};

	if (!RegisterClass(&temp_wnd_class)) {
		print_wnd_error("Failed to register temporary OpenGL window class");
        return b_FALSE;
    }

	HWND hwnd = CreateWindowEx(
		0,
		temp_wnd_class.lpszClassName,
		temp_wnd_class.lpszClassName,
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		temp_wnd_class.hInstance,
		0);

	if (!hwnd) {
		print_wnd_error("Failed to create temporary OpenGL window");
        return b_FALSE;
    }

	HDC hdc = GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd = {
		.nSize = sizeof(pfd),
		.nVersion = 1,
		.iPixelType = PFD_TYPE_RGBA,
		.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		.cColorBits = 32,
		.cAlphaBits = 8,
		.iLayerType = PFD_MAIN_PLANE,
		.cDepthBits = 24,
		.cStencilBits = 8,
	};

	int pixel_format = ChoosePixelFormat(hdc, &pfd);
	if (!pixel_format) {
		print_wnd_error("Failed to find a suitable pixel format for temporary OpenGL context creation");
	    ReleaseDC(hwnd, hdc);
	    DestroyWindow(hwnd);
        return b_FALSE;
    }
    
	if (!SetPixelFormat(hdc, pixel_format, &pfd)) {
		print_wnd_error("Failed to set the pixel format for temporary OpenGL context creation");
	    DestroyWindow(hwnd);
        return b_FALSE;
    }

	HGLRC hglrc = wglCreateContext(hdc);
	if (!hglrc) {
		print_wnd_error("Failed to create temporary OpenGL rendering context");
	    ReleaseDC(hwnd, hdc);
	    DestroyWindow(hwnd);
        return b_FALSE;
    }

    HGLRC current_hglrc = wglGetCurrentContext();

	if (!wglMakeCurrent(hdc, hglrc)) {
		print_wnd_error("Failed to make temporary OpenGL rendering context current");
	    wglDeleteContext(hglrc);
	    ReleaseDC(hwnd, hdc);
	    DestroyWindow(hwnd);
        return b_FALSE;
    }

	wgl_create_context_attribs_arb = (wgl_proc_wglCreateContextAttribsARB*)wglGetProcAddress("wglCreateContextAttribsARB");
	wgl_choose_pixel_format_arb = (wgl_proc_wglChoosePixelFormatARB*)wglGetProcAddress("wglChoosePixelFormatARB");

	wglMakeCurrent(hdc, current_hglrc);
	wglDeleteContext(hglrc);
	ReleaseDC(hwnd, hdc);
	DestroyWindow(hwnd);

	has_completed = b_TRUE;
    return b_TRUE;
}

void on_gl_debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param) {
	const char* gl_message_type_strings[] = {
		"Deprecated Behaviour",
		"ERROR",
		"Marker",
		"Other",
		"Performance",
		"Pop Group",
		"Portability",
		"Push Group",
		"Undefined Behaviour"
	};
	const char* gl_message_severity_strings[] = {
		"High",
		"Low",
		"Medium",
		"Notification"
	};

	const char* type_string;
	switch(type) {
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_string = gl_message_type_strings[0]; break;
		case GL_DEBUG_TYPE_ERROR:               type_string = gl_message_type_strings[1]; break;
		case GL_DEBUG_TYPE_MARKER:              type_string = gl_message_type_strings[2]; break;
		default:
		case GL_DEBUG_TYPE_OTHER:               type_string = gl_message_type_strings[3]; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         type_string = gl_message_type_strings[4]; break;
		case GL_DEBUG_TYPE_POP_GROUP:           type_string = gl_message_type_strings[5]; break;
		case GL_DEBUG_TYPE_PORTABILITY:         type_string = gl_message_type_strings[6]; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          type_string = gl_message_type_strings[7]; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  type_string = gl_message_type_strings[8]; break;
	}
	
	const char* severity_string;
	switch(severity) {
		case GL_DEBUG_SEVERITY_HIGH:         severity_string = gl_message_severity_strings[0]; break;
		case GL_DEBUG_SEVERITY_LOW:          severity_string = gl_message_severity_strings[1]; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       severity_string = gl_message_severity_strings[2]; break;
		default:
		case GL_DEBUG_SEVERITY_NOTIFICATION: severity_string = gl_message_severity_strings[3]; break;
	}

	if (type == GL_DEBUG_TYPE_ERROR)
		log_error("OpenGL Error [%i]: (%s), Severity: %s. %s\n", id, type_string, severity_string, message);
}

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_gl_ARB                        0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x0001

HGLRC create_graphics_context(HDC hdc, int glv_major, int glv_minor) {
	if (!init_wgl_extensions())
        return NULL;

	int pixel_format_attributes[] = {
		WGL_DRAW_TO_WINDOW_ARB,  GL_TRUE,
		WGL_SUPPORT_gl_ARB,      GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,   GL_TRUE,
		WGL_ACCELERATION_ARB,    WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,      WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,      32,
		WGL_DEPTH_BITS_ARB,      24,
		WGL_STENCIL_BITS_ARB,    8,
		0
	};

	int pixel_format;
	UINT num_pixel_formats;
	wgl_choose_pixel_format_arb(hdc, pixel_format_attributes, 0, 1, &pixel_format, &num_pixel_formats);
	if (num_pixel_formats == 0) {
		print_wnd_error("Failed to set the OpenGL pixel format");
        return NULL;
    }

	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(hdc, pixel_format, sizeof(pfd), &pfd);
	if (!SetPixelFormat(hdc, pixel_format, &pfd)) {
		print_wnd_error("Failed to set the OpenGL pixel format");
        return NULL;
    }

	int gl_attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, glv_major,
		WGL_CONTEXT_MINOR_VERSION_ARB, glv_minor,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0,
	};

	HGLRC hglrc = wgl_create_context_attribs_arb(hdc, 0, gl_attributes);
	if (!hglrc) {
		print_wnd_error("Failed to create OpenGL context");
        return NULL;
    }

	if (!wglMakeCurrent(hdc, hglrc)) {
		print_wnd_error("Failed to make OpenGL context current");
        wglDeleteContext(hglrc);
        return NULL;
    }

	if (!gladLoaderLoadGL()) {
		log_error("Failed to load OpenGL function pointers.\n");
        return NULL;
    }

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(on_gl_debug_message, NULL);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	return hglrc;
}

void window_input_state_init(struct window_input_state_st* input_state) {
    for (int i = 0; i < KEY__MAX; ++i)
		input_state->keys[i].is_down = b_FALSE;
	input_state->mouse.client_position.x = 0;
	input_state->mouse.client_position.y = 0;
	for (int i = 0; i < MOUSE_BUTTON__MAX; ++i)
		input_state->mouse.buttons[i].is_down = b_FALSE;
}

enum key_en virtual_key_code_to_key(WORD virtual_key_code) {
	if (virtual_key_code >= 0x30 && virtual_key_code <= 0x39)
		return KEY_0 + (virtual_key_code - 0x30);
	if (virtual_key_code >= 0x41 && virtual_key_code <= 0x5A)
		return KEY_A + (virtual_key_code - 0x41);
	switch (virtual_key_code) {
		case VK_SHIFT:
		case VK_LSHIFT:      return KEY_LEFT_SHIFT;
		case VK_RSHIFT:      return KEY_RIGHT_SHIFT;
		case VK_CONTROL:
		case VK_LCONTROL:    return KEY_LEFT_CONTROL;
		case VK_RCONTROL:    return KEY_RIGHT_CONTROL;
		case VK_SPACE:       return KEY_SPACE;
		case VK_LEFT:        return KEY_LEFT;
		case VK_RIGHT:       return KEY_RIGHT;
		case VK_UP:          return KEY_UP;
		case VK_DOWN:        return KEY_DOWN;
	}
	return KEY_UNDEFINED;
}

void handle_mouse_button_message(struct window_st* window, enum mouse_button_en mouse_button, bool_ty is_down) {
	window->input_state.mouse.buttons[mouse_button].is_down = is_down;
	if (is_down) {
		// mouse button press
	} else {
		// mouse button release
	}
}

bool_ty window_post_create(struct window_st* window, HWND hwnd) {
	HDC hdc = GetDC(hwnd);
	HGLRC hglrc = create_graphics_context(hdc, TARGET_OPENGL_VERSION_MAJOR, TARGET_OPENGL_VERSION_MINOR);
	if (!hglrc) {
		ReleaseDC(hwnd, hdc);
		return b_FALSE;
	}
	glViewport(0, 0, (GLsizei)window->size.x, (GLsizei)window->size.y);

	window->hwnd = hwnd;
	window->hdc = hdc;
	window->hglrc = hglrc;

	window->is_closed = b_FALSE;
	window_input_state_init(&window->input_state);
	window->cursor_style = CURSOR_STYLE_NORMAL;
	window->cursor_mode = CURSOR_MODE_NORMAL;

	return b_TRUE;
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
	struct window_st* window = (struct window_st*)GetWindowLongPtr(hwnd, 0);

	switch (msg) {
        case WM_CREATE: {
			CREATESTRUCT create_struct = *(CREATESTRUCT*)(LPVOID)l_param;
			window = (struct window_st*)create_struct.lpCreateParams;
			if (!window_post_create(window, hwnd))
				return -1;
			SetWindowLongPtr(hwnd, 0, (LONG_PTR)window);
            return 0;
        }

		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP: {
			WORD virtual_key_code = LOWORD(w_param);
			WORD key_flags = HIWORD(l_param);
			WORD scan_code = LOBYTE(key_flags);
			if ((key_flags & KF_EXTENDED) == KF_EXTENDED) // extended-key flag, 1 if scancode has 0xE0 prefix
				scan_code = MAKEWORD(scan_code, 0xE0);
			switch (virtual_key_code) {
				case VK_SHIFT:   // converts to VK_LSHIFT or VK_RSHIFT
				case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
				case VK_MENU: {  // converts to VK_LMENU or VK_RMENU
					virtual_key_code = LOWORD(MapVirtualKey((UINT)scan_code, MAPVK_VSC_TO_VK_EX));
					break;
				}
			}

			BOOL was_key_down = (key_flags & KF_REPEAT) == KF_REPEAT;
			BOOL is_key_down = !((key_flags & KF_UP) == KF_UP);

			enum key_en key = virtual_key_code_to_key(virtual_key_code);
			if (key != KEY_UNDEFINED)
				window->input_state.keys[key].is_down = is_key_down;
			
			WORD repeat_count = LOWORD(l_param);
			for (WORD i = 0; i < repeat_count; ++i) {
				if (!was_key_down && is_key_down) {
					// key press
				} else if (was_key_down && !is_key_down) {
					// key release
				}
			}
			
			return 0;
		}

		case WM_CHAR: {
			log_info("WM_CHAR: character code: %c\n", (char)w_param);
			return 0;
		}

		case WM_MOUSEMOVE: {
            const WORD mousex = LOWORD(l_param);
            const WORD mousey = HIWORD(l_param);

			if (window->cursor_mode == CURSOR_MODE_DISABLED) {
				RECT rect;
				GetClientRect(window->hwnd, &rect);
				POINT mouse_screen_position;
				mouse_screen_position.x = window->input_state.mouse.client_position.x;
				mouse_screen_position.y = window->input_state.mouse.client_position.y;
				MapWindowPoints(window->hwnd, NULL, &mouse_screen_position, 1);
				SetCursorPos(mouse_screen_position.x, mouse_screen_position.y);
			} else {
				window->input_state.mouse.client_position.x = mousex;
				window->input_state.mouse.client_position.y = mousey;
			}			
			return 0;
		}

		case WM_LBUTTONDOWN: {
			handle_mouse_button_message(window, MOUSE_BUTTON_LEFT, b_TRUE);
			return 0;
		}

		case WM_LBUTTONUP: {
			handle_mouse_button_message(window, MOUSE_BUTTON_LEFT, b_FALSE);
			return 0;
		}

		case WM_MBUTTONDOWN: {
			handle_mouse_button_message(window, MOUSE_BUTTON_MIDDLE, b_TRUE);
			return 0;
		}

		case WM_MBUTTONUP: {
			handle_mouse_button_message(window, MOUSE_BUTTON_MIDDLE, b_FALSE);
			return 0;
		}

		case WM_RBUTTONDOWN: {
			handle_mouse_button_message(window, MOUSE_BUTTON_RIGHT, b_TRUE);
			return 0;
		}

		case WM_RBUTTONUP: {
			handle_mouse_button_message(window, MOUSE_BUTTON_RIGHT, b_FALSE);
			return 0;
		}

		case WM_DROPFILES: {
			HDROP hdrop = (HDROP)w_param;
			POINT drop_position;
			DragQueryPoint(hdrop, &drop_position);
			UINT num_files = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
			for (UINT i = 0; i < num_files; ++i) {
				UINT filename_length = DragQueryFile(hdrop, i, NULL, 0) + 1;
				LPTSTR filename = malloc(filename_length * sizeof(TCHAR));
				if (DragQueryFile(hdrop, i, filename, filename_length)){
					// todo: send file drop event: position of drop and list of filenames
				}
				free(filename);
			}
			DragFinish(hdrop);
			return 0;
		}

		case WM_SIZING:
		case WM_SIZE: {
			window->size.x = LOWORD(l_param);
			window->size.y = HIWORD(l_param);
			return 0;
		}

		case WM_CLOSE: {
            ReleaseDC(hwnd, window->hdc);
			DestroyWindow(hwnd);
			return 0;
		}

		case WM_DESTROY: {
			window->is_closed = b_TRUE;
            return 0;
		}

		// case WM_SETCURSOR: {
		// 	return 0;
		// }

		case WM_THEMECHANGED: {
			log_info("THEME CHANGE\n");
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, w_param, l_param);
}

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

struct window_st* window_create(struct point_st size, const char* title) {
	WNDCLASSA main_wnd_class;
	if (!GetClassInfo(GetModuleHandle(NULL), WINDOW_CLASS_NAME, &main_wnd_class)) {
		main_wnd_class = (WNDCLASSA) {
			.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
			.lpfnWndProc = wnd_proc,
			.hInstance = GetModuleHandle(NULL),
			.hCursor = LoadCursor(NULL, IDC_ARROW),
			.hbrBackground = NULL,
			.lpszClassName = WINDOW_CLASS_NAME,
			.cbWndExtra = sizeof(struct extra_window_data_st*)
		};

		if (!RegisterClass(&main_wnd_class)) {
			print_wnd_error("Failed to register main window class");
			return b_FALSE;
		}
	}

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = size.x;
	rect.bottom = size.y;
	DWORD default_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
	AdjustWindowRect(&rect, default_style, FALSE);

	struct window_st* window = malloc(sizeof(struct window_st));
	*window = (struct window_st) { 
		.hwnd = INVALID_HANDLE_VALUE,
		.hdc = INVALID_HANDLE_VALUE,
		.hglrc = INVALID_HANDLE_VALUE,
		.is_closed = b_TRUE,
		.size = size
	};

	HWND hwnd = CreateWindowExA(
		WS_EX_ACCEPTFILES,
		WINDOW_CLASS_NAME,
		title,
		default_style,
		CW_USEDEFAULT, /* Let Windows pick window position */
		SW_SHOWNORMAL, /* Because we set x param to CW_USEDEFAULT, y param is used to specify the value give to ShowWindow after creation */
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		main_wnd_class.hInstance,
		(LPVOID)window); /* Send our window handle to WM_CREATE message */

	if (!hwnd) {
		print_wnd_error("Failed to create window");
        return NULL;
	}

    // Must link with dwmapi.lib
	//BOOL value = TRUE;
	//DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, (LPCVOID)&value, sizeof(value));

	return window;
}

void window_destroy(struct window_st** window) {
	free(*window);
    *window = NULL;
}

void window_handle_system_events(struct window_st* window) {
	MSG msg;
	while (PeekMessage(&msg, window->hwnd, 0, 0, PM_REMOVE)) {
		if (msg.message != WM_QUIT) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

bool_ty window_is_closed(const struct window_st* window) {
	return window->is_closed;
}

struct point_st window_get_size(const struct window_st* window) {
	return window->size;
}

bool_ty window_is_key_down(const struct window_st* window, enum key_en key) {
	return window->input_state.keys[key].is_down;
}

bool_ty window_is_mouse_button_down(const struct window_st* window, enum mouse_button_en mouse_button) {
	return window->input_state.mouse.buttons[mouse_button].is_down;
}

struct point_st window_get_mouse_position(const struct window_st* window) {
    return window->input_state.mouse.client_position;
}

void window_set_title(struct window_st* window, const char* title) {
	SetWindowTextA(window->hwnd, title);
}

void window_set_cursor_style(struct window_st* window, enum cursor_style_en cursor_style) {
	window->cursor_style = cursor_style;
	if (window->cursor_mode != CURSOR_MODE_NORMAL)
		return;
	switch(cursor_style) {
		default:
		case CURSOR_STYLE_NORMAL:          SetCursor(LoadCursor(NULL, IDC_ARROW)); break;
		case CURSOR_STYLE_TEXT_SELECT:     SetCursor(LoadCursor(NULL, IDC_IBEAM)); break;
		case CURSOR_STYLE_BUSY:            SetCursor(LoadCursor(NULL, IDC_WAIT)); break;
		case CURSOR_STYLE_BUSY_BACKGROUND: SetCursor(LoadCursor(NULL, IDC_APPSTARTING)); break;
		case CURSOR_STYLE_CROSS:           SetCursor(LoadCursor(NULL, IDC_CROSS)); break;
		case CURSOR_STYLE_SIZE_HORIZONTAL: SetCursor(LoadCursor(NULL, IDC_SIZEWE)); break;
		case CURSOR_STYLE_SIZE_VERTICAL:   SetCursor(LoadCursor(NULL, IDC_SIZENS)); break;
		case CURSOR_STYLE_SIZE_DIAG_NWSE:  SetCursor(LoadCursor(NULL, IDC_SIZENWSE)); break;
		case CURSOR_STYLE_SIZE_DIAG_NESW:  SetCursor(LoadCursor(NULL, IDC_SIZENESW)); break;
		case CURSOR_STYLE_MOVE:            SetCursor(LoadCursor(NULL, IDC_SIZEALL)); break;
		case CURSOR_STYLE_HAND:            SetCursor(LoadCursor(NULL, IDC_HAND)); break;
	}
}

void window_set_cursor_mode(struct window_st* window, enum cursor_mode_en cursor_mode) {
	window->cursor_mode = cursor_mode;
	switch(cursor_mode) {
		default:
		case CURSOR_MODE_NORMAL: {
			ClipCursor(NULL);
			window_set_cursor_style(window, window->cursor_style);
			break;
		}

		case CURSOR_MODE_HIDDEN: {
			SetCursor(NULL);
			break;
		}

		case CURSOR_MODE_DISABLED: {
			RECT rect;
			GetClientRect(window->hwnd, &rect);

			POINT mouseScreenPosition;
			mouseScreenPosition.x = window->input_state.mouse.client_position.x;
			mouseScreenPosition.y = window->input_state.mouse.client_position.y;
			MapWindowPoints(window->hwnd, NULL, &mouseScreenPosition, 2);

			rect.left = mouseScreenPosition.x;
			rect.top = mouseScreenPosition.y;
			rect.right = rect.left + 1;
			rect.bottom = rect.top + 1;

			ClipCursor(&rect);

			SetCursor(NULL);
			break;
		}
	}
}

void window_set_is_resizable(struct window_st* window, bool_ty is_resizable) {
	LONG style = GetWindowLong(window->hwnd, GWL_STYLE);
	LONG resize_style = WS_THICKFRAME | WS_MAXIMIZEBOX;
	style = is_resizable ? style | resize_style : style & ~resize_style;
	SetWindowLong(window->hwnd, GWL_STYLE, style);
	SetWindowPos(window->hwnd, 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

struct gfx_window_surface_st* gfx_window_surface_create(const struct window_st* window) {
	struct gfx_window_surface_st* window_surface = malloc(sizeof(struct gfx_window_surface_st));
	window_surface->window = window;
	// window->screen_shader = graphics_shader_create();
    // graphics_shader_compile(window->screen_shader,
    //     "#version 330 core\n"
    //     "out vec2 vertex_tex_coords;\n"
    //     "void main() {\n"
    //         "vec2 vertices[3] = vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3));\n"
    //         "gl_Position = vec4(vertices[gl_VertexID], 0, 1);\n"
    //         "vertex_tex_coords = 0.5 * gl_Position.xy + vec2(0.5);\n"
    //     "}",
    //     "#version 330 core\n"
    //     "uniform sampler2D u_texture;\n"
    //     "in vec2 vertex_tex_coords;\n"
    //     "void main() {\n"
    //         "gl_FragColor = texture(u_texture, vertex_tex_coords);\n"
    //     "}");

	// window->default_framebuffer = graphics_framebuffer_create();
	
    // window->default_framebuffer_color_texture = graphics_texture_create();
    // graphics_texture_set(window->default_framebuffer_color_texture, window->size, TEXTURE_FORMAT_RGB, NULL);
	// graphics_framebuffer_attach_color_texture(window->default_framebuffer, window->default_framebuffer_color_texture, 0);
	
    // window->default_framebuffer_depth_texture = graphics_texture_create();
    // graphics_texture_set(window->default_framebuffer_depth_texture, window->size, TEXTURE_FORMAT_DEPTH_STENCIL, NULL);
	// graphics_framebuffer_attach_depth_texture(window->default_framebuffer, window->default_framebuffer_depth_texture);

	// glGenVertexArrays(1, &window->gl_screen_vao);
	return window_surface;
}

void gfx_window_surface_destroy(struct gfx_window_surface_st** window_surface) {
	// graphics_shader_destroy(&(*window)->screen_shader);
	// graphics_framebuffer_destroy(&(*window)->default_framebuffer);
	// graphics_texture_destroy(&(*window)->default_framebuffer_color_texture);
	// graphics_texture_destroy(&(*window)->default_framebuffer_depth_texture);
	// glDeleteVertexArrays(1, &(*window)->gl_screen_vao);
	free(*window_surface);
	*window_surface = NULL;
}

void gfx_window_surface_get_resolution(const struct gfx_window_surface_st* window_surface) {
	// return graphics_texture_get_size(window->default_framebuffer_color_texture);
}

void gfx_window_surface_set_resolution(struct gfx_window_surface_st* window_surface) {
	// graphics_texture_set(window->default_framebuffer_color_texture, resolution, TEXTURE_FORMAT_RGB, NULL);
	// graphics_texture_set(window->default_framebuffer_depth_texture, resolution, TEXTURE_FORMAT_DEPTH_STENCIL, NULL);
}

void gfx_window_surface_present(struct gfx_window_surface_st* window_surface) {
	// GLboolean gl_depth_test_enabled;
	// glGetBooleanv(GL_DEPTH_TEST, &gl_depth_test_enabled);
	// glDisable(GL_DEPTH_TEST);
	
	// glViewport(0, 0, window->size.x, window->size.y);

	// glClearColor(1.0f, 0, 0, 1.0f);

	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// glClear(GL_COLOR_BUFFER_BIT);

	// graphics_shader_enable(window->screen_shader);
	// graphics_shader_set_sampler2d_texture(window->screen_shader, "u_texture", window->default_framebuffer_color_texture);
	// glBindVertexArray(window->gl_screen_vao);
    // glDrawArrays(GL_TRIANGLES, 0, 3);

	// struct point_st screen_resolution = graphics_texture_get_size(window->default_framebuffer_color_texture);
    // graphics_set_viewport(point_make(0, 0), screen_resolution);
	
    // graphics_framebuffer_enable(window->default_framebuffer);

	// if (gl_depth_test_enabled)
	// 	glEnable(GL_DEPTH_TEST);

	SwapBuffers(window_surface->window->hdc);
}
