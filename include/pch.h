#pragma once

// --- Standard C++ Library Headers ---
// General utilities, input/output, containers
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <filesystem>


// --- OpenGL/Graphics Libraries ---
// Include the most common headers used across OpenGL projects.
// NOTE: You may need to adjust these includes based on your specific setup (e.g., using GLFW, SDL, or a different GL loader).

// ** For Modern OpenGL with a loader like GLEW or GLAD **
// Include the GL loader first, as it typically includes the necessary <GL/gl.h> or similar.
#include <glad/glad.h>
#include <GLFW/glfw3.h> // For window and context management

// ** Optional: Common Math Library (e.g., GLM) **
// Including this greatly simplifies vector/matrix math.
#define GLM_FORCE_RADIANS // Good practice for consistency
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <spdlog/spdlog.h>
// --- Project-Specific Headers (Optional) ---
// If you have a few core classes that rarely change, you can add them here.
// e.g., #include "shader.h"
// e.g., #include "camera.h"
