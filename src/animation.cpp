#include "animation.hpp"


VRMAnimation LoadAnimationClip(const std::string &path) {
    VRMAnimation clip;
    clip.doc = fx::gltf::LoadFromBinary(path.c_str(), { UINT32_MAX, UINT32_MAX, UINT32_MAX });

    // TODO Cleanup
    fx::gltf::Document &doc = clip.doc;
    if (doc.animations.empty()) {
        std::cerr << "Animation clip has no animations" << std::endl;
        return {};
    }

    const fx::gltf::Animation &anim = doc.animations[0];
    clip.name = anim.name;
    clip.duration = 0.0f;
    for (auto &sampler : anim.samplers) clip.samplers.push_back(sampler);
    for (auto &channel : anim.channels) clip.channels.push_back(channel);

    for (const auto &sampler : clip.samplers) {
        const auto &accessor = doc.accessors[sampler.input];
        if (accessor.count == 0) continue;

        std::vector<float> times(accessor.count);
        BufferData(doc, accessor, times.data());

        float lastTime = times.back();
        clip.duration = std::max(clip.duration, lastTime);
    }

    return clip;
}

void ApplyAnimationToModel(Model &model, const VRMAnimation &clip, const fx::gltf::Document &doc, float time) {
    for (int i = 0; i < model.boneCount; i++) {
        model.bindPose[i] = {
            .translation = { 0.0f, 0.0f, 0.0f },
            .rotation = { 0.0f, 0.0f, 0.0f, 1.0f },
            .scale = { 1.0f, 1.0f, 1.0f },
        };
    }

    for (size_t i = 0; i < clip.channels.size(); i++) {
        const fx::gltf::Animation::Channel &channel = clip.channels[i];
        const fx::gltf::Animation::Sampler &sampler = clip.samplers[i];

        if (channel.target.node < 0 || channel.target.node >= int(doc.nodes.size())) {
            std::cout << "OOB channel.target.node (" << channel.target.node << "), ignoring" << std::endl;
            continue;
        }

        const fx::gltf::Node &node = doc.nodes[channel.target.node];

        int boneIndex = -1;
        for (int j = 0; j < model.boneCount; j++) {
            // std::cout << "Node name: " << model.bones[j].name << std::endl;
            if (node.name == model.bones[j].name) {
                boneIndex = j;
                break;
            }
        }

        if (boneIndex == -1) {
            // std::cout << "Root bone (" << boneIndex << "), ignoring" << std::endl;
            continue;
        }

        Transform &pose = model.bindPose[boneIndex];
        const fx::gltf::Accessor &aTime = doc.accessors[sampler.input];
        const fx::gltf::Accessor &aTranslation = doc.accessors[sampler.output];

        const size_t frameCount = aTime.count;
        std::cout << "Frames: " << frameCount << std::endl;

        // if (channel.target.path == "translation") {
        //     Vector3 *translations = (Vector3*)malloc(frameCount * sizeof(Vector3));
        // }
    }

    //     Transform &pose = model.bindPose[boneIndex];
    //     const fx::gltf::Accessor &inputAccessor = doc.accessors[sampler.input];
    //     const fx::gltf::Accessor &outputAccessor = doc.accessors[sampler.output];

    //     // Get keyframe count
    //     const size_t frameCount = inputAccessor.count;

    //     // TODO For now, we pick the first frame regardless of 'time'
    //     // TODO "weights"
    //     if (channel.target.path == "translation") {
    //         Vector3 *translations = (Vector3 *)malloc(sizeof(Vector3) * frameCount);
    //         BufferData(doc, outputAccessor, translations);
    //         pose.translation = translations[index];
    //         free(translations);
    //     } else if (channel.target.path == "rotation") {
    //         Quaternion *rotations = (Quaternion *)malloc(sizeof(Quaternion) * frameCount);
    //         BufferData(doc, outputAccessor, rotations);
    //         pose.rotation = rotations[index];
    //         free(rotations);
    //     } else if (channel.target.path == "scale") {
    //         Vector3 *scales = (Vector3 *)malloc(sizeof(Vector3) * frameCount);
    //         BufferData(doc, outputAccessor, scales);
    //         pose.scale = scales[index];
    //         free(scales);
    //     }
    // }



    // for (const auto &channel : clip.channels) {
    //     size_t joint = channel.target.node;
    //     if (joint < 0 || joint >= model.boneCount) {
    //         std::cout << "OOB joint (" << joint << "), ignoring" << std::endl;
    //         continue;
    //     }

    //     const auto &sampler = clip.samplers[channel.sampler];
    //     const auto &inputAccessor = doc.accessors[sampler.input];
    //     const auto &outputAccessor = doc.accessors[sampler.output];

    //     size_t keyCount = inputAccessor.count;
    //     std::vector<float> times(keyCount);
    //     BufferData(doc, inputAccessor, times.data());
        
    //     size_t prevIndex = 0;
    //     for (; prevIndex < keyCount - 1; ++prevIndex) {
    //         if (time < times[prevIndex + 1]) break;
    //     }

    //     float t0 = times[prevIndex];
    //     float t1 = times[std::min(prevIndex + 1, keyCount - 1)];
    //     float localTime = (time - t0) / (t1 - t0);

    //     if (channel.target.path == "rotation") {
    //         std::vector<Quaternion> quats(keyCount);
    //         BufferData(doc, outputAccessor, quats.data());
    //         Quaternion q0 = quats[prevIndex];
    //         Quaternion q1 = quats[std::min(prevIndex + 1, keyCount - 1)];
    //         Quaternion q = QuaternionSlerp(q0, q1, localTime);
    //         model.bindPose[joint].rotation = q;
    //     } else if (channel.target.path == "translation") {
    //         std::vector<Vector3> vecs(keyCount);
    //         BufferData(doc, outputAccessor, vecs.data());
    //         Vector3 v0 = vecs[prevIndex];
    //         Vector3 v1 = vecs[std::min(prevIndex + 1, keyCount - 1)];
    //         model.bindPose[joint].translation = Vector3Lerp(v0, v1, localTime);
    //     } else if (channel.target.path == "scale") {
    //         std::vector<Vector3> vecs(keyCount);
    //         BufferData(doc, outputAccessor, vecs.data());
    //         Vector3 s0 = vecs[prevIndex];
    //         Vector3 s1 = vecs[std::min(prevIndex + 1, keyCount - 1)];
    //         model.bindPose[joint].scale = Vector3Lerp(s0, s1, localTime);
    //     }
    // }
}
