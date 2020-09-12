#pragma once

class GameWorld;

class GameSystemsGroup;

/*!
 * \brief Class for representing a game system
 *
 * This class allows to represent a game system with user-specified functionality
 */
class GameSystem {
 public:
  GameSystem();
  virtual ~GameSystem();

  /*!
* \brief Performs the game system update with fixed internal step
*
* \param gameWorld the game world pointer
* \param delta delta time
*/
  virtual void fixedUpdate(GameWorld* gameWorld, float delta);

  /*!
   * \brief Performs the game system update
   *
   * \param gameWorld the game world pointer
   * \param delta delta time
   */
  virtual void update(GameWorld* gameWorld, float delta);

  /*!
   * \brief Renders the game system data
   */
  virtual void render(GameWorld* gameWorld);

  /*!
   * \brief It is called before rendering of the game world
   */
  virtual void beforeRender(GameWorld* gameWorld);

  /*!
   * \brief It is called after rendering of the game world
   */
  virtual void afterRender(GameWorld* gameWorld);

  /*!
   * \brief Calls at the time of the game system registration
   *
   * \param gameWorld the game world pointer
   */
  virtual void configure(GameWorld* gameWorld);

  /*!
   * \brief Calls at the time of the game system removal
   *
   * \param gameWorld the game world pointer
   */
  virtual void unconfigure(GameWorld* gameWorld);

  /*!
   * \brief Sets the game system active status
   *
   * \param isActive the active status flag
   */
  void setActive(bool isActive);

  /*!
   * @brief Checks whether the game system is active
   *
   * @return active status flag
   */
  [[nodiscard]] bool isActive() const;

  /*!
   * \brief Calls at the time of the game system registration
   */
  virtual void activate();

  /*!
   * \brief Calls at the time of the game system removal
   */
  virtual void deactivate();

  /*!
   * @brief Returns GameWorld reference
   * @return GameWorld reference
   */
  GameWorld& getGameWorld() const;

 private:
  bool m_isActive = false;

  std::weak_ptr<GameWorld> m_gameWorld;

 private:
  friend class GameSystemsGroup;
};
