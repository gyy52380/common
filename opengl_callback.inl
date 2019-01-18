// including windows is difficult
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef near
#undef far

#include <stdio.h>

#include "GL/glew_static.h"

static void APIENTRY opengl_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (type == GL_DEBUG_TYPE_OTHER_ARB) return;

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_LOW_ARB:     fprintf(stderr, "[LOW] ");    break;
		case GL_DEBUG_SEVERITY_MEDIUM_ARB:  fprintf(stderr, "[MEDIUM] "); break;
		case GL_DEBUG_SEVERITY_HIGH_ARB:    fprintf(stderr, "[HIGH] ");   break;
	}

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR_ARB:               fprintf(stderr, "ERROR: ");               break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: fprintf(stderr, "DEPRECATED_BEHAVIOR: "); break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  fprintf(stderr, "UNDEFINED_BEHAVIOR: ");  break;
		case GL_DEBUG_TYPE_PORTABILITY_ARB:         fprintf(stderr, "PORTABILITY: ");         break;
		case GL_DEBUG_TYPE_PERFORMANCE_ARB:         fprintf(stderr, "PERFORMANCE: ");         break;
		case GL_DEBUG_TYPE_OTHER_ARB:               fprintf(stderr, "OTHER: ");               break;
	}

	fprintf(stderr, "id=0x%u %s\n", (unsigned int)id, message);
}