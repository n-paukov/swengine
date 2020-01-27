#include "MeshImporter.h"

#include <spdlog/spdlog.h>

#include <string>
#include <bitset>
#include <Engine/Exceptions/EngineRuntimeException.h>
#include <Engine/swdebug.h>

#include "AssimpMeshLoader.h"
#include "SkeletonImporter.h"

MeshImporter::MeshImporter()
{

}

std::unique_ptr<RawMesh> MeshImporter::importFromFile(const std::string& path, const MeshImportOptions& options)
{    
    spdlog::info("Load source mesh: {}", path);

    AssimpMeshLoadOptions assimpOptions;
    assimpOptions.flipUV = options.flipUV;
    assimpOptions.glueByMaterials = options.glueByMaterials;
    assimpOptions.calculateTangents = options.calculateTangents;
    assimpOptions.joinIdenticalVertices = options.joinIdenticalVertices;
    assimpOptions.maxBonexPerVertex = options.maxBonesPerVertex;

    std::unique_ptr<AssimpScene> scene = AssimpMeshLoader::loadScene(path, assimpOptions);

    spdlog::info("Source mesh is loaded");
    spdlog::info("Start mesh parsing");

    std::unique_ptr<RawSkeleton> skeleton = nullptr;

    if (options.loadSkin) {
        spdlog::info("Start to load mesh skeleton...");

        skeleton = getSkeleton(path, options);

        spdlog::info("Mesh skeleton is loaded");
    }

    std::unique_ptr<RawMesh> mesh = convertSceneToMesh(scene->getScene(), skeleton.get(), options);

    spdlog::info("Mesh is parsed ({} vertices, {} indices, {} submeshes)",
                 mesh->header.verticesCount, mesh->header.indicesCount, mesh->header.subMeshesIndicesOffsetsCount);

    return mesh;
}

