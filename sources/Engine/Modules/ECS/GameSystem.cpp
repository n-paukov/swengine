#include "precompiled.h"

#pragma hdrstop

#include "GameSystem.h"

GameSystem::GameSystem() = default;

GameSystem::~GameSystem() = default;

void GameSystem::update(GameWorld* gameWorld, float delta)
{
  ARG_UNUSED(gameWorld);
  ARG_UNUSED(delta);
}

void GameSystem::render(GameWorld* gameWorld)
{
  ARG_UNUSED(gameWorld);
}

void GameSystem::beforeRender(GameWorld* gameWorld)
{
  ARG_UNUSED(gameWorld);
}

void GameSystem::afterRender(GameWorld* gameWorld)
{
  ARG_UNUSED(gameWorld);
}

void GameSystem::configure(GameWorld* gameWorld)
{
  ARG_UNUSED(gameWorld);
}

void GameSystem::unconfigure(GameWorld* gameWorld)
{
  ARG_UNUSED(gameWorld);
}

void GameSystem::fixedUpdate(GameWorld* gameWorld, float delta)
{
  ARG_UNUSED(gameWorld);
  ARG_UNUSED(delta);
}
