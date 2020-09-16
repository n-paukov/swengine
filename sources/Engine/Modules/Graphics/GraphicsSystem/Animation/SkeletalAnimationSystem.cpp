#include "precompiled.h"

#pragma hdrstop

#include "SkeletalAnimationSystem.h"
#include "Bone.h"

SkeletalAnimationSystem::SkeletalAnimationSystem() = default;

SkeletalAnimationSystem::~SkeletalAnimationSystem() = default;

void SkeletalAnimationSystem::configure()
{
}

void SkeletalAnimationSystem::unconfigure()
{
}

void SkeletalAnimationSystem::update(float delta)
{
  for (GameObject obj : getGameWorld()->allWith<SkeletalAnimationComponent>()) {
    auto animationComponent = obj.getComponent<SkeletalAnimationComponent>();
    auto& statesMachine = animationComponent->getAnimationStatesMachineRef();

    if (statesMachine.isActive()) {
      updateAnimationStateMachine(statesMachine, delta);

      if (obj.hasComponent<MeshRendererComponent>()) {
        updateObjectBounds(*obj.getComponent<TransformComponent>().get(),
          *animationComponent.get(),
          *obj.getComponent<MeshRendererComponent>().get(),
          delta);
      }
    }
  }
}

void SkeletalAnimationSystem::updateAnimationStateMachine(AnimationStatesMachine& stateMachine, float delta)
{
  stateMachine.increaseCurrentTime(delta);
}

void SkeletalAnimationSystem::updateObjectBounds(TransformComponent& transformComponent,
  SkeletalAnimationComponent& skeletalAnimationComponent,
  MeshRendererComponent& meshRendererComponent,
  float delta)
{
  ARG_UNUSED(delta);

  glm::mat4 boundTransformation = transformComponent.getTransform().getTransformationMatrix() *
    skeletalAnimationComponent.getMatrixPalette().bonesTransforms[0];

  meshRendererComponent.updateBounds(boundTransformation);
}