std::unique_ptr<RawMesh> MeshImporter::convertSceneToMesh(const aiScene& scene,
                                                          const RawSkeleton* skeleton,
                                                          const MeshImportOptions& options)
{
    SW_ASSERT(!options.loadSkin || (options.loadSkin && skeleton != nullptr));


    std::unordered_map<std::string, int> bonesMap;

    if (options.loadSkin) {
        bonesMap = getBonesMap(*skeleton);
    }

    std::vector<RawVector3> positions;
    std::vector<RawVector3> normals;
    std::vector<RawVector3> tangents;
    std::vector<RawVector2> uv;

    std::vector<uint8_t> bonesFreeDataPosition;
    std::vector<RawU8Vector4> bonesIDs;
    std::vector<RawU8Vector4> bonesWeights;

    std::vector<std::vector<std::uint16_t>> subMeshesIndices;

    glm::vec3 aabbMin(std::numeric_limits<float>::max());
    glm::vec3 aabbMax(std::numeric_limits<float>::min());

    std::unordered_map<std::string, const aiMesh*> meshesList;

    aiMatrix4x4 rootTransform;
    aiIdentityMatrix4(&rootTransform);

    collectMeshes(scene, *scene.mRootNode, meshesList, rootTransform);

    if (meshesList.empty()) {
        ENGINE_RUNTIME_ERROR("Failed to import mesh, geometry is not found");
    }

    for (auto [ subMeshName, subMeshPtr ] : meshesList) {
        const aiMesh& subMesh = *subMeshPtr;
        size_t subMeshIndex = subMeshesIndices.size() - 1;

        bool requiredAttributesFound = subMesh.HasPositions() && subMesh.HasNormals()
                && subMesh.HasTextureCoords(0) && subMesh.HasFaces() && subMesh.HasTangentsAndBitangents();

        if (!requiredAttributesFound) {
            spdlog::info("Submesh #{} ({}) is incomplete and was skipped", subMeshIndex, subMeshName);
            continue;
        }

        // Vertices
        size_t verticesAddIndex = positions.size();

        for (size_t vertexIndex = 0; vertexIndex < subMesh.mNumVertices; vertexIndex++) {
            positions.push_back({ subMesh.mVertices[vertexIndex].x,
                                  subMesh.mVertices[vertexIndex].y,
                                  subMesh.mVertices[vertexIndex].z });
            aabbMin.x = std::fminf(aabbMin.x, subMesh.mVertices[vertexIndex].x);
            aabbMin.y = std::fminf(aabbMin.y, subMesh.mVertices[vertexIndex].y);
            aabbMin.z = std::fminf(aabbMin.z, subMesh.mVertices[vertexIndex].z);

            aabbMax.x = std::fmaxf(aabbMax.x, subMesh.mVertices[vertexIndex].x);
            aabbMax.y = std::fmaxf(aabbMax.y, subMesh.mVertices[vertexIndex].y);
            aabbMax.z = std::fmaxf(aabbMax.z, subMesh.mVertices[vertexIndex].z);

            normals.push_back({ subMesh.mNormals[vertexIndex].x,
                                subMesh.mNormals[vertexIndex].y,
                                subMesh.mNormals[vertexIndex].z });

            uv.push_back({ subMesh.mTextureCoords[0][vertexIndex].x,
                           subMesh.mTextureCoords[0][vertexIndex].y });

            bonesIDs.push_back({ 0, 0, 0, 0 });
            bonesWeights.push_back({ 0, 0, 0, 0 });
            bonesFreeDataPosition.push_back(0);

            // TODO: implement tangents, bitangents, bones import
        }

        // Indices
        bool nonTrianglePolygonFound = false;

        size_t indicesOffset = verticesAddIndex;

        std::vector<uint16_t> indices;

        for (size_t faceIndex = 0; faceIndex < subMesh.mNumFaces; faceIndex++) {
            const aiFace& face = subMesh.mFaces[faceIndex];

            if (face.mNumIndices != 3) {
                nonTrianglePolygonFound = true;
                break;
            }

            for (size_t indexNumber = 0; indexNumber < 3; indexNumber++) {
                indices.push_back(static_cast<uint16_t>(face.mIndices[indexNumber] + indicesOffset));
            }
        }

        if (nonTrianglePolygonFound) {
            spdlog::info("Submesh #{} ({}) has non-triangle polygon and was skipped", subMeshIndex, subMeshName);
            continue;
        }

        if (options.loadSkin) {
            if (!subMesh.HasBones()) {
                spdlog::warn("Submesh #{} ({}) has not any attached bones", subMeshIndex, subMeshName);
            }

            for (size_t boneIndex = 0; boneIndex < subMesh.mNumBones; boneIndex++) {
                const aiBone& bone = *subMesh.mBones[boneIndex];

                std::string boneName = bone.mName.C_Str();

                auto rawBoneIt = bonesMap.find(boneName);
                int skeletonBoneIndex = rawBoneIt->second;

                if (rawBoneIt == bonesMap.end()) {
                    ENGINE_RUNTIME_ERROR("Bone " + boneName + " that is attached to the submesh is not found in the skeleton");
                }

                for (size_t weightIndex = 0; weightIndex < bone.mNumWeights; weightIndex++) {
                    const aiVertexWeight& vertexWeight = bone.mWeights[weightIndex];

                    size_t affectedVertexId = verticesAddIndex + vertexWeight.mVertexId;
                    uint8_t weight = static_cast<uint8_t>(vertexWeight.mWeight * 255);

                    uint8_t boneDataPosition = bonesFreeDataPosition[affectedVertexId];

                    SW_ASSERT(boneDataPosition < options.maxBonesPerVertex);

                    bonesIDs[affectedVertexId].data[boneDataPosition] = static_cast<uint8_t>(skeletonBoneIndex);
                    bonesWeights[affectedVertexId].data[boneDataPosition] = weight;

                    bonesFreeDataPosition[affectedVertexId]++;
                }
            }
        }

        // Correct bones influence weights
        int unskinnedVerticesCount = 0;

        for (RawU8Vector4& weights : bonesWeights) {
            int weightsSum = weights.x + weights.y + weights.z + weights.w;

            if (weightsSum == 0) {
                unskinnedVerticesCount++;
                continue;
            }

            int weightsAddition = 255 - weightsSum;

            // TODO: remove hardcode, do it some more convinient way
            SW_ASSERT(weightsSum == 253 || weightsSum == 254 || weightsSum == 255);

            if (weights.x >= weights.y && weights.x >= weights.z && weights.x >= weights.w) {
                weights.x += weightsAddition;
            }
            else if (weights.y >= weights.x && weights.y >= weights.z && weights.y >= weights.w) {
                weights.y += weightsAddition;
            }
            else if (weights.z >= weights.x && weights.z >= weights.y && weights.z >= weights.w) {
                weights.y += weightsAddition;
            }
            else {
                weights.z += weightsAddition;
            }

            SW_ASSERT(weights.x + weights.y + weights.z + weights.w == 255);
        }

        if (unskinnedVerticesCount > 0) {
            spdlog::warn("There is {} unskinned vertices", unskinnedVerticesCount);
        }

        subMeshesIndices.push_back(std::move(indices));
    }

    // Mesh formation
    SW_ASSERT(positions.size() == normals.size() && positions.size() == uv.size());

    std::unique_ptr<RawMesh> mesh = std::make_unique<RawMesh>();

    mesh->positions = positions;
    mesh->normals = normals;
    mesh->tangents = tangents;
    mesh->uv = uv;
    mesh->bonesIds = bonesIDs;
    mesh->bonesWeights = bonesWeights;

    for (const auto& subMeshIndices : subMeshesIndices) {
        mesh->subMeshesIndicesOffsets.push_back(static_cast<uint16_t>(mesh->indices.size()));
        mesh->indices.insert(mesh->indices.end(), subMeshIndices.begin(), subMeshIndices.end());
    }

    mesh->aabb = AABB(aabbMin, aabbMax);

    const uint16_t verticesCount = static_cast<uint16_t>(mesh->positions.size());
    const uint16_t indicesCount = static_cast<uint16_t>(mesh->indices.size());

    mesh->header.formatVersion = MESH_FORMAT_VERSION;
    mesh->header.verticesCount = verticesCount;
    mesh->header.indicesCount = indicesCount;
    mesh->header.subMeshesIndicesOffsetsCount = static_cast<uint16_t>(subMeshesIndices.size());

    RawMeshAttributes storedAttributesMask = RawMeshAttributes::Empty;
    storedAttributesMask = RawMeshAttributes::Positions | RawMeshAttributes::Normals | RawMeshAttributes::UV;
    mesh->header.storedAttributesMask = static_cast<bitmask64>(storedAttributesMask);

    return mesh;
}

