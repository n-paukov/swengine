#include "GameComponentsLoader.h"

#include <Engine/Modules/Graphics/Resources/TextureResourceManager.h>
#include <Engine/Exceptions/exceptions.h>

#include <utility>

#include "Game/PlayerComponent.h"
#include "Game/Inventory/InventoryComponent.h"
#include "Game/Dynamic/InteractiveObjectComponent.h"
#include "Game/Dynamic/ActorComponent.h"

GameComponentsLoader::GameComponentsLoader(
  std::shared_ptr<GameWorld> gameWorld,
  std::shared_ptr<ResourcesManager> resourceManager)
  : m_gameWorld(std::move(gameWorld)),
    m_resourceManager(std::move(std::move(resourceManager)))
{

}

void GameComponentsLoader::loadPlayerData(GameObject& gameObject,
  const pugi::xml_node& data)
{
  float height = data.attribute("height").as_float(1.0f);

  auto& playerComponent = *gameObject.addComponent<PlayerComponent>(height).get();

  float walk_speed = data.attribute("walk_speed").as_float(1.0f);
  playerComponent.setMovementSpeed(walk_speed);
}

void GameComponentsLoader::loadInventoryItemData(GameObject& gameObject, const pugi::xml_node& data)
{
  std::string itemId = data.attribute("name").as_string();
  std::string itemName = data.attribute("title").as_string();
  std::string iconName = data.attribute("icon").as_string();

  auto iconTexture = m_resourceManager->getResource<GLTexture>(iconName);

  auto& inventoryItemComponent = *gameObject.addComponent<InventoryItemComponent>(iconTexture, itemId, itemName).get();

  inventoryItemComponent.setReadable(data.attribute("readable").as_bool());
  inventoryItemComponent.setUsable(data.attribute("usable").as_bool());
  inventoryItemComponent.setDroppable(data.attribute("droppable").as_bool());

  inventoryItemComponent.setShortDescription(data.child_value("short_desc"));
  inventoryItemComponent.setLongDescription(data.child_value("long_desc"));
}

void GameComponentsLoader::loadInventoryData(GameObject& gameObject, const pugi::xml_node& data)
{
  gameObject.addComponent<InventoryComponent>().get();

  for (pugi::xml_node itemNode : data.child("items").children("item")) {
    std::string itemObjectName = itemNode.attribute("name").as_string();

    GameObject itemObject = m_gameWorld->findGameObject(itemObjectName);

    if (!itemObject.isAlive()) {
      THROW_EXCEPTION(EngineRuntimeException,
        fmt::format("Inventory item object {} is not alive at the loading time", itemObjectName));
    }

    m_gameWorld->emitEvent<InventoryItemActionCommandEvent>(
      {gameObject,
        InventoryItemActionTriggerType::RelocateToInventory,
        itemObject});
  }
}

void GameComponentsLoader::loadInteractiveData(GameObject& gameObject, const pugi::xml_node& data)
{
  auto& interactiveComponent = *gameObject.addComponent<InteractiveObjectComponent>().get();

  std::string objectName = data.attribute("name").as_string();
  interactiveComponent.setName(objectName);

  pugi::xml_node takeableConditions = data.child("takeable");

  if (takeableConditions) {
    interactiveComponent.setTakeable(true);
  }

  pugi::xml_node usableConditions = data.child("usable");

  if (usableConditions) {
    interactiveComponent.setUsable(true);
  }

  pugi::xml_node talkableConditions = data.child("talkable");

  if (talkableConditions) {
    interactiveComponent.setTalkable(true);
  }
}

void GameComponentsLoader::loadActorData(GameObject& gameObject, const pugi::xml_node& data)
{
  auto& actorComponent = *gameObject.addComponent<ActorComponent>().get();

  std::string actorName = data.attribute("name").as_string();
  actorComponent.setName(actorName);

  pugi::xml_node dialoguesNode = data.child("dialogues");

  for (pugi::xml_node dialogueNode : dialoguesNode.children("dialogue")) {
    std::string dialogueId = dialogueNode.attribute("id").as_string();
    bool isStartedByNPC = dialogueNode.attribute("npc_start").as_bool(false);

    actorComponent.addDialogue(ActorDialogue(dialogueId, isStartedByNPC));
  }

  pugi::xml_node healthNode = data.child("health");

  if (healthNode) {
    actorComponent.setHealth(healthNode.attribute("value").as_float(0.0f));
    actorComponent.setHealthLimit(healthNode.attribute("limit").as_float(100.0f));
  }
}
