#pragma once

#include <Engine/Modules/ECS/ECS.h>
#include <Engine/Modules/Graphics/GraphicsSystem/CameraComponent.h>
#include <Engine/Modules/Input/InputModule.h>

#include "PlayerComponent.h"

class PlayerControlSystem : public GameSystem,
                            public EventsListener<MouseWheelEvent> {
 public:
  explicit PlayerControlSystem(std::shared_ptr<InputModule> inputModule);
  ~PlayerControlSystem() override;

  void configure(GameWorld* gameWorld) override;
  void unconfigure(GameWorld* gameWorld) override;

  void update(GameWorld* gameWorld, float delta) override;
  void render(GameWorld* gameWorld) override;

  EventProcessStatus receiveEvent(GameWorld* gameWorld, const MouseWheelEvent& event) override;

 private:
  [[nodiscard]] Camera* getPlayerCamera() const;
  [[nodiscard]] Transform* getPlayerTransform() const;

  void updateViewParameters(const glm::vec2& mouseDelta, float delta);
  void updatePlayerAndCameraPosition(float delta);

 private:
  GameObject* m_playerObject = nullptr;
  int16_t m_runningAnimationStateId = -1;
  int16_t m_idleAnimationStateId = -1;

  std::shared_ptr<InputModule> m_inputModule;

};