void MeshImporter::collectMeshes(const aiScene& scene,
                                 const aiNode& sceneNode,
                                 std::unordered_map<std::string, const aiMesh*>& meshesList,
                                 const aiMatrix4x4& parentNodeTransform) const
{
    std::string currentNodeName = sceneNode.mName.C_Str();
    aiMatrix4x4 currentNodeTransform = parentNodeTransform * sceneNode.mTransformation;

    for (size_t meshIndex = 0; meshIndex < sceneNode.mNumMeshes; meshIndex++) {
        const aiMesh* mesh = scene.mMeshes[sceneNode.mMeshes[meshIndex]];

        std::string meshName = mesh->mName.C_Str();

        auto meshIt = meshesList.find(meshName);

        if (meshIt != meshesList.end()) {
            // The mesh is already collected
            // Nodes structure and ability to use one mesh in two different nodes is ignored

            spdlog::warn("The same mesh is attached to multiple nodes ({}), attachment is skipped (node {})",
                         meshName, currentNodeName);
            continue;
        }

        if (!currentNodeTransform.IsIdentity()) {
            spdlog::warn("The mesh {} node {} has non-identity transform, "
                         "all transform data will be skipped", meshName, currentNodeName);
        }

        meshesList.insert({ meshName, mesh });
    }


    for (size_t childIndex = 0; childIndex < sceneNode.mNumChildren; childIndex++) {
        const aiNode* childNode = sceneNode.mChildren[childIndex];

        collectMeshes(scene, *childNode, meshesList, currentNodeTransform);
    }
}

std::unique_ptr<RawSkeleton> MeshImporter::getSkeleton(const std::string& path, const MeshImportOptions& options) const
{
    SkeletonImporter importer;
    SkeletonImportOptions importOptions;
    importOptions.maxBonexPerVertex = options.maxBonesPerVertex;

    return importer.importFromFile(path, importOptions);
}

std::unordered_map<std::string, int> MeshImporter::getBonesMap(const RawSkeleton& skeleton) const
{
    std::unordered_map<std::string, int> bonesMap;

    for (size_t boneIndex = 0; boneIndex < skeleton.bones.size(); boneIndex++) {
        const RawBone& bone = skeleton.bones[boneIndex];
        bonesMap[std::string(bone.name)] = static_cast<int>(boneIndex);
    }

    return bonesMap;
}
