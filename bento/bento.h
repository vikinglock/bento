#pragma once

#include "backend/vao.h"

#include "backend/file/file.h"

#ifdef USE_METAL
#include "backend/metal/metal.h"
#elif USE_OPENGL
#include "backend/opengl/opengl.h"
#elif USE_VULKAN
#include "backend/vulkan/vulkan.h"
#else
#include "backend/opengl/opengl.h"
#endif

#ifdef WINDOWS
constexpr double pi = 3.14159265358979323;
constexpr float fpi = 3.14159265f;
#else
#define pi 3.14159265358979323
#define fpi 3.14159265f
#endif

//also very sorry if this is a really unorthodox method of drawing stuff or handling graphics        but i don't care

#include "backend/sound/sound.h"

#ifdef IOS
#define main reallylonganduniquenameforbentosoyoullneveraccidentallymakeafunctionforthis
#endif