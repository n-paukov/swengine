#include "MeshRenderingSystem.h"

#include "Modules/ECS/ECS.h"

#include "TransformComponent.h"
#include "MeshRendererComponent.h"

MeshRenderingSystem::MeshRenderingSystem(std::shared_ptr<GLGraphicsContext> graphicsContext,
                                         std::shared_ptr<SharedGraphicsState> sharedGraphicsState)
    : RenderingSystem(graphicsContext, sharedGraphicsState)
{

}

MeshRenderingSystem::~MeshRenderingSystem()
{

}

void MeshRenderingSystem::configure(GameWorld* gameWorld)
{
    ARG_UNUSED(gameWorld);
}

void MeshRenderingSystem::unconfigure(GameWorld* gameWorld)
{
    ARG_UNUSED(gameWorld);
}

void MeshRenderingSystem::update(GameWorld* gameWorld, float delta)
{
    ARG_UNUSED(gameWorld);
    ARG_UNUSED(delta);
}

void MeshRenderingSystem::renderForward(GameWorld* gameWorld)
{
    for (const GameObject* obj : gameWorld->allWith<MeshRendererComponent, TransformComponent>()) {
        const auto& meshComponent = obj->getComponent<MeshRendererComponent>();

        if (meshComponent->isCulled()) {
            continue;
        }

        Mesh* mesh = meshComponent->getMeshInstance().get();

        SW_ASSERT(mesh != nullptr);

        Transform* transform = obj->getComponent<TransformComponent>()->getTransform();

        const size_t subMeshesCount = mesh->getSubMeshesCount();

        SW_ASSERT(subMeshesCount != 0);

        m_sharedGraphicsState->getFrameStats().increaseSubMeshesCount(subMeshesCount);

        for (size_t subMeshIndex = 0; subMeshIndex < subMeshesCount; subMeshIndex++) {
            Material* material = meshComponent->getMaterialInstance(subMeshIndex).get();

            SW_ASSERT(material != nullptr);

            GLShadersPipeline* shadersPipeline = material->getGpuMaterial().getShadersPipeline().get();

            SW_ASSERT(shadersPipeline != nullptr);

            GLShader* vertexShader = shadersPipeline->getShader(GL_VERTEX_SHADER);

            if (transform != nullptr) {
                if (vertexShader->hasParameter("transform.localToWorld")) {
                    vertexShader->setParameter("transform.localToWorld", transform->getTransformationMatrix());
                }
            }

            Camera* camera = m_sharedGraphicsState->getActiveCamera().get();

            if (camera != nullptr) {
                if (vertexShader->hasParameter("scene.worldToCamera")) {
                    vertexShader->setParameter("scene.worldToCamera", camera->getViewMatrix());
                    vertexShader->setParameter("scene.cameraToProjection", camera->getProjectionMatrix());
                }
            }

            m_sharedGraphicsState->getFrameStats().increasePrimitivesCount(mesh->getSubMeshIndicesCount(subMeshIndex) / 3);

            m_graphicsContext->executeRenderTask({
                &material->getGpuMaterial(),
                mesh->getGeometryStore(),
                mesh->getSubMeshIndicesOffset(subMeshIndex),
                mesh->getSubMeshIndicesCount(subMeshIndex)
            });
        }
    }
}
