#pragma once

#include "../GL/glew_static.h"
#include "imgui.h"
#include "../platform/lk_platform.h"

// used by the user
bool imgui_init();
void imgui_shutdown();
void imgui_new_frame(LK_Platform* platform);

// not used by the user, but can be useful
void render_draw_lists(ImDrawData* draw_data);
void check_shader(GLuint handle, const char* name);
void check_program(GLuint handle, const char* name);
void imgui_create_opengl_stuff();

// @INCOMPLETE
const char* get_clipboard_text(void*);
void set_clipboard_text(void*, const char* text);