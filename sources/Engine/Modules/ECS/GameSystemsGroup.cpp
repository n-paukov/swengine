#include "GameSystemsGroup.h"

#include <algorithm>

#include "GameSystem.h"
#include "GameWorld.h"

GameSystemsGroup::GameSystemsGroup(std::shared_ptr<GameWorld> gameWorld)
    : m_gameWorld(gameWorld)
{

}

GameSystemsGroup::~GameSystemsGroup()
{
    SW_ASSERT(!m_isConfigured);
}

void GameSystemsGroup::configure(GameWorld* gameWorld)
{
    SW_ASSERT(!m_isConfigured);

    m_isConfigured = true;

    for (auto& system : m_gameSystems)
        system->configure(gameWorld);
}

void GameSystemsGroup::unconfigure(GameWorld* gameWorld)
{
    SW_ASSERT(m_isConfigured);

    for (auto& system : m_gameSystems)
        system->unconfigure(gameWorld);

    m_isConfigured = false;
}

void GameSystemsGroup::render(GameWorld* gameWorld)
{
    for (auto& system : m_gameSystems)
        system->render(gameWorld);
}

void GameSystemsGroup::update(GameWorld* gameWorld, float delta)
{
    for (auto& system : m_gameSystems)
        system->update(gameWorld, delta);
}

void GameSystemsGroup::addGameSystem(std::shared_ptr<GameSystem> system)
{
    m_gameSystems.push_back(system);

    if (m_isConfigured) {
        system->configure(m_gameWorld.lock().get());
    }
}

void GameSystemsGroup::removeGameSystem(std::shared_ptr<GameSystem> system)
{
    if (m_isConfigured) {
        system->unconfigure(m_gameWorld.lock().get());
    }

    m_gameSystems.erase(std::remove(m_gameSystems.begin(), m_gameSystems.end(), system), m_gameSystems.end());
}
