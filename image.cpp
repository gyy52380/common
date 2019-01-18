#include "image.h"

#include <stdlib.h>
#include <memory.h>

#include "GL/glew_static.h"

#include "libraries/stb_image.h"

#include "common.h"
#include "math.h"


namespace img
{


// all of these functions assume RGBA, all the time, forever
// im lazy

GLuint make_texture_from_pixel_data(u8* pixel_data, i32 width, i32 height)
{
	if (!pixel_data)
		return 0;

	GLuint id = 0;

	glGenTextures(1, (GLuint*) &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);

	//float border_color[4] = { 1, 0, 0, 0.5 };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

	// basic trilinear filtering, no need to make a mipmap
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	return id;
}

void reload_texture_from_image(Image *image)
{
	if (!image->data)
		return;
	if (!image->id)
		return;

	glBindTexture(GL_TEXTURE_2D, image->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void make_texture_from_image(Image *image)
{
	if (image->data)
		image->id = make_texture_from_pixel_data(image->data, image->width, image->height);
	else
		image->id = 0;
}

Image make_image_from_file(const char* path, bool keep_pixel_data)
{
	Image image;

	image.data = stbi_load(path, &image.width, &image.height, &image.channel_count, STBI_rgb_alpha);
	if (image.data == NULL)
	{
		printf("Cant load image file for texture: \"%s\"\n", path);
		printf("STBI_FAILURE_REASON: %s\n", stbi_failure_reason());
		return image;
	}

	// generate opengl texture
	make_texture_from_image(&image);

	if (!keep_pixel_data)
		stbi_image_free(image.data);

	return image;
}


Image make_blank_image(u32 width, u32 height)
{
	Image image;
	image.width = width;
	image.height = height;
	image.channel_count = 4;
	image.data = (u8*) calloc(width * height * image.channel_count, sizeof(u8));
	return image;
}

Image make_blank_image(Region* region_ptr, u32 width, u32 height)
{
	Image image;
	image.width = width;
	image.height = height;
	image.channel_count = 4;
	image.data = RegionArray(region_ptr, u8, width * height * image.channel_count);

	return image;
}

inline void color_a_pixel(u8* pixel, u32 color, u32 channel_count)
{
	for (u32 channel = 0; channel < channel_count; channel++)
	{
		u8 	color_component = (u8)(color >> (channel * 8));
		u8* pixel_component = pixel + channel;
		*pixel_component = color_component;
	}
}

// color a pixel at a pixel offset from top left corner of image
inline void set_pixel_color(Image* image, Vec2i32 pixel_coords, u32 color)
{
	if (pixel_coords.x < 0 || pixel_coords.x >= image->width)
		return;
	if (pixel_coords.y < 0 || pixel_coords.y >= image->height)
		return;

	const u32 stride = image->width * image->channel_count;

	u8* pixel = image->data + (pixel_coords.y * stride) + (pixel_coords.x * image->channel_count);

	for (u32 channel = 0; channel < image->channel_count; channel++)
	{
		u8 	color_component = (u8)(color >> (channel * 8));
		u8* pixel_component = pixel + channel;
		*pixel_component = color_component;
	}
}

void clear(Image *image)
{
	for (i32 y = 0; y < image->height; y++)
		for (i32 x = 0; x < image->width; x++)
			set_pixel_color(image, {x, y}, 0);
}

// both points are included: [top_left, bottom_right]
void draw_rectangle(Image* image, Vec2i32 top_left, Vec2i32 bot_right, u32 color, u32 thickness)
{
	// reduce thickness if rectangle is thicker than its dimensions
	const u32 rectangle_width  = bot_right.x - top_left.x + 1;
	const u32 rectangle_height = bot_right.y - top_left.y + 1;

	const u32 thickness_top_bottom = thickness <= rectangle_height ? thickness : rectangle_height;
	const u32 thickness_left_right = thickness <= rectangle_width  ? thickness : rectangle_width;

	// draw the the top and bottom side
	for (i32 x = top_left.x; x <= bot_right.x; x++)
	{
		for (i32 i = 0; i < thickness_top_bottom; i++)
		{
			Vec2i32 pixel_coords_top = {x, top_left.y  + i};
			Vec2i32 pixel_coords_bot = {x, bot_right.y - i};

			set_pixel_color(image, pixel_coords_top, color);
			set_pixel_color(image, pixel_coords_bot, color);
		}
	}

	// draw the left and right side
	for (i32 y = top_left.y; y <= bot_right.y; y++)
	{
		for (i32 i = 0; i < thickness_left_right; i++)
		{
			Vec2i32 pixel_coords_left  = {top_left.x  + i, y};
			Vec2i32 pixel_coords_right = {bot_right.x - i, y};

			set_pixel_color(image, pixel_coords_left, color);
			set_pixel_color(image, pixel_coords_right, color);
		}

	}
}


}