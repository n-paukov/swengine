#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <memory>
#include <utility>

#include "GameSystemsGroup.h"
#include "GameObject.h"
#include "GameObjectsStorage.h"

#include "GameObjectsSequentialView.h"
#include "GameObjectsComponentsView.h"

#include "EventsListener.h"

// TODO: replace raw EventListener pointers with shared pointers to avoid memory leaks or usage of invalid pointers

// TODO: add ability to subscribe to events not only objects but also callbacks, probably replace
// polymorphic objects with std::function objects, but foresee deleting of them (for example, by unique identifies)

/*!
 * \brief Class for representing the game world
 * 
 * This class allows to store game object, their components
 * and systems. Also the class provide functions to find these things
 * and iterate over them.
 */
class GameWorld : public std::enable_shared_from_this<GameWorld> {
 public:
  GameWorld(const GameWorld& gameWorld) = delete;

  ~GameWorld() {
    m_gameSystemsGroup->unconfigure();
    m_gameObjectsStorage.reset();
  }

  /*!
 * \brief Performs the game world update with fixed internal step
 *
 * The function performs update of the game world and
 * calls systems update methods
 *
 * \param delta delta time
 */
  void fixedUpdate(float delta) {
    m_gameSystemsGroup->fixedUpdate(delta);
  }

  /*!
   * \brief Performs the game world update
   *
   * The function performs update of the game world and
   * calls systems update methods
   *
   * \param delta delta time
   */
  void update(float delta) {
    m_gameSystemsGroup->update(delta);
  }

  /*!
   * \brief Renders the game world
   */
  void render() {
    m_gameSystemsGroup->render();
  }

  /*!
   * \brief It is called before rendering of the game world
   */
  void beforeRender() {
    m_gameSystemsGroup->beforeRender();
  }

  /*!
   * \brief It is called after rendering of the game world
   */
  void afterRender() {
    m_gameSystemsGroup->afterRender();
  }

  /*!
   * \brief getGameSystemsGroup Returns main game systems group
   * \return the main game systems group
   */
  GameSystemsGroup* getGameSystemsGroup() const {
    return m_gameSystemsGroup.get();
  }

  /*!
   * \brief Creates and registers a new game object
   *
   * \return the object pointer
   */
  GameObject createGameObject() {
    GameObject gameObject = m_gameObjectsStorage->create();
    emitEvent(GameObjectAddEvent{gameObject});

    return gameObject;
  }

  /*!
 * \brief Creates and registers a new game object with specified unique name
 *
 * \return the object pointer
 */
  GameObject createGameObject(const std::string& name) {
    GameObject gameObject = m_gameObjectsStorage->createNamed(name);
    emitEvent(GameObjectAddEvent{gameObject});

    return gameObject;
  }

  /*!
   * \brief Finds the game object by ID
   *
   * \param id Identifier of the game object
   * \return the object pointer
   */
  GameObject findGameObject(GameObjectId id) const {
    return m_gameObjectsStorage->getById(id);
  }

  /*!
   * \brief Finds the game object by name
   *
   * \param id Name of the game object
   * \return the object pointer
   */
  GameObject findGameObject(const std::string& name) const {
    return m_gameObjectsStorage->getByName(name);
  }

  /*!
   * \brief Finds the game object by predicate
   *
   * \param predicate predicate for the object determination
   * \return the object pointer
   */
  GameObject findGameObject(const std::function<bool(const GameObject&)>& predicate) {
    for (const GameObjectData& object : m_gameObjectsStorage->getGameObjects()) {
      GameObject gameObject(object.id, object.revision, m_gameObjectsStorage.get());

      if (gameObject.isAlive() && predicate(gameObject)) {
        return gameObject;
      }
    }

    return GameObject();
  }

  /*!
   * \brief Removes the game objects
   *
   * \param gameObject removed game object
   */
  void removeGameObject(GameObject& gameObject) {
    m_gameObjectsStorage->remove(gameObject);
  }

  /*!
   * \brief Performs specified action for each existing game object
   *
   * \param action action to perform
   */
  void forEach(const std::function<void(GameObject&)>& action) {
    for (GameObject gameObject : this->all()) {
      action(gameObject);
    }
  }

  /*!
   * \brief Returns view for iterate over all game objects
   *
   * \return view for iterate over all game objects
   */
  GameObjectsSequentialView all() {
    GameObjectsSequentialIterator begin(m_gameObjectsStorage.get(), 0, false);
    GameObjectsSequentialIterator end(m_gameObjectsStorage.get(), m_gameObjectsStorage->getSize(), true);

    return GameObjectsSequentialView(begin, end);
  }

