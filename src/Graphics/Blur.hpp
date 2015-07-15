#pragma once

#include <Texture2D.hpp>
#include <CubeMap.hpp>

void blur(const Texture2D& t, size_t resx, size_t resy = 0, unsigned int level = 0);

/**
 * This is very wrong, it just blur each face separatly.
**/
void blur(const CubeMap& t, size_t resx, size_t resy = 0, unsigned int level = 0);
