#include "main.hpp"
#include "utils.hpp"
#include "mesh.hpp"
#include "animation.hpp"
#include <raylib.h>
#include <iostream>
#include <string>


int main() {
    const std::string path = "./assets/model.vrm";
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "VRM renderer");

    VRMModel modelvrm = LoadVRM(path);
    Model model = ModelFromVRM(modelvrm);
    VRMAnimation anim = LoadAnimationClip("./assets/taunt.glb");
    std::cout << modelvrm.vrm00.humanoid.humanBones.size() << std::endl;

    Camera camera = {0};
    camera.position = {4.0f, 4.0f, 4.0f};
    camera.target = {0.0f, 1.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float animTime = 0.0f;
    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        animTime += delta;
        if (animTime > anim.duration) animTime = 0.0f;
        ApplyAnimationToModel(model, anim, anim.doc, animTime);

        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawModel(model, Vector3{0, 0, 0}, 1.0, WHITE);
        DrawGrid(10, 1.0f);
        EndMode3D();

        EndDrawing();
    }

    UnloadModel(model);
    CloseWindow();
}