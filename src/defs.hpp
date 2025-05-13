#pragma once
#include <unordered_map>
#include <fx/gltf.h>
#define USE_VRMC_VRM_0_0
#define USE_VRMC_VRM_1_0
#include <VRMC/VRM.h>


const std::unordered_map<fx::gltf::Accessor::Type, size_t> numTypeComponents = {
    { fx::gltf::Accessor::Type::Scalar, 1 },
    { fx::gltf::Accessor::Type::Vec2,   2 },
    { fx::gltf::Accessor::Type::Vec3,   3 },
    { fx::gltf::Accessor::Type::Vec4,   4 },
    { fx::gltf::Accessor::Type::Mat2,   4 },
    { fx::gltf::Accessor::Type::Mat3,   9 },
    { fx::gltf::Accessor::Type::Mat4,   16},
};

const std::unordered_map<fx::gltf::Accessor::ComponentType, size_t> numTypeBytes = {
    { fx::gltf::Accessor::ComponentType::Byte,          1 },
    { fx::gltf::Accessor::ComponentType::UnsignedByte,  1 },
    { fx::gltf::Accessor::ComponentType::Short,         2 },
    { fx::gltf::Accessor::ComponentType::UnsignedShort, 2 },
    { fx::gltf::Accessor::ComponentType::Float,         4 },
    { fx::gltf::Accessor::ComponentType::UnsignedInt,   4 },
};

struct VRMModel {
    VRMC_VRM_0_0::Vrm vrm00;
    VRMC_VRM_1_0::Vrm vrm10;
    fx::gltf::Document doc;
};

// TODO: Animation mapping Unity-fbx to VRM.Humanoid
struct VRMAnimation {
    fx::gltf::Document doc;
    std::vector<fx::gltf::Animation::Sampler> samplers;
    std::vector<fx::gltf::Animation::Channel> channels;
    std::string name;
    float duration;
};
