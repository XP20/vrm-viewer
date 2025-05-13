#include "mesh.hpp"
#include "defs.hpp"
#include <raymath.h>


void LoadModelMeshes(Model &model, const fx::gltf::Document &doc) {
    model.meshCount = 0;
    for (const auto &mesh : doc.meshes) { model.meshCount += mesh.primitives.size(); }
    model.meshes = (Mesh*)malloc(model.meshCount * sizeof(Mesh));
    model.meshMaterial = (int*)malloc(model.meshCount * sizeof(int));

    size_t idx = 0;
    for (const fx::gltf::Mesh &mesh : doc.meshes) {
        for (const fx::gltf::Primitive &primitive : mesh.primitives) {
            Mesh rlMesh = {0};

            const fx::gltf::Accessor &accessor = doc.accessors[primitive.indices];
            const size_t numElements = accessor.count * numTypeComponents.at(accessor.type);
            rlMesh.triangleCount = numElements / 3;
            rlMesh.indices = (unsigned short*)malloc(numElements * sizeof(unsigned short));
            BufferDataArray(doc, accessor, rlMesh.indices, sizeof(unsigned short));

            for (const auto &vertex : primitive.attributes) {
                const fx::gltf::Accessor &accessor = doc.accessors[vertex.second];
                const size_t size = numTypeComponents.at(accessor.type) * numTypeBytes.at(accessor.componentType);
                const size_t bytes = accessor.count * size;

                if (vertex.first == "POSITION") {
                    rlMesh.vertices = (float*)malloc(bytes);
                    BufferData(doc, accessor, rlMesh.vertices);
                    rlMesh.vertexCount = accessor.count;
                } else if (vertex.first == "NORMAL") {
                    rlMesh.normals = (float*)malloc(bytes);
                    BufferData(doc, accessor, rlMesh.normals);
                } else if (vertex.first == "TEXCOORD_0") {
                    rlMesh.texcoords = (float*)malloc(bytes);
                    BufferData(doc, accessor, rlMesh.texcoords);
                } else if (vertex.first == "JOINTS_0") {
                    rlMesh.boneIds = (unsigned char*)malloc(bytes);
                    BufferData(doc, accessor, rlMesh.boneIds);
                } else if (vertex.first == "WEIGHTS_0") {
                    rlMesh.boneWeights = (float*)malloc(bytes);
                    BufferData(doc, accessor, rlMesh.boneWeights);
                }
            }

            UploadMesh(&rlMesh, false);
            model.meshes[idx] = rlMesh;
            model.meshMaterial[idx] = primitive.material;
            idx++;
        }
    }
}

void LoadModelMaterials(Model &model, const fx::gltf::Document &doc) {
    model.materialCount = doc.materials.size();
    model.materials = (Material*)malloc(model.materialCount * sizeof(Material));

    std::vector<Image> images;
    for (const fx::gltf::Image &image : doc.images) {
        const fx::gltf::BufferView &bufferView = doc.bufferViews[image.bufferView];
        const fx::gltf::Buffer &buffer = doc.buffers[bufferView.buffer];
        const unsigned char* data = &buffer.data[bufferView.byteOffset];

        const char* extension;
        if (image.mimeType == "image/png") { extension = ".png"; }
        else if (image.mimeType == "image/jpg") { extension = ".jpg"; }
        else if (image.mimeType == "image/jpeg") { extension = ".jpeg"; }

        Image rlImage = LoadImageFromMemory(extension, data, bufferView.byteLength);
        images.push_back(rlImage);
    }

    std::vector<Texture2D> textures;
    for (const fx::gltf::Texture &texture : doc.textures) {
        const Image &image = images[texture.source];
        Texture2D rlTexture = LoadTextureFromImage(image);
        UnloadImage(image);

        // TODO: Sampler

        GenTextureMipmaps(&rlTexture);
        SetTextureFilter(rlTexture, TEXTURE_FILTER_ANISOTROPIC_16X);  // Sharper but more flicker
        // SetTextureFilter(rlTexture, TEXTURE_FILTER_TRILINEAR);  // Softer but less flicker
        textures.push_back(rlTexture);
    }

    size_t idx = 0;
    for (const fx::gltf::Material &material : doc.materials) {
        const size_t textureIdx = material.pbrMetallicRoughness.baseColorTexture.index;
        const Texture2D &texture = textures[textureIdx];

        // TODO: Other material properties

        Material rlMaterial = LoadMaterialDefault();
        SetMaterialTexture(&rlMaterial, MATERIAL_MAP_DIFFUSE, texture);
        model.materials[idx] = rlMaterial;
        idx++;
    }
}

