#pragma once

#include <vector>
#include <memory>

#include "GameSystem.h"

class GameWorld;

class GameSystemsGroup : public GameSystem {
 public:
  explicit GameSystemsGroup(std::shared_ptr<GameWorld> gameWorld);
  ~GameSystemsGroup() override;

  void configure(GameWorld* gameWorld) override;
  void unconfigure(GameWorld* gameWorld) override;

  void beforeRender(GameWorld* gameWorld) override;
  void render(GameWorld* gameWorld) override;
  void afterRender(GameWorld* gameWorld) override;

  void fixedUpdate(GameWorld* gameWorld, float delta) override;
  void update(GameWorld* gameWorld, float delta) override;

  virtual void addGameSystem(std::shared_ptr<GameSystem> system);
  virtual void removeGameSystem(std::shared_ptr<GameSystem> system);

  template<class T>
  std::shared_ptr<T> getGameSystem() const;

 protected:
  const std::vector<std::shared_ptr<GameSystem>>& getGameSystems() const;

 private:
  std::vector<std::shared_ptr<GameSystem>> m_gameSystems;
  std::weak_ptr<GameWorld> m_gameWorld;

  bool m_isConfigured = false;
};

template<class T>
std::shared_ptr<T> GameSystemsGroup::getGameSystem() const
{
  static_assert(std::is_base_of_v<GameSystem, T>);

  for (std::shared_ptr<GameSystem> gameSystem : m_gameSystems) {
    std::shared_ptr<T> foundGameSystem = std::dynamic_pointer_cast<T>(gameSystem);

    if (foundGameSystem != nullptr) {
      return foundGameSystem;
    }
  }

  return nullptr;
}

