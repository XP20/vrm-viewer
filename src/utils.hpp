#pragma once
#include "defs.hpp"
#include <iostream>


void BufferData(const fx::gltf::Document &doc, const fx::gltf::Accessor &accessor, void *pDest);
void BufferDataArray(const fx::gltf::Document &doc, const fx::gltf::Accessor &accessor, void *pDest, size_t align);
VRMModel LoadVRM(const std::string &path);
