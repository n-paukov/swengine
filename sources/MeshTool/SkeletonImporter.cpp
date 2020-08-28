#include "SkeletonImporter.h"

#include <unordered_set>
#include <spdlog/spdlog.h>
#include <Engine/swdebug.h>
#include <Engine/Exceptions/exceptions.h>

#include "AssimpMeshLoader.h"
#include "utils.h"

SkeletonImporter::SkeletonImporter()
{

}

std::unique_ptr<RawSkeleton> SkeletonImporter::importFromFile(const std::string& path,
  const SkeletonImportOptions& options)
{
  spdlog::info("Load source mesh: {}", path);

  AssimpMeshLoadOptions assimpOptions;
  assimpOptions.maxBonexPerVertex = options.maxBonexPerVertex;

  std::unique_ptr<AssimpScene> scene = AssimpMeshLoader::loadScene(path, assimpOptions);

  spdlog::info("Source mesh is loaded");
  spdlog::info("Start mesh parsing");

  std::unique_ptr<RawSkeleton> skeleton = convertSceneToSkeleton(scene->getScene(), options);

  spdlog::info("Mesh is parsed, skeleton is extracted ({} bones)",
    skeleton->header.bonesCount);

  return skeleton;
}

#include <glm/gtx/string_cast.hpp>

std::unique_ptr<RawSkeleton> SkeletonImporter::convertSceneToSkeleton(const aiScene& scene,
  const SkeletonImportOptions& options)
{
  ARG_UNUSED(options);

  std::unordered_map<std::string, const aiBone*> usedBonesList = collectBones(scene);
  std::unordered_map<std::string, ImportSceneBoneData> bonesList;

  const aiNode* rootNode = findRootBoneNode(scene.mRootNode, usedBonesList);

  if (rootNode == nullptr) {
    THROW_EXCEPTION(EngineRuntimeException, "Failed to find the root bone");
  }

  spdlog::warn("Root trasform: {}", glm::to_string(aiMatrix4x4ToGlm(&scene.mRootNode->mTransformation)));

  aiMatrix4x4 rootTransform;
  aiIdentityMatrix4(&rootTransform);

  const aiNode* currentNode = rootNode->mParent;

  while (currentNode != nullptr) {
    rootTransform = currentNode->mTransformation * rootTransform;
    currentNode = currentNode->mParent;
  }

  traverseSkeletonHierarchy(rootNode, rootTransform, usedBonesList, bonesList);

  //glm::mat4 transform = glm::inverse(aiMatrix4x4ToGlm(&bonesList.begin()->second.sceneTransfromationMatrix));
  //glm::vec4 vertex = transform * glm::vec4(5.0f, 0.0f, 0.0f, 1.0f);

  std::vector<RawBone> rawBonesList;
  buildSkeleton(rootNode, bonesList, rawBonesList, -1);

  SW_ASSERT(rawBonesList.size() > 0);

  std::unique_ptr<RawSkeleton> skeleton = std::make_unique<RawSkeleton>();
  skeleton->header.formatVersion = SKELETON_FORMAT_VERSION;
  skeleton->header.bonesCount = static_cast<uint8_t>(rawBonesList.size());
  skeleton->bones = std::move(rawBonesList);

//  for (auto bone : skeleton->bones) {
//    auto translation = rawMatrix4ToGLMMatrix4(bone.inverseBindPoseMatrix);
//    auto translation2 = aiMatrix4x4ToGlm(&usedBonesList[std::string(bone.name)]->mOffsetMatrix);
//
//    auto t1 = aiVec3ToGlm(usedBonesList[std::string(bone.name)]->mOffsetMatrix * aiVector3D(0.0f, 0.0f, 0.0f));
//
//    spdlog::warn("Skeleton: {}", glm::to_string(translation));
//    spdlog::warn("Orig: {} {}", glm::to_string(translation2), glm::to_string(t1));
//  }

  return skeleton;
}

std::unordered_map<std::string, const aiBone*> SkeletonImporter::collectBones(const aiScene& scene) const
{
  std::unordered_map<std::string, const aiBone*> bonesList;

  for (size_t meshIndex = 0; meshIndex < scene.mNumMeshes; meshIndex++) {
    const aiMesh& mesh = *scene.mMeshes[meshIndex];

    if (mesh.HasBones()) {
      for (size_t boneIndex = 0; boneIndex < mesh.mNumBones; boneIndex++) {
        const aiBone* bone = mesh.mBones[boneIndex];
        std::string boneName = bone->mName.C_Str();

        auto boneIt = bonesList.find(boneName);

        if (boneIt != bonesList.end() && boneIt->second != bone) {
          THROW_EXCEPTION(EngineRuntimeException, "Failed to collect bones names, "
                                                  "there are to bones with the same name");
        }

        bonesList.insert({boneName, bone});
      }
    }
  }

  return bonesList;
}