void LoadModelSkeleton(Model &model, const fx::gltf::Document &doc) {
    //! Assuming only one skinned mesh
    const fx::gltf::Skin &skin = doc.skins[0];
    model.boneCount = skin.joints.size();
    model.bones = (BoneInfo*)malloc(model.boneCount * sizeof(BoneInfo));
    model.bindPose = (Transform*)malloc(model.boneCount * sizeof(Transform));

    // TODO Handle this
    std::vector<Matrix> inverseBindMatrices;
    if (skin.inverseBindMatrices >= 0) {
        const auto &accessor = doc.accessors[skin.inverseBindMatrices];
        inverseBindMatrices.resize(accessor.count);
        BufferData(doc, accessor, inverseBindMatrices.data());
    }

    for (size_t i = 0; i < model.boneCount; i++) {
        const uint32_t joint = skin.joints[i];
        const fx::gltf::Node &node = doc.nodes[joint];

        // TODO Skip? zeroing bone data, alt ={0};
        BoneInfo &bone = model.bones[i];
        std::memset(&bone, 0, sizeof(BoneInfo));

        // TODO Skip? setting bone name
        std::strncpy(bone.name, node.name.c_str(), 31);
        bone.name[31] = '\0';

        // TODO Fix finding bone parent
        bone.parent = -1;
        for (size_t j = 0; j < model.boneCount; j++) {
            if (doc.nodes[skin.joints[j]].children.end() !=
                std::find(doc.nodes[skin.joints[j]].children.begin(), doc.nodes[skin.joints[j]].children.end(), joint)) {
                    bone.parent = j;
                    break;
                }
        }
        if (bone.parent == -1) std::cout << "Didn't find bone parent" << std::endl;

        // TODO Simplify transform from glTF node
        Matrix mat = MatrixIdentity();
        if (!node.matrix.empty()) {
            mat = Matrix{
                node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
                node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
                node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
                node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]
            };
        } else {
            Matrix t = MatrixTranslate(node.translation[0], node.translation[1], node.translation[2]);
            Quaternion q = { node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3] };
            Matrix r = QuaternionToMatrix(q);
            Matrix s = MatrixScale(node.scale[0], node.scale[1], node.scale[2]);
            mat = MatrixMultiply(mat, s);
            mat = MatrixMultiply(mat, r);
            mat = MatrixMultiply(mat, t);
            //mat = MatrixMultiply(t, MatrixMultiply(r, s));  // T * R * S backwards order
        }
        model.bindPose[i] = (Transform){
            .translation = { mat.m12, mat.m13, mat.m14 },
            .rotation = QuaternionFromMatrix(mat),
            .scale = { 1.0f, 1.0f, 1.0f }  // TODO Get real scale
        };
    }
}

Model ModelFromVRM(const VRMModel &vrm) {
    const fx::gltf::Document &doc = vrm.doc;
    Model model = {0};

    LoadModelMeshes(model, doc);
    LoadModelMaterials(model, doc);
    LoadModelSkeleton(model, doc);

    model.transform = MatrixIdentity();

    return model;

    //? Loading model skeleton?
    // TODO: doc.animations
    // TODO: doc.skins
    // TODO: doc.asset
    // rlModel.boneCount = somecount;
    // rlModel.bones = malloced bones array;
    // BoneInfo bone = { char name[32], int parent };
    // rlModel.bindPose = initial bone pose transform array;>

    // TODO leftover
    // std::vector<Texture> textures{};
        // std::vector<Material> materials{};
            // std::vector<Sampler> samplers{};
            // std::vector<Skin> skins{};
            // std::vector<Node> nodes{};
            // std::vector<Animation> animations{};
            // std::vector<Camera> cameras{};
            // std::vector<Scene> scenes{};
}
