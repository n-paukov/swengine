#include "precompiled.h"

#pragma hdrstop

#include "LevelsManager.h"

#include <utility>

#include "Modules/Graphics/GraphicsSystem/GraphicsSceneManagementSystem.h"
#include "Modules/Math/MathUtils.h"
#include "Utility/files.h"

LevelsManager::LevelsManager(const std::shared_ptr<GameWorld>& gameWorld,
  const std::shared_ptr<ResourcesManager>& resourceManager)
  : m_gameWorld(gameWorld),
    m_resourceManager(resourceManager),
    m_gameObjectsLoader(gameWorld, resourceManager)
{

}

LevelsManager::~LevelsManager()
{
  unloadLevel();
}

void LevelsManager::unloadLevel()
{
  if (m_isLevelLoaded) {
    m_gameWorld->emitEvent<UnloadSceneCommandEvent>(UnloadSceneCommandEvent{});

    for (GameObject object : m_gameWorld->all()) {
      m_gameWorld->removeGameObject(object);
    }

    m_isLevelLoaded = false;
  }

  m_gameObjectsLoader.resetLoadedObjects();
}

void LevelsManager::loadLevelStaticObjects(
  const std::string& levelName,
  std::vector<std::string>& objectsIds)
{
  spdlog::info("Load level static objects: {}", levelName);

  auto levelDescriptionDocument = openLevelDescriptionFile(levelName,
    "level_static",
    "objects");

  pugi::xml_node levelDescription = levelDescriptionDocument->child("objects");

  for (pugi::xml_node& objectNode : levelDescription.children("object")) {
    auto transformNode = objectNode.child("transform");

    if (transformNode) {
      if (transformNode.attribute("static")) {
        THROW_EXCEPTION(EngineRuntimeException, "Level static objects shouldn't use static attribute");
      }

      transformNode.append_attribute("static") = "true";
    }
  }

  for (const pugi::xml_node& objectNode : levelDescription.children("object")) {
    std::string gameObjectSpawnName = m_gameObjectsLoader.loadGameObject(objectNode);

    objectsIds.push_back(gameObjectSpawnName);
  }
}

void LevelsManager::loadLevelDynamicObjects(
  const std::string& levelName,
  std::vector<std::string>& objects)
{
  // TODO: remove level-spawn-list conception, use casual spawn lists instead
  spdlog::info("Load level dynamic objects: {}", levelName);

  auto levelDescriptionDocument = openLevelDescriptionFile(levelName,
    "level_spawn",
    "objects");

  pugi::xml_node levelDescription = levelDescriptionDocument->child("objects");

  for (pugi::xml_node& objectNode : levelDescription.children("object")) {
    auto transformNode = objectNode.child("transform");

    if (transformNode) {
      if (transformNode.attribute("static")) {
        THROW_EXCEPTION(EngineRuntimeException, "Level dynamic objects shouldn't use static attribute");
      }

      transformNode.append_attribute("static") = "false";
    }
  }

  for (const pugi::xml_node& objectNode : levelDescription.children("object")) {
    std::string gameObjectSpawnName = m_gameObjectsLoader.loadGameObject(objectNode);

    objects.push_back(gameObjectSpawnName);
  }
}

void LevelsManager::loadLevel(const std::string& name, LevelLoadingMode loadingMode)
{
  spdlog::info("Load level {}", name);

  std::vector<std::string> levelStaticObjects;
  loadLevelStaticObjects(name, levelStaticObjects);

  std::vector<std::string> sceneObjectsNames = levelStaticObjects;

  if (loadingMode == LevelLoadingMode::AllData) {
    std::vector<std::string> levelDynamicObjects;

    loadLevelDynamicObjects(name, levelDynamicObjects);

    // dynamic objects should be spawned from game scripts
    //sceneObjectsNames.insert(sceneObjectsNames.end(), levelDynamicObjects.begin(), levelDynamicObjects.end());
  }

  std::vector<GameObject> sceneObjects;

  for (const std::string& objectSpawnName : sceneObjectsNames) {
    GameObject gameObject = m_gameObjectsLoader.buildGameObject(objectSpawnName);

    sceneObjects.push_back(gameObject);

    if (gameObject.hasComponent<TransformComponent>()) {
      gameObject.getComponent<TransformComponent>()->setLevelId(name);
    }
  }

  m_gameWorld->emitEvent<LoadSceneCommandEvent>(LoadSceneCommandEvent{.sceneObjects=sceneObjects});

  m_isLevelLoaded = true;

  spdlog::info("Level {} is loaded", name);
}

GameObjectsLoader& LevelsManager::getObjectsLoader()
{
  return m_gameObjectsLoader;
}

std::shared_ptr<pugi::xml_document> LevelsManager::openLevelDescriptionFile(const std::string& levelName,
  const std::string& descriptionFile,
  const std::string& descriptionNodeName)
{
  std::string levelPath = FileUtils::getLevelPath(levelName);

  if (!FileUtils::isDirExists(levelPath)) {
    THROW_EXCEPTION(EngineRuntimeException, "Level does not exists: " + levelPath);
  }

  std::string levelDescPath = levelPath + "/" + descriptionFile + ".xml";

  auto levelDescription = std::make_shared<pugi::xml_document>();
  levelDescription->reset(std::get<0>(XMLUtils::openDescriptionFile(levelDescPath, descriptionNodeName)));

  return levelDescription;
}

std::shared_ptr<GameWorld> LevelsManager::getGameWorld() const
{
  return m_gameWorld;
}

void LevelsManager::loadSpawnObjectsList(const std::string& spawnListName)
{
  spdlog::info("Load spawn objects list: {}", spawnListName);

  std::string spawnListPath = FileUtils::getSpawnListPath(spawnListName);
  auto levelDescriptionDocument = std::get<0>(XMLUtils::openDescriptionFile(spawnListPath, "objects"));

  pugi::xml_node spawnListDescription = levelDescriptionDocument.child("objects");

  for (pugi::xml_node& objectNode : spawnListDescription.children("object")) {
    auto transformNode = objectNode.child("transform");

    if (transformNode) {
      if (transformNode.attribute("static")) {
        THROW_EXCEPTION(EngineRuntimeException, "Spawn objects shouldn't use static attribute");
      }

      transformNode.append_attribute("static") = "false";
    }
  }

  for (const pugi::xml_node& objectNode : spawnListDescription.children("object")) {
    std::string gameObjectSpawnName = m_gameObjectsLoader.loadGameObject(objectNode);
    LOCAL_VALUE_UNUSED(gameObjectSpawnName);
  }
}

bool LevelsManager::isLevelLoaded() const
{
  return m_isLevelLoaded;
}