void SkeletonImporter::traverseSkeletonHierarchy(const aiNode* sceneNode,
  const aiMatrix4x4& parentNodeTransform,
  const std::unordered_map<std::string, const aiBone*>& usedBones,
  std::unordered_map<std::string, ImportSceneBoneData>& bonesData)
{
  std::string currentNodeName = sceneNode->mName.C_Str();
  aiMatrix4x4 currentNodeTransform = parentNodeTransform * sceneNode->mTransformation;

  auto usedBoneIt = usedBones.find(currentNodeName);

  if (usedBoneIt != usedBones.end()) {
    //aiMatrix4x4 tempInvMatrixs = currentNodeTransform.Inverse() * usedBoneIt->second->mOffsetMatrix;

    aiMatrix4x4 offsetMatrix = usedBoneIt->second->mOffsetMatrix;

    offsetMatrix.Inverse();

    bonesData[currentNodeName] = {usedBoneIt->second, offsetMatrix};
  }

  for (size_t childIndex = 0; childIndex < sceneNode->mNumChildren; childIndex++) {
    const aiNode* childNode = sceneNode->mChildren[childIndex];

    traverseSkeletonHierarchy(childNode, currentNodeTransform, usedBones, bonesData);
  }
}

const aiNode* SkeletonImporter::findRootBoneNode(const aiNode* sceneRootNode,
  const std::unordered_map<std::string,
                           const aiBone*>& bonesList) const
{
  for (auto[boneName, bone] : bonesList) {
    const aiNode* boneNode = sceneRootNode->FindNode(boneName.c_str());
    std::string boneNodeName = boneNode->mParent->mName.C_Str();

    if (boneNode != nullptr && boneNode->mParent != nullptr &&
      bonesList.find(boneNodeName) == bonesList.end()) {
      return boneNode;
    }
  }

  return nullptr;
}

void SkeletonImporter::buildSkeleton(const aiNode* skeletonNode,
  const std::unordered_map<std::string, ImportSceneBoneData>& bonesList,
  std::vector<RawBone>& rawBonesList,
  int parentBoneId) const
{
  SW_ASSERT(skeletonNode != nullptr);
  SW_ASSERT(bonesList.size() > 0);

  std::string boneName = skeletonNode->mName.C_Str();

  // Fail with error if there is non-leaf node that is not presented in bones list as a bone
  auto boneIt = bonesList.find(boneName);

  if (boneIt == bonesList.end()) {
    if (skeletonNode->mNumChildren != 0) {
      if (skeletonNode->mNumMeshes == 0) {
        THROW_EXCEPTION(EngineRuntimeException, "Failed to build skeleton, "
                                                "the bone node is not presented in list of bones' names ( " + boneName
          + " )");
      }
      else {
        THROW_EXCEPTION(EngineRuntimeException, "There are node with meshes in skeleton tree (" + boneName + ") "
                                                                                                             "such format is not supported");
      }
    }
    else {
      // It is a leaf node, just skip it
      spdlog::warn("Leaf node {} is skipped", boneName);
      return;
    }

  }

  const aiBone* boneData = boneIt->second.bone;
  SW_ASSERT(boneData != nullptr);

  if (boneName.size() > MAX_BONE_NAME_LENGTH) {
    THROW_EXCEPTION(EngineRuntimeException, "The bone name is too long");
  }

  // Fill RawBone data structure
  RawBone bone;
  strcpy_s(bone.name, boneName.c_str());
  bone.parentId = static_cast<uint8_t>(parentBoneId);

  glm::mat4 inverseBindPoseMatrix = glm::inverse(aiMatrix4x4ToGlm(&boneIt->second.sceneTransfromationMatrix));
  bone.inverseBindPoseMatrix = glmMatrix4ToRawMatrix4(inverseBindPoseMatrix);

  rawBonesList.push_back(bone);

  // Continue to traverse nodes tree
  int newParentBoneId = static_cast<int>(rawBonesList.size()) - 1;

  for (size_t childIndex = 0; childIndex < skeletonNode->mNumChildren; childIndex++) {
    const aiNode* childNode = skeletonNode->mChildren[childIndex];

    buildSkeleton(childNode, bonesList, rawBonesList, newParentBoneId);
  }
}
