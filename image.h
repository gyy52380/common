#pragma once

#include "GL/glew_static.h"

#include "common.h"
#include "math.h"

struct Image
{
	umm id = 0;
	i32 width;
	i32 height;
	i32 channel_count; // actual channel_count in source image
	u8 *data = NULL;
};
	

// all of these functions assume RGBA, all the time
namespace img
{


Image make_image_from_file(const char* path, bool keep_pixel_data=false);

Image make_blank_image(u32 width, u32 height);
Image make_blank_image(Region* region_ptr, u32 width, u32 height);

void make_texture_from_image(Image *image);
void reload_texture_from_image(Image *image);
GLuint make_texture_from_pixel_data(u8* pixel_data, i32 width, i32 height);

void clear(Image *image);
void draw_rectangle(Image* image, Vec2i32 top_left, Vec2i32 bot_right, u32 color, u32 thickness=1);


}