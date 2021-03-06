#pragma once

#include <memory>

#include "Transform.h"
#include "Modules/ECS/GameObjectsFactory.h"
#include "Modules/Math/geometry.h"
#include "Modules/Math/MathUtils.h"

struct TransformComponentBindingParameters {
  glm::vec3 position{};
  glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
  glm::vec3 frontDirection{};
  bool isStatic{};
  std::string levelId;
  bool isOnline{};

  template<class Archive>
  void serialize(Archive& archive)
  {
    archive(
      cereal::make_nvp("position", position),
      cereal::make_nvp("scale", scale),
      cereal::make_nvp("front_direction", frontDirection),
      cereal::make_nvp("is_static", isStatic),
      cereal::make_nvp("level_id", levelId),
      cereal::make_nvp("is_online", isOnline));
  };
};

class TransformComponent {
 public:
  static constexpr bool s_isSerializable = true;
  using BindingParameters = TransformComponentBindingParameters;

 public:
  TransformComponent();

  [[nodiscard]] Transform& getTransform();
  [[nodiscard]] const Transform& getTransform() const;

  [[nodiscard]] std::shared_ptr<Transform> getTransformPtr() const;

  // TODO: move static mode settings to constructor parameters
  //  and do not allow to change it after component creation
  void setStaticMode(bool isStatic);
  [[nodiscard]] bool isStatic() const;

  void updateBounds(const glm::mat4& transformation);
  void updateBounds(const glm::vec3& origin, const glm::quat& orientation);

  [[nodiscard]] const AABB& getBoundingBox() const;
  [[nodiscard]] const Sphere& getBoundingSphere() const;

  void setBounds(const AABB& bounds);
  [[nodiscard]] const AABB& getOriginalBounds() const;

  [[nodiscard]] BindingParameters getBindingParameters() const;

  void setLevelId(const std::string& levelId);
  [[nodiscard]] const std::string& getLevelId() const;

  void setOnlineMode(bool isOnline);
  [[nodiscard]] bool isOnline() const;

 private:
  std::shared_ptr<Transform> m_transform;

  bool m_isStatic = false;

  AABB m_boundingBox;
  Sphere m_boundingSphere;

  AABB m_originalBounds;

  std::string m_levelId;
  bool m_isOnline = false;
};

class TransformComponentBinder : public GameObjectsComponentBinder<TransformComponent> {
 public:
  explicit TransformComponentBinder(ComponentBindingParameters  componentParameters);

  void bindToObject(GameObject& gameObject) override;

 private:
  ComponentBindingParameters m_bindingParameters;
};