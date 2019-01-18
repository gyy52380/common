#include "platform/lk_platform.h"

#include <math.h>
#include <stdio.h>
#include <assert.h>


#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef near
#undef far

#include "GL/glew_static.h"

#include "imgui/my_imgui_implementation.h"

#include "common.h"
#include "math.h"

#include "image.h"
#include "boxes.h"
#include "ui.h"


#include "Persistent_Data.h"
#define DATA (*((Persistent_Data*)platform->client_data))

LK_CLIENT_EXPORT void lk_client_init(LK_Platform* platform)
{
    platform->window.title = (char*)"Video Label";
    platform->window.backend = LK_WINDOW_OPENGL;

    platform->opengl.major_version = 3;
    platform->opengl.minor_version = 3;
    platform->opengl.swap_interval = 1;
    platform->opengl.sample_count  = 4;

    platform->client_data = new Persistent_Data;

    // called once at startup to make the .exe dir the root for all paths
    auto set_root_directory = []() -> void
    {
        int size = MAX_PATH + 1;
        char* dir = (char*) LocalAlloc(LMEM_FIXED, size);
        while (GetModuleFileNameA(NULL, dir, size) == size)
        {
            LocalFree(dir);
            size *= 2;
            dir = (char*) LocalAlloc(LMEM_FIXED, size);
        }

        char* c = dir;
        while (*c) c++;
        while (c > dir)
        {
            c--;
            if (*c == '/' || *c == '\\')
            {
                *(c + 1) = 0;
                break;
            }
        }

        SetCurrentDirectoryA(dir);
    };
    set_root_directory();
}


#include "opengl_callback.inl" // opengl error reporting implementation
LK_CLIENT_EXPORT void lk_client_frame(LK_Platform* platform)
{
    ////////
    // -- SYSTEM INIT
    ////////

    static bool first = true;
    if (first)
    {
        first = false;

        // -- opengl init
        glewExperimental = GL_TRUE;
        glewInit();
        // opengl error reporting callback
        if (glDebugMessageCallbackARB)
        {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
            glDebugMessageCallbackARB(opengl_callback, NULL);
            GLuint ids;
            glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &ids, true);
        }

        // -- systems init
        imgui_init();
        

        // -- logic init
        DATA.image = img::make_image_from_file("../../../test_images/Capture.jpg");
        DATA.box_context = make_box_context(DATA.image.width, DATA.image.height);
    }


    //
    // -- MAIN PROGRAM LOOP
    //

    // rewind the temporary memory stack every frame
    Scoped_Region_Cursor temporary_memory_killer(temp);


    imgui_new_frame(platform);
    ImGui::ShowTestWindow();


    ImGui::Begin("Image View");
    {
        // container for rendering, zooming, panning current video frame
        static ui::Image_View image_view;
		image_view.image = &DATA.image;
        image_view.update_position_and_mouse_inputs();

		ImGui::Text("Zoom: %.2f %%", image_view.zoom_scale * 100);

        u32 view_width = ImGui::GetWindowWidth();
		image_view.draw(view_width, 0);

        if (ImGui::IsItemHovered())
        {
            if (!platform->keyboard.state[LK_KEY_SPACE].down)
            {
                Vec2i32 pixel = vec2i32(image_view.pixel_under_mouse);
                change_active_box_with_mouse(&DATA.box_context, platform, pixel);
            }
        }

        redraw_canvas(&DATA.box_context);

        // canvas draws boxes over the screen
		ui::Image_View canvas_view = image_view;
        canvas_view.image = &DATA.box_context.canvas;

		canvas_view.draw(view_width, 0);
    }
    ImGui::End();


    // -- group view window
    //    -- used for selecting the labeling class of the box and removing wrong boxes by the user

    // GROUPS CURRENTLY LEAK MEMORY IF REMOVED BY USER! (because of stupid string names)
    ImGui::Begin("Group View");
    {
        static const auto COLOR_PICKER_FLAGS = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview;
        static char  input_text_buffer[1024];
        static u32   input_color_buffer = 0xAA0000FF;

        // -- add group sub-view
        ImGui::Text("Add a group:");

        ui::imgui::ColorEdit4("Group color", &input_color_buffer, COLOR_PICKER_FLAGS);
        ImGui::InputText("Group name", input_text_buffer, ArrayCount(input_text_buffer));
        if (ImGui::Button("Add group"))
        {
            String temp_name = make_string(input_text_buffer);
            ZeroStaticArray(input_text_buffer);

            add_box_group(&DATA.box_context, temp_name, input_color_buffer);
        }

        // -- active group and box sub-view
        ImGui::Text("\n Select active group:");
        for (auto* group_ptr : DATA.box_context.all_box_groups)
        {   
            ImGui::PushID(group_ptr);

            // radio button for switching the active group
            if (ImGui::RadioButton(make_c_style_string(group_ptr->name), group_ptr == DATA.box_context.active_group))
            {
                DATA.box_context.active_group = group_ptr; 
            }
            ImGui::SameLine();

            // colored square that shows the group color
            ui::imgui::ColorEdit4("", &group_ptr->color, COLOR_PICKER_FLAGS);
            // collapsing header with boxes and delete buttons
            // TODO render tiny boxes next to remove button
            ImGui::Indent();
            if (ImGui::CollapsingHeader("boxes"))
            {
                int counter = 0; // counter for enumerating "remove box" labels, can be removed later
                for (auto box_ptr : group_ptr->boxes)
                {
                    ImGui::PushID(box_ptr);

                    ImGui::Text("Box %d\n", counter);
                    ImGui::SameLine();
                    
                    if (ImGui::Button("Remove"))
                    {
                        remove_box(&DATA.box_context, box_ptr, group_ptr);
                    }
                    
                    ImGui::PopID(); // box id
                    counter++;                
                }
            }
            ImGui::Unindent();

            ImGui::PopID(); // box_group id
        }
    }
    ImGui::End();


    ImGui::Begin("Testing Window");
    {
        if (ImGui::TreeNode("a node"))
        {
            ImGui::Text("tree node open?");
            ImGui::Text("tree node open?");
            ImGui::Text("tree node open?");
            ImGui::TreePop();
        }
    }
    ImGui::End();


    glClearColor(70.0f/255.0f, 130.0f/255.0f, 180.0f/255.0f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
}
