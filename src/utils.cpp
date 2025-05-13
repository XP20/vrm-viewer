#include "utils.hpp"


void BufferData(const fx::gltf::Document &doc, const fx::gltf::Accessor &accessor, void *pDest) {
    const fx::gltf::BufferView &bufferView = doc.bufferViews[accessor.bufferView];
    const fx::gltf::Buffer &buffer = doc.buffers[bufferView.buffer];

    const size_t size = numTypeComponents.at(accessor.type) * numTypeBytes.at(accessor.componentType);
    const uint8_t *pData = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
    std::memcpy(pDest, pData, accessor.count * size);
}

void BufferDataArray(const fx::gltf::Document &doc, const fx::gltf::Accessor &accessor, void *pDest, size_t align) {
    const fx::gltf::BufferView &bufferView = doc.bufferViews[accessor.bufferView];
    const fx::gltf::Buffer &buffer = doc.buffers[bufferView.buffer];

    const size_t size = accessor.count * numTypeComponents.at(accessor.type);
    const uint8_t *pData = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
    const size_t byteCount = numTypeBytes.at(accessor.componentType);
    for (size_t i = 0; i < size; i++) std::memcpy(pDest + i*align, pData + i*byteCount, align);
}

VRMModel LoadVRM(const std::string &path) {
    VRMModel model;
    model.doc = fx::gltf::LoadFromBinary(path.c_str(), { UINT32_MAX, UINT32_MAX, UINT32_MAX });
    const auto &exts = model.doc.extensionsAndExtras["extensions"];

    if (exts.contains("VRMC_vrm")) {
        VRMC_VRM_1_0::from_json(exts["VRMC_vrm"], model.vrm10);
    } else if (exts.contains("VRM")) {
        VRMC_VRM_0_0::from_json(exts["VRM"], model.vrm00);
    } else {
        std::cerr << "No VRM signature found" << std::endl;
    }

    return model;
}
