#pragma once
#include "../ecs/Registry.h"
#include "../editor/EditorCamera.h"

namespace aether {

    class Entity;

    class Scene {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = std::string());
        void DestroyEntity(Entity entity);

        void OnUpdateRuntime(float ts);
        void OnUpdateEditor(float ts, EditorCamera& camera);

        Registry& GetRegistry() { return m_Registry; }

    private:
        Registry m_Registry;
        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };
}