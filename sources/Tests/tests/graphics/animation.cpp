#include "animation.h"

Skeleton generateTestSkeleton()
{
  std::vector<Bone> bones;

  bones.push_back(Bone("root", Bone::ROOT_BONE_PARENT_ID,
    glm::inverse(MathUtils::getTranslationMatrix({1.0f, 0.0f, 0.0f}))));
  bones.push_back(Bone("spin", 0, glm::inverse(MathUtils::getTranslationMatrix({0.0f, 1.0f, 0.0f}))));
  bones.push_back(Bone("spin_child", 1, glm::inverse(MathUtils::getTranslationMatrix({0.0f, 0.0f, 1.0f}))));

  Skeleton skeleton(bones);

  return skeleton;
}

AnimationClip generateTestAnimationClip()
{
  std::vector<BoneAnimationChannel> bonesAnimationChannels;

  bonesAnimationChannels.push_back(BoneAnimationChannel({
    BoneAnimationPositionFrame{0.0f, {0.0f, 0.0f, 0.0f}},
    BoneAnimationPositionFrame{30.0f, {30.0f, 0.0f, 0.0f}},
    BoneAnimationPositionFrame{60.0f, {0.0f, 0.0f, 0.0f}}
  }, {}));

  bonesAnimationChannels.push_back(BoneAnimationChannel({
    BoneAnimationPositionFrame{0.0f, {0.0f, 0.0f, 0.0f}},
    BoneAnimationPositionFrame{30.0f, {00.0f, 30.0f, 0.0f}},
    BoneAnimationPositionFrame{60.0f, {0.0f, 0.0f, 0.0f}}
  }, {
    BoneAnimationOrientationFrame{0.0f,
      glm::angleAxis(glm::radians(0.0f), MathUtils::AXIS_X)},
    BoneAnimationOrientationFrame{30.0f,
      glm::angleAxis(glm::radians(90.0f), MathUtils::AXIS_X)},
    BoneAnimationOrientationFrame{60.0f,
      glm::angleAxis(glm::radians(0.0f), MathUtils::AXIS_X)},
  }));

  bonesAnimationChannels.push_back(BoneAnimationChannel({}, {}));

  AnimationClip animationClip = AnimationClip("clip", 60.0f, 30.0f, bonesAnimationChannels);

  return animationClip;
}

AnimationClipInstance generateTestAnimationClipInstance()
{
  auto skeleton = std::make_shared<Skeleton>(generateTestSkeleton());
  auto clip = std::make_shared<AnimationClip>(generateTestAnimationClip());

  AnimationClipInstance clipInstance(skeleton, clip);

  return clipInstance;
}
