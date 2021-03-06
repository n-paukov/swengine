#pragma once

#include <Engine/Modules/ECS/ECS.h>
#include <Engine/Utility/xml.h>

#include "InfoportionsSystem.h"

enum class GameLogicCommunicatorRole {
  Actor,
  NPCActor
};

enum class GameLogicCommunicationDirection {
  ToNPCActor,
  ToActor
};

class GameLogicConditionsManager;

class GameLogicCondition {
 public:
  explicit GameLogicCondition(GameLogicConditionsManager* conditionsManager)
    : m_conditionsManager(conditionsManager)
  {

  }

  virtual ~GameLogicCondition() = default;

  [[nodiscard]] virtual bool calculateValue() = 0;

  [[nodiscard]] inline GameLogicConditionsManager* getConditionsManager() const
  {
    return m_conditionsManager;
  }

 private:
  GameLogicConditionsManager* m_conditionsManager;
};

class GameLogicActorCondition : public GameLogicCondition {
 public:

  explicit GameLogicActorCondition(GameLogicConditionsManager* conditionsManager)
    : GameLogicCondition(conditionsManager)
  {

  }

  void setActor(const GameObject& actor);
  [[nodiscard]] GameObject getActor() const;

  void setRole(GameLogicCommunicatorRole role);
  [[nodiscard]] GameLogicCommunicatorRole getRole() const;

 private:
  GameObject m_actor{};
  GameLogicCommunicatorRole m_role{};
};

class GameLogicConditionHasObject : public GameLogicActorCondition {
 public:
  explicit GameLogicConditionHasObject(
    GameLogicConditionsManager* conditionsManager,
    std::string objectId);
  ~GameLogicConditionHasObject() override = default;

  [[nodiscard]] bool calculateValue() override;

 private:
  std::string m_objectId;
};

class GameLogicConditionHasNotObject : public GameLogicActorCondition {
 public:
  explicit GameLogicConditionHasNotObject(
    GameLogicConditionsManager* conditionsManager,
    std::string objectId);
  ~GameLogicConditionHasNotObject() override = default;

  [[nodiscard]] bool calculateValue() override;

 private:
  std::string m_objectId;
};

class GameLogicConditionHasInfoportion : public GameLogicActorCondition {
 public:
  explicit GameLogicConditionHasInfoportion(
    GameLogicConditionsManager* conditionsManager,
    std::string infoportionName);
  ~GameLogicConditionHasInfoportion() override = default;

  [[nodiscard]] bool calculateValue() override;

 private:
  std::string m_infoportionName;
};

class GameLogicConditionBooleanUnary : public GameLogicCondition {
 public:
  explicit GameLogicConditionBooleanUnary(
    GameLogicConditionsManager* conditionsManager,
    std::unique_ptr<GameLogicCondition> condition)
    : GameLogicCondition(conditionsManager),
      m_condition(std::move(condition))
  {

  }

  [[nodiscard]] GameLogicCondition* getCondition();

 private:
  std::unique_ptr<GameLogicCondition> m_condition;
};

class GameLogicConditionBooleanBinary : public GameLogicCondition {
 public:
  explicit GameLogicConditionBooleanBinary(
    GameLogicConditionsManager* conditionsManager,
    std::vector<std::unique_ptr<GameLogicCondition>> conditions)
    : GameLogicCondition(conditionsManager),
      m_conditions(std::move(conditions))
  {

  }

  [[nodiscard]] std::vector<std::unique_ptr<GameLogicCondition>>& getConditions();

 private:
  std::vector<std::unique_ptr<GameLogicCondition>> m_conditions;
};

class GameLogicConditionAll : public GameLogicConditionBooleanBinary {
 public:
  explicit GameLogicConditionAll(
    GameLogicConditionsManager* conditionsManager,
    std::vector<std::unique_ptr<GameLogicCondition>> conditions);

  ~GameLogicConditionAll() override = default;

  [[nodiscard]] bool calculateValue() override;
};

class GameLogicConditionAny : public GameLogicConditionBooleanBinary {
 public:
  explicit GameLogicConditionAny(
    GameLogicConditionsManager* conditionsManager,
    std::vector<std::unique_ptr<GameLogicCondition>> conditions);

  ~GameLogicConditionAny() override = default;

  [[nodiscard]] bool calculateValue() override;
};

class GameLogicConditionNot : public GameLogicConditionBooleanUnary {
 public:
  explicit GameLogicConditionNot(
    GameLogicConditionsManager* conditionsManager,
    std::unique_ptr<GameLogicCondition> condition);

  ~GameLogicConditionNot() override = default;

  [[nodiscard]] bool calculateValue() override;

 private:
  std::unique_ptr<GameLogicCondition> m_condition;
};

class GameLogicConditionHasNotInfoportion : public GameLogicActorCondition {
 public:
  explicit GameLogicConditionHasNotInfoportion(
    GameLogicConditionsManager* conditionsManager,
    std::string infoportionName);
  ~GameLogicConditionHasNotInfoportion() override = default;

