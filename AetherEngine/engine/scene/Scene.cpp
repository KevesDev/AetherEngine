#include "Scene.h"
#include "../ecs/Components.h"
#include "../renderer/Renderer2D.h"
#include "../ecs/Entity.h"

namespace aether {

    Scene::Scene() {
    }

    Scene::~Scene() {
    }

    Entity Scene::CreateEntity(const std::string& name) {
        Entity entity = { m_Registry.CreateEntity(), &m_Registry };
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        entity.AddComponent<TransformComponent>();
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        m_Registry.DestroyEntity(entity.GetID());
    }

    void Scene::OnUpdateRuntime(float ts) {
        // TODO:
        // 1. Scripting/AI (Future LogicGraph integration goes here)

        // 2. Physics (Placeholder for future PhysicsSystem)

        // 3. Rendering
        // The View guarantees every entity yielded has both components.
        auto view = m_Registry.view<TransformComponent, SpriteComponent>();
        for (auto entityID : view) {
            Entity entity = { entityID, &m_Registry };
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& sprite = entity.GetComponent<SpriteComponent>();

            Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
        }
    }

    void Scene::OnUpdateEditor(float ts, EditorCamera& camera) {
        Renderer2D::BeginScene(camera);

        auto view = m_Registry.view<TransformComponent, SpriteComponent>();
        for (auto entityID : view) {
            Entity entity = { entityID, &m_Registry };
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& sprite = entity.GetComponent<SpriteComponent>();

            Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
        }

        Renderer2D::EndScene();
    }
}