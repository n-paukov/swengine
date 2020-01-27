#pragma once

#include <vector>
#include <utility>
#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Modules/Graphics/OpenGL/GLGeometryStore.h"
#include "Modules/Math/geometry.h"

enum class MeshAttributes {
    Empty = 0,
    Positions = 1,
    Normals = 2,
    UV = 4,
    Tangents = 8,
    BonesIDs = 16,
    BonesWeights = 32,
};

class Mesh
{
public:
    Mesh();
    ~Mesh();

    size_t addSubMesh(const std::vector<uint16_t>& indices);
    void setIndices(const std::vector<uint16_t>& indices, size_t subMeshIndex);

    void setVertices(const std::vector<glm::vec3>& vertices);
    void setNormals(const std::vector<glm::vec3>& normals);
    void setTangents(const std::vector<glm::vec3>& tangents);
    void setUV(const std::vector<glm::vec2>& uv);

    void setSkinData(const std::vector<glm::u8vec4>& bonesIDs, const std::vector<glm::u8vec4>& bonesWeights);

    bool hasVertices() const;
    bool hasNormals() const;
    bool hasTangents() const;
    bool hasUV() const;
    bool isSkinned() const;

    void setSubMeshesIndices(const std::vector<uint16_t>& indices, const std::vector<uint16_t>& subMeshesOffsets);

    size_t getSubMeshesCount() const;
    size_t getSubMeshIndicesOffset(size_t subMeshIndex) const;
    size_t getSubMeshIndicesCount(size_t subMeshIndex) const;

    GLGeometryStore* getGeometryStore();

    void setAABB(const AABB& aabb);
    const AABB& getAABB() const;

private:
    void calculateSubMeshesOffsets();

    void updateGeometryBuffer();

    template<class T>
    std::vector<T> constructVerticesList() const;

private:
    std::unique_ptr<GLGeometryStore> m_geometryStore;

    std::vector<glm::vec3> m_vertices;

    std::vector<std::vector<uint16_t>> m_indices;
    std::vector<size_t> m_subMeshesOffsets;

    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec3> m_tangents;
    std::vector<glm::vec2> m_uv;
    std::vector<glm::u8vec4> m_bonesIDs;
    std::vector<glm::u8vec4> m_bonesWeights;

    bool m_needGeometryBufferUpdate = false;

    AABB m_aabb;
};


inline MeshAttributes operator|(MeshAttributes a, MeshAttributes b)
{
    return static_cast<MeshAttributes>(static_cast<int>(a) | static_cast<int>(b));
}

inline MeshAttributes operator&(MeshAttributes a, MeshAttributes b)
{
    return static_cast<MeshAttributes>(static_cast<int>(a) & static_cast<int>(b));
}

inline MeshAttributes operator~(MeshAttributes a)
{
    return static_cast<MeshAttributes>(~static_cast<int>(a));
}
