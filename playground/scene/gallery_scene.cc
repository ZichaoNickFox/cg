#include "playground/scene/gallery_scene.h"

#include "playground/scene.pb.h"
#include "imgui.h"

void GalleryScene::OnEnter(Context* context) {

}

void GalleryScene::OnUpdate(Context* context) {

}

void GalleryScene::OnGui(Context* context) {
  bool open = true;
  ImGui::Begin("Scene", &open, ImGuiWindowFlags_AlwaysAutoResize);
  for (int scene_type = SceneType_MIN + 1; scene_type < SceneType_ARRAYSIZE; ++scene_type) {
    if (ImGui::Button(SceneType_Name(scene_type).c_str())) {
      context->SetNextSceneType(static_cast<SceneType>(scene_type));
      ImGui::NewLine();
    }
  }
  ImGui::End();
}

void GalleryScene::OnRender(Context* context) {
}

void GalleryScene::OnExit(Context* context) {
}