  [[nodiscard]] bool calculateValue() override;

 private:
  std::string m_infoportionName;
};

class GameLogicAction {
 public:
  explicit GameLogicAction(GameLogicConditionsManager* conditionsManager)
    : m_conditionsManager(conditionsManager)
  {

  }

  virtual ~GameLogicAction() = default;

  virtual void execute() = 0;

  [[nodiscard]] inline GameLogicConditionsManager* getConditionsManager() const
  {
    return m_conditionsManager;
  }

 private:
  GameLogicConditionsManager* m_conditionsManager;
};

class GameLogicActionSpawnObject : public GameLogicAction {
 public:
  explicit GameLogicActionSpawnObject(
    GameLogicConditionsManager* conditionsManager,
    std::string objectSpawnName,
    const glm::vec3& position,
    const glm::vec3& direction);
  ~GameLogicActionSpawnObject() override = default;

  void execute() override;

 private:
  std::string m_objectSpawnName;
  glm::vec3 m_position;
  glm::vec3 m_direction;
};

class GameLogicActorAction : public GameLogicAction {
 public:
  explicit GameLogicActorAction(
    GameLogicConditionsManager* conditionsManager)
    : GameLogicAction(conditionsManager)
  {

  }

  void setActor(const GameObject& actor);
  [[nodiscard]] GameObject getActor() const;

  void setRole(GameLogicCommunicatorRole role);
  [[nodiscard]] GameLogicCommunicatorRole getRole() const;

 private:
  GameObject m_actor{};
  GameLogicCommunicatorRole m_role{};
};

class GameLogicActionAddInfoportion : public GameLogicActorAction {
 public:
  explicit GameLogicActionAddInfoportion(
    GameLogicConditionsManager* conditionsManager,
    std::string infoportionName);
  ~GameLogicActionAddInfoportion() override = default;

  void execute() override;

 private:
  std::string m_infoportionName;
};

class GameLogicActionRemoveInfoportion : public GameLogicActorAction {
 public:
  explicit GameLogicActionRemoveInfoportion(
    GameLogicConditionsManager* conditionsManager,
    std::string infoportionName);
  ~GameLogicActionRemoveInfoportion() override = default;

  void execute() override;

 private:
  std::string m_infoportionName;
};

class GameLogicActionDirected : public GameLogicAction {
 public:
  explicit GameLogicActionDirected(
    GameLogicConditionsManager* conditionsManager);
  ~GameLogicActionDirected() override = default;

  void setInitiator(GameObject initiator);
  [[nodiscard]] GameObject getInitiator() const;

  void setTarget(GameObject target);
  [[nodiscard]] GameObject getTarget() const;

  void setDirection(GameLogicCommunicationDirection direction);
  [[nodiscard]] GameLogicCommunicationDirection getDirection() const;

 private:
  GameObject m_initiator;
  GameObject m_target;
  GameLogicCommunicationDirection m_direction{};
};

class GameLogicActionTransferItem : public GameLogicActionDirected {
 public:
  explicit GameLogicActionTransferItem(
    GameLogicConditionsManager* conditionsManager,
    std::string itemName);
  ~GameLogicActionTransferItem() override = default;

  void setItemName(const std::string& name);
  [[nodiscard]] const std::string& getItemName() const;

  void execute() override;

 private:
  std::string m_itemName;
};

class GameLogicActionStopDialogue : public GameLogicActorAction {
 public:
  explicit GameLogicActionStopDialogue(
    GameLogicConditionsManager* conditionsManager);
  ~GameLogicActionStopDialogue() override = default;

  void execute() override;

 private:
  std::string m_infoportionName;
};


using GameLogicActionsList = std::vector<std::shared_ptr<GameLogicAction>>;

class GameLogicConditionsManager {
 public:
  explicit GameLogicConditionsManager(
    std::shared_ptr<GameWorld> gameWorld);

  [[nodiscard]] GameObject getPlayer() const;

  std::shared_ptr<GameLogicCondition> buildConditionsTree(pugi::xml_node conditionsNode);
  void traverseConditionsTree(GameLogicCondition* conditionNode,
    const std::function<void(GameLogicCondition*)>& visitor);

  GameLogicActionsList buildActionsList(pugi::xml_node actionsNode);

  [[nodiscard]] GameWorld& getGameWorld();

  void setupConditionCommunicators(GameLogicCondition* condition, GameObject actor, GameObject npc);
  void setupActionsCommunicators(const GameLogicActionsList& actionsList, GameObject actor, GameObject npc);

 private:
  GameLogicCondition* parseConditionsNode(pugi::xml_node conditionsNode);
  GameLogicCondition* parseConditionsNodeAll(pugi::xml_node conditionsNode);

  GameLogicCommunicatorRole getCommunicatorRoleByName(const std::string& roleName);
  GameLogicCommunicationDirection getCommunicationDirectionByName(const std::string& directionType);
 private:
  std::shared_ptr<GameWorld> m_gameWorld;
};
