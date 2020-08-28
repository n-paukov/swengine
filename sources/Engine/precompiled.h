//#ifndef USE_PRECOMPILED_HEADERS
//#pragma error
//#endif

#if defined __cplusplus
// Standard library
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <unordered_set>
#include <functional>
#include <memory>
#include <algorithm>
#include <cmath>
#include <array>
#include <type_traits>
#include <typeindex>
#include <fstream>
#include <sstream>
#include <utility>

// Third-party dependencies
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <SDL.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include <pugixml.hpp>
#include <stb_image.h>
#include <spdlog/spdlog.h>

#include <btBulletDynamicsCommon.h>

// Engine headers to precompile
#include "Utility/helpers.h"
#include "swdebug.h"

#endif
