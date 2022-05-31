#include "playground/scene/instance_scene.h"

#include "engine/transform.h"
#include "engine/math.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

constexpr int instance_num = 10000;

void InstanceScene::OnEnter(Context* context) {
  rock_.Init(context, "rock", "rock");
  planet_.Init(context, "planet", "planet");

  InitModelMatrices();

  rock_.AddVertexAttribute({"model", engine::kVertexLayoutIndex5, engine::kVertexLayoutIndex8, 4, 1}, models_);
    
  context->SetCamera(camera_.get());
  camera_->mutable_transform()->SetTranslation(glm::vec3(0, 0, 15));
}

void InstanceScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "InstanceScene");
}

void InstanceScene::OnRender(Context* context) {
  InstanceSceneShader(context, &rock_);
  rock_.OnRender(context, models_.size());

  SimpleModelShader(context, &planet_);
  planet_.OnRender(context);

  OnRenderCommon _(context);
}

void InstanceScene::OnExit(Context* context) {
}

void InstanceScene::InitModelMatrices() {
  models_.resize(instance_num);
  float radius = 8.0;
  float offset = 2.5f;
  for(uint64_t i = 0; i < instance_num; i++) {
    glm::mat4 model(1);
    // 1. λ�ƣ��ֲ��ڰ뾶Ϊ 'radius' ��Բ���ϣ�ƫ�Ƶķ�Χ�� [-offset, offset]
    float angle = (float)i / (float)instance_num * 360.0f;
    float displacement = engine::RandFromTo(0, (int)(2 * offset * 100)) / 100.0f;
    float x = sin(angle) * radius + displacement;
    displacement = engine::RandFromTo(0, (int)(2 * offset * 100)) / 100.0f;
    float y = displacement * 0.4f; // �����Ǵ��ĸ߶ȱ�x��z�Ŀ��ҪС
    displacement = engine::RandFromTo(0, (int)(2 * offset * 100)) / 100.0f;
    float z = cos(angle) * radius + displacement;
    model = glm::translate(model, glm::vec3(x, y, z));

    // 2. ���ţ��� 0.05 �� 0.25f ֮������
    float scale = engine::RandFromTo(0, 20) / 1000.0f + 0.05;
    model = glm::scale(model, glm::vec3(scale, scale, scale));

    // 3. ��ת������һ�����룩���ѡ�����ת�����������������ת
    float rotAngle = engine::RandFromTo(0, 360);
    model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

    models_[i] = model;
  }
}
