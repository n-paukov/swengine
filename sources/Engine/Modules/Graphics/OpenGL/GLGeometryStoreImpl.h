#pragma once

#include "GLGeometryStore.h"
#include "GLDebug.h"

template<class T>
void GLGeometryStore::createBuffersAndVAO(const std::vector<T> &vertices, const std::vector<uint16_t> &indices)
{
    // Create and fill vertex buffer
    GL_CALL_BLOCK_BEGIN();

    glCreateBuffers(1, &m_vertexBuffer);
    glNamedBufferStorage(m_vertexBuffer,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(T)),
        vertices.data(), GL_DYNAMIC_STORAGE_BIT);

    m_verticesCount = vertices.size();

    GL_CALL_BLOCK_END();

    // Create and fill index buffer if is is needed
    if (indices.size() > 0) {
        GLsizeiptr indicesMemorySize = static_cast<GLsizeiptr>(indices.size() * sizeof(indices[0]));

        GL_CALL_BLOCK_BEGIN();
        glCreateBuffers(1, &m_indexBuffer);
        glNamedBufferStorage(m_indexBuffer, indicesMemorySize, indices.data(), GL_DYNAMIC_STORAGE_BIT);

        m_indicesCount = indices.size();

        GL_CALL_BLOCK_END();
    }

    // Create and set up Vertex Array Object
    createVAOAndSetupAttributes<T>();
}

template<>
void GLGeometryStore::createVAOAndSetupAttributes<VertexPos3Norm3UV>()
{
    glCreateVertexArrays(1, &m_vertexArrayObject);

    glVertexArrayVertexBuffer(m_vertexArrayObject, 0, m_vertexBuffer, 0, sizeof(VertexPos3Norm3UV));

    if (isIndexed()) {
        glVertexArrayElementBuffer(m_vertexArrayObject, m_indexBuffer);
    }

    glEnableVertexArrayAttrib(m_vertexArrayObject, 0);
    glEnableVertexArrayAttrib(m_vertexArrayObject, 1);
    glEnableVertexArrayAttrib(m_vertexArrayObject, 2);

    glVertexArrayAttribFormat(m_vertexArrayObject, 0, 3, GL_FLOAT, GL_FALSE, offsetof(VertexPos3Norm3UV, pos));
    glVertexArrayAttribFormat(m_vertexArrayObject, 1, 3, GL_FLOAT, GL_FALSE, offsetof(VertexPos3Norm3UV, norm));
    glVertexArrayAttribFormat(m_vertexArrayObject, 2, 2, GL_FLOAT, GL_FALSE, offsetof(VertexPos3Norm3UV, uv));

    glVertexArrayAttribBinding(m_vertexArrayObject, 0, 0);
    glVertexArrayAttribBinding(m_vertexArrayObject, 1, 0);
    glVertexArrayAttribBinding(m_vertexArrayObject, 2, 0);
}
