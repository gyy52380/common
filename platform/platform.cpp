//#define LK_PLATFORM_DLL_NAME "app" we dont use hotcode reloading because its a pain to set it up in visual studio
#define LK_PLATFORM_NO_DLL
#define LK_PLATFORM_IMPLEMENTATION
#include "lk_platform.h"

// this file expands lk_platform.h implementation and is the actual .exe produced.
// the .exe produces only a thin Windows window platform that handles widnowing, user input etc.
// platform.exe also dynamically loads app.dll, from where it runs lk_platform_frame() function every frame
// this allows for .dll hot-reloading. User can compile his application code without closing the platform.exe window