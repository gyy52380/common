#include "my_imgui_implementation.h"

#include <stdio.h>

#include "imconfig.h"
#include "imgui.h"

#include "../GL/glew_static.h"
#include "../platform/lk_platform.h"

//#include "imgui.cpp"
//#include "imgui_draw.cpp"
//#include "imgui_demo.cpp"

static GLuint font_texture = 0;
static GLuint imgui_shader_handle;

void render_draw_lists(ImDrawData* draw_data)
{
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
    glActiveTexture(GL_TEXTURE0);
    GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
    GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
    GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glViewport(0, 0, fb_width, fb_height);

    float R = io.DisplaySize.x;
    float B = io.DisplaySize.y;
    float orthographic_matrix[4][4] =
    {
        {  2.0f / R,  0.0f,      0.0f,     0.0f },
        {  0.0f,      2.0f / -B, 0.0f,     0.0f },
        {  0.0f,      0.0f,     -1.0f,     0.0f },
        { -1.0f,      1.0f,      0.0f,     1.0f },
    };

    glUseProgram(imgui_shader_handle);
    int uniform_tex         = glGetUniformLocation(imgui_shader_handle, "Texture");
    int uniform_projmtx     = glGetUniformLocation(imgui_shader_handle, "ProjMtx");
    int attribute_position  = glGetAttribLocation(imgui_shader_handle, "Position");
    int attribute_uv        = glGetAttribLocation(imgui_shader_handle, "UV");
    int attribute_color     = glGetAttribLocation(imgui_shader_handle, "Color");

    glUniform1i(uniform_tex, 0);
    glUniformMatrix4fv(uniform_projmtx, 1, GL_FALSE, &orthographic_matrix[0][0]);
    glBindSampler(0, 0);

    GLuint vbo, ibo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(attribute_position);
    glEnableVertexAttribArray(attribute_uv);
    glEnableVertexAttribArray(attribute_color);
    glVertexAttribPointer(attribute_position, 2, GL_FLOAT,         GL_FALSE, sizeof(ImDrawVert), (void*) offsetof(ImDrawVert, pos));
    glVertexAttribPointer(attribute_uv,       2, GL_FLOAT,         GL_FALSE, sizeof(ImDrawVert), (void*) offsetof(ImDrawVert, uv));
    glVertexAttribPointer(attribute_color,    4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(ImDrawVert), (void*) offsetof(ImDrawVert, col));

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* index_offset = NULL;

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
            if (cmd->UserCallback)
            {
                cmd->UserCallback(cmd_list, cmd);
            }
            else
            {
                ImVec4 clip_rect = ImVec4(cmd->ClipRect.x, cmd->ClipRect.y, cmd->ClipRect.z, cmd->ClipRect.w);
                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));

                    GLuint texture = (GLuint)(intptr_t) cmd->TextureId;
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glDrawElements(GL_TRIANGLES, cmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, index_offset);
                }
            }

            index_offset += cmd->ElemCount;
        }
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);

    glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindSampler(0, last_sampler);
    glActiveTexture(last_active_texture);
    glBindVertexArray(last_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

void check_shader(GLuint handle, const char* name)
{
    GLint status = 0, log_length = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);

    if (!status)
    {
        fprintf(stderr, "OpenGL: failed to compile %s!\n", name);
    }

    if (log_length > 0)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetShaderInfoLog(handle, log_length, NULL, (GLchar*) buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
}

void check_program(GLuint handle, const char* name)
{
    GLint status = 0, log_length = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);

    if (!status)
    {
        fprintf(stderr, "OpenGL: failed to link %s!\n", name);
    }

    if (log_length > 0)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetProgramInfoLog(handle, log_length, NULL, (GLchar*) buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
}

void imgui_create_opengl_stuff()
{
    GLint last_texture, last_array_buffer, last_vertex_array;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    const GLchar* vertex_shader = R"shader(
        #version 130

        uniform mat4 ProjMtx;

        in vec2 Position;
        in vec2 UV;
        in vec4 Color;
        out vec2 Frag_UV;
        out vec4 Frag_Color;

        void main()
        {
            Frag_UV = UV;
            Frag_Color = Color;
            gl_Position = ProjMtx * vec4(Position.xy,0,1);
        }
    )shader";

    const GLchar* fragment_shader = R"shader(
        #version 130

        uniform sampler2D Texture;

        in vec2 Frag_UV;
        in vec4 Frag_Color;
        out vec4 Out_Color;

        void main()
        {
            Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
        }
    )shader";

    GLuint vs_object = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs_object, 1, &vertex_shader, NULL);
    glCompileShader(vs_object);
    check_shader(vs_object, "vertex shader");

    GLuint fs_object = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_object, 1, &fragment_shader, NULL);
    glCompileShader(fs_object);
    check_shader(fs_object, "fragment shader");

    imgui_shader_handle = glCreateProgram();
    glAttachShader(imgui_shader_handle, vs_object);
    glAttachShader(imgui_shader_handle, fs_object);
    glLinkProgram(imgui_shader_handle);
    check_program(imgui_shader_handle, "shader program");

    glDetachShader(imgui_shader_handle, vs_object);
    glDetachShader(imgui_shader_handle, fs_object);
    glDeleteShader(vs_object);
    glDeleteShader(fs_object);


    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)(intptr_t) font_texture;


    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindVertexArray(last_vertex_array);
}

