#pragma once
#include <vector>
#include <map>
#include "../definitions/components/shader_object.hpp"

namespace shaders {


TwoShaderProgram static_object_program("texture_2d_v.glsl", "texture_2d_f.glsl");
TwoShaderProgram mipmap_program("texture_2d_v.glsl", "texture_2d_f.glsl");
TwoShaderProgram visibility_program("texture_camera_2d_v.glsl", "texture_2d_f.glsl");
TwoShaderProgram raycast_program("raycast_2d_v.glsl", "raycast_2d_f.glsl");
TwoShaderProgram bezier_program("bezier_2d_v.glsl", "bezier_2d_f.glsl");
TwoShaderProgram bezier_raycast_program("bezier_2d_v.glsl", "bezier_raycast_2d_f.glsl");


}
