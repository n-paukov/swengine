#pragma once

#include <Engine/Modules/ECS/ECS.h>
#include <Engine/Modules/Application/GameConsole.h>
#include <Engine/Modules/Graphics/GraphicsSystem/CameraComponent.h>
#include <Engine/Modules/Graphics/GraphicsSystem/SharedGraphicsState.h>
#include <Engine/Modules/Input/InputModule.h>

#include "PlayerComponent.h"

class PlayerControlSystem : public GameSystem,
                            public EventsListener<MouseWheelEvent>,
                            public EventsListener<InputActionToggleEvent> {
 public:
  explicit PlayerControlSystem(std::shared_ptr<InputModule> inputModule,
    std::shared_ptr<SharedGraphicsState> sharedGraphicsState);
  ~PlayerControlSystem() override = default;

  void configure() override;
  void unconfigure() override;

  void activate() override;
  void deactivate() override;

  void fixedUpdate(float delta) override;
  void update(float delta) override;

  void render() override;

  EventProcessStatus receiveEvent(GameWorld* gameWorld, const MouseWheelEvent& event) override;
  EventProcessStatus receiveEvent(GameWorld* gameWorld, const InputActionToggleEvent& event) override;

 private:
  [[nodiscard]] Camera& getPlayerCamera();

 private:
  GameObject m_playerObject;

  int16_t m_walkAnimationStateId = -1;
  int16_t m_idleAnimationStateId = -1;

  std::shared_ptr<InputModule> m_inputModule;
  std::shared_ptr<SharedGraphicsState> m_sharedGraphicsState;
};