const char* get_clipboard_text(void*)
{
    // @Incomplete
    return "";
}

void set_clipboard_text(void*, const char* text)
{
    // @Incomplete
}

bool imgui_init()
{
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = LK_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = LK_KEY_ARROW_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = LK_KEY_ARROW_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = LK_KEY_ARROW_UP;
    io.KeyMap[ImGuiKey_DownArrow] = LK_KEY_ARROW_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = LK_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = LK_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = LK_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = LK_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = LK_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = LK_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = LK_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = LK_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_Space] = LK_KEY_SPACE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    io.RenderDrawListsFn = render_draw_lists;
    io.SetClipboardTextFn = set_clipboard_text;
    io.GetClipboardTextFn = get_clipboard_text;
    io.ClipboardUserData = NULL;

    return true;
}

void imgui_shutdown()
{
    if (imgui_shader_handle)
        glDeleteProgram(imgui_shader_handle);

    if (font_texture)
        glDeleteTextures(1, &font_texture);

    ImGui::DestroyContext();
}

void imgui_new_frame(LK_Platform* platform)
{
    if (!font_texture)
    {
        imgui_create_opengl_stuff();
    }

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w = platform->window.width;
    int h = platform->window.height;
    int display_w = platform->window.width;
    int display_h = platform->window.height;
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    io.DeltaTime = platform->time.delta_seconds;

    int mx = platform->mouse.x;
    int my = platform->mouse.y;
    io.MousePos = ImVec2((float) mx, (float) my);
    io.MouseDown[0] = platform->mouse.left_button.down;
    io.MouseDown[1] = platform->mouse.right_button.down;

    io.MouseWheel = 0;
    if (platform->mouse.delta_wheel > 0)
        io.MouseWheel = 1;
    if (platform->mouse.delta_wheel < 0)
        io.MouseWheel = -1;

    for (int key = 0; key < LK__KEY_COUNT; key++)
        io.KeysDown[key] = platform->keyboard.state[key].down;
    io.KeyShift = platform->keyboard.state[LK_KEY_LEFT_SHIFT  ].down;
    io.KeyCtrl  = platform->keyboard.state[LK_KEY_LEFT_CONTROL].down;
    io.KeyAlt   = platform->keyboard.state[LK_KEY_LEFT_ALT    ].down;
    io.KeySuper = platform->keyboard.state[LK_KEY_LEFT_WINDOWS].down;

    io.AddInputCharactersUTF8(platform->keyboard.text);

    ImGui::NewFrame();
}