  /*!
   * \brief Returns view of game objects with specified components
   *
   * \return view of game objects with specified components
   */
  template<class ComponentType>
  GameObjectsComponentsView<ComponentType> allWith();

  /*!
   * \brief Subscribes the event listener for the specified event
   *
   * \param listener event listener object
   */
  template<class T>
  void subscribeEventsListener(EventsListener<T>* listener);

  /*!
   * Unsubscribes the event listener from the specified event
   *
   * \param listener event listener object
   */
  template<class T>
  void unsubscribeEventsListener(EventsListener<T>* listener);

  /*!
   * \brief Unsubscribes the event listener from all events
   *
   * \param listener events listener object
   */
  void cancelEventsListening(BaseEventsListener* listener) {
    for (auto& it : m_eventsListeners) {
      std::vector<BaseEventsListener*>& listeners = it.second;
      listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }
  }

  /*!
   * \brief Sends the event data to all appropriate listeners
   *
   * \param event event data
   */
  template<class T>
  EventProcessStatus emitEvent(const T& event);

 public:
  static std::shared_ptr<GameWorld> createInstance() {
    std::shared_ptr<GameWorld> gameWorld(new GameWorld());
    gameWorld->setGameSystemsGroup(std::make_unique<GameSystemsGroup>(gameWorld.get()));
    gameWorld->m_gameObjectsStorage = std::make_unique<GameObjectsStorage>(gameWorld.get());

    // Create internal ill-formed GameObject to mark the zero id as reserved
    RETURN_VALUE_UNUSED(gameWorld->createGameObject());

    return gameWorld;
  }

 private:
  GameWorld() = default;

  /*!
 * \brief setGameSystemsGroup Sets main game systems group
 * \param group group to set
 */
  void setGameSystemsGroup(std::unique_ptr<GameSystemsGroup> group) {
    m_gameSystemsGroup = std::move(group);
    m_gameSystemsGroup->configure();
    m_gameSystemsGroup->setActive(true);
  }

 private:
  std::unique_ptr<GameSystemsGroup> m_gameSystemsGroup;
  std::unique_ptr<GameObjectsStorage> m_gameObjectsStorage;

  std::unordered_map<std::type_index, std::vector<BaseEventsListener*>> m_eventsListeners;

 private:
  friend class GameObject;
};

template<class ComponentType>
inline GameObjectsComponentsView<ComponentType> GameWorld::allWith()
{
  GameObjectsComponentsIterator<ComponentType> begin(m_gameObjectsStorage.get(), 0, false);
  GameObjectsComponentsIterator<ComponentType> end(m_gameObjectsStorage.get(), m_gameObjectsStorage->getSize(), true);

  return GameObjectsComponentsView<ComponentType>(begin, end);
}

template<class T>
inline void GameWorld::subscribeEventsListener(EventsListener<T>* listener)
{
  auto typeId = std::type_index(typeid(T));

  auto eventListenersListIt = m_eventsListeners.find(typeId);

  if (eventListenersListIt == m_eventsListeners.end()) {
    m_eventsListeners.insert({typeId, std::vector<BaseEventsListener*>{listener}});
  }
  else {
    eventListenersListIt->second.push_back(listener);
  }
}

template<class T>
inline void GameWorld::unsubscribeEventsListener(EventsListener<T>* listener)
{
  auto typeId = std::type_index(typeid(T));

  auto eventListenersListIt = m_eventsListeners.find(typeId);

  if (eventListenersListIt == m_eventsListeners.end()) {
    return;
  }

  eventListenersListIt->second.erase(std::remove(eventListenersListIt->second.begin(),
    eventListenersListIt->second.end(), listener),
    eventListenersListIt->second.end());
}

template<class T>
inline EventProcessStatus GameWorld::emitEvent(const T& event)
{
  bool processed = false;
  auto typeId = std::type_index(typeid(T));

  auto eventListenersListIt = m_eventsListeners.find(typeId);

  if (eventListenersListIt != m_eventsListeners.end()) {
    for (BaseEventsListener* baseListener : eventListenersListIt->second) {
      auto* listener = reinterpret_cast<EventsListener<T>*>(baseListener);
      EventProcessStatus processStatus = listener->receiveEvent(this, event);

      if (processStatus == EventProcessStatus::Prevented) {
        return EventProcessStatus::Prevented;
      }
      else if (processStatus == EventProcessStatus::Processed) {
        processed = true;
      }
    }
  }

  return (processed) ? EventProcessStatus::Processed : EventProcessStatus::Skipped;
}
