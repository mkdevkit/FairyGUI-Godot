#ifndef __GODOT_TYPES_H__
#define __GODOT_TYPES_H__

// Godot type mappings for FairyGUI port from cocos2d-x
// This header provides type aliases to bridge cocos2d API to Godot API

// STL includes required by many FairyGUI headers
#include <functional>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <climits>

// Godot engine core headers
#include "core/object/ref_counted.h"
#include "core/object/object.h"
#include "core/os/memory.h"
#include "core/io/image.h"
#include "core/io/file_access.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/math/rect2.h"
#include "core/math/color.h"
#include "core/math/transform_2d.h"
#include "core/variant/variant.h"
#include "core/variant/array.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

// Scene headers
#include "scene/main/node.h"
#include "scene/2d/node_2d.h"
#include "scene/main/canvas_item.h"
#include "scene/gui/control.h"
#include "scene/resources/texture.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/style_box.h"
#include "scene/resources/font.h"
#include "scene/2d/sprite_2d.h"
#include "scene/2d/camera_2d.h"
#include "core/math/vector4.h"

// C++ does not typedef `Rect` in the engine, add an alias.
// Code uses `Rect` as an alias for `Rect2`.
using Rect = Rect2;

// Empty string constant (replaces EMPTY_STRING)
const std::string EMPTY_STRING;

// Math utility macros
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define MATH_DEG_TO_RAD(angle) ((angle) * 3.14159265358979323846f / 180.0f)

// Godot memory helper
#define FGUI_CREATE(T) memnew(T)
#define FGUI_DELETE(T) do { delete (T); (T) = nullptr; } while(0)

#endif
