#pragma once
#include "utils.hpp"
#include <raylib.h>
#include <fx/gltf.h>


VRMAnimation LoadAnimationClip(const std::string &path);
void ApplyAnimationToModel(Model &model, const VRMAnimation &clip, const fx::gltf::Document &doc, float time);
