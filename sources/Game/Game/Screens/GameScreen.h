#pragma once

#include <Engine/Modules/Graphics/GUI/GUIConsole.h>
#include <Engine/Modules/LevelsManagement/LevelsManager.h>

#include "BaseGameScreen.h"
#include "Game/PlayerControlSystem.h"
#include "Game/Game.h"

#include "Game/Inventory/InventoryUI.h"

class GameScreen : public BaseGameScreen,
  public EventsListener<InputActionToggleEvent>,
  public EventsListener<GameConsoleChangeVisibilityEvent> {
 public:
  GameScreen(
    std::shared_ptr<InputModule> inputModule,
    std::shared_ptr<GameSystemsGroup> gameApplicationSystemsGroup,
    std::shared_ptr<LevelsManager> levelsManager,
    std::shared_ptr<GraphicsScene> graphicsScene,
    std::shared_ptr<GUILayout> debugGUILayout,
    std::shared_ptr<InventoryUI> inventoryUILayout);
  ~GameScreen() override;

  void activate() override;
  void deactivate() override;

  void load() override;
  void unload() override;

  void update(float delta) override;
  void render() override;

  EventProcessStatus receiveEvent(GameWorld* gameWorld, const InputActionToggleEvent& event) override;
  EventProcessStatus receiveEvent(GameWorld* gameWorld, const GameConsoleChangeVisibilityEvent& event) override;

 private:
  void initializeGame();
  void deinitializeGame();

  void initializeDebugGUI();
  void deinitializeDebugGUI();

 private:
  std::shared_ptr<InputModule> m_inputModule;
  std::shared_ptr<GameSystemsGroup> m_gameApplicationSystemsGroup;
  std::shared_ptr<LevelsManager> m_levelsManager;
  std::shared_ptr<GraphicsScene> m_graphicsScene;

  std::unique_ptr<Game> m_game;

  std::shared_ptr<GUILayout> m_gameGUILayout;
  std::shared_ptr<GUILayout> m_debugGUILayout;
  std::shared_ptr<InventoryUI> m_inventoryUILayout;

  std::shared_ptr<GUIText> m_primivitesCountText;
  std::shared_ptr<GUIText> m_subMeshesCountText;
  std::shared_ptr<GUIText> m_culledSubMeshesCountText;
};

