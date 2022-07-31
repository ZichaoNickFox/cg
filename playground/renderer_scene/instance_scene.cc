#include "playground/renderer_scene/instance_scene.h"

#include "renderer/shader.h"
#include "renderer/transform.h"
#include "renderer/math.h"
#include "playground/renderer_scene/common.h"

constexpr int instance_num = 10000;

void InstanceScene::OnEnter(Scene* context) {
  rock_.Init(context, "rock", "rock");
  planet_.Init(context, "planet", "planet");

  InitModelMatrices();

  rock_.AddVertexAttribute({"model", renderer::kVertexLayoutIndex5, renderer::kVertexLayoutIndex8, 4, 1}, models_);
    
  context->SetCamera(camera_.get());
  camera_->mutable_transform()->SetTranslation(glm::vec3(0, 0, 15));
}

void InstanceScene::OnUpdate() {
  OnUpdateCommon _(scene, "InstanceScene");
}

void InstanceScene::OnRender() {
  InstanceSceneShader(scene, &rock_);
  rock_.OnRender(scene, models_.size());

  SimpleModelShader(scene, &planet_);
  planet_.OnRender(scene);

  OnRenderCommon _(scene);
}

void InstanceScene::OnExit(Scene* context) {
}

void InstanceScene::InitModelMatrices() {
  models_.resize(instance_num);
  float radius = 8.0;
  float offset = 2.5f;
  for(uint64_t i = 0; i < instance_num; i++) {
    glm::mat4 model(1);
    // 1. λ�ƣ��ֲ��ڰ뾶Ϊ 'radius' ��Բ���ϣ�ƫ�Ƶķ�Χ�� [-offset, offset]
    float angle = (float)i / (float)instance_num * 360.0f;
    float displacement = util::RandFromTo(0, (int)(2 * offset * 100)) / 100.0f;
    float x = sin(angle) * radius + displacement;
    displacement = util::RandFromTo(0, (int)(2 * offset * 100)) / 100.0f;
    float y = displacement * 0.4f; // �����Ǵ��ĸ߶ȱ�x��z�Ŀ���ҪС
    displacement = util::RandFromTo(0, (int)(2 * offset * 100)) / 100.0f;
    float z = cos(angle) * radius + displacement;
    model = glm::translate(model, glm::vec3(x, y, z));

    // 2. ���ţ��� 0.05 �� 0.25f ֮������
    float scale = util::RandFromTo(0, 20) / 1000.0f + 0.05;
    model = glm::scale(model, glm::vec3(scale, scale, scale));

    // 3. ��ת������һ�����룩���ѡ�����ת�����������������ת
    float rotAngle = util::RandFromTo(0, 360);
    model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

    models_[i] = model;
  }
}
