#pragma once
#include "utils.hpp"
#include <raylib.h>
#include <fx/gltf.h>
#define USE_VRMC_VRM_0_0
#define USE_VRMC_VRM_1_0
#include <VRMC/VRM.h>


void LoadModelMeshes(Model &model, const fx::gltf::Document &doc);
void LoadModelMaterials(Model &model, const fx::gltf::Document &doc);
void LoadModelSkeleton(Model &model, const fx::gltf::Document &doc);
Model ModelFromVRM(const VRMModel &vrm);
