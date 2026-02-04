#pragma once

#include "../ecs/Registry.h"
#include "../core/systems/SystemScheduler.h"
#include "../core/UUID.h"

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace aether {

    class Entity;

    /**
     * Scene
     * The self-contained world simulation context.
     * * HYBRID ARCHITECTURE:
     * 1. Decoupled from Application: Does not know about EditorCameras or Windows.
     * 2. Split Pipeline:
     * - OnUpdateSimulation: Runs the Authoritative Logic (Scheduler).
     * - OnRender: Renders the current state to ANY provided view matrix.
     */
    class Scene {
    public:
        Scene();
        ~Scene() = default;

        /**
         * Creates a deep copy of the scene.
         * Used by the Editor to fork the "Edit" scene into a "Runtime" scene.
         * Implemented via Serialization to ensure exact state replication.
         */
        static std::shared_ptr<Scene> Copy(std::shared_ptr<Scene> other);

        /**
         * Creates an entity with a generated UUID.
         * Used for runtime spawning.
         */
        Entity CreateEntity(const std::string& name = std::string());

        /**
         * Creates an entity with a specific UUID.
         * IMPORTANT:
         * Required for the Serializer to restore entity relationships reliably.
         */
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());

        void DestroyEntity(Entity entity);

        /**
         * Simulation Step (Fixed-Step).
         * Executes the SystemScheduler (Input, Physics, Logic).
         * Called by EditorLayer (Play Mode) and Client Game Loop.
         * TODO: Ensure server controls this!
         */
        void OnUpdateSimulation(float deltaTime);

        /**
         * Presentation Step (Variable-Step).
         * Renders the scene to the provided ViewProjection matrix.
         * DECOUPLING:
         * - Editor calls this with m_EditorCamera.GetViewProjection().
         * - Client calls this with the active CameraComponent's ViewProjection.
         */
        void OnRender(const glm::mat4& viewProjection);

        /**
         * Viewport Resize.
         * Updates the AspectRatio of active CameraComponents.
         * Needed because the Scene Viewport size != Window Size (in Editor).
         */
        void OnViewportResize(uint32_t width, uint32_t height);

        Entity GetPrimaryCameraEntity();

        // Data-Driven System Configuration
        void AddSystemConfig(const std::string& systemName) { m_SystemConfigs.push_back(systemName); }
        const std::vector<std::string>& GetSystemConfigs() const { return m_SystemConfigs; }

        Registry& GetRegistry() { return m_Registry; }

        // Helper to get the ViewProjection of the primary runtime camera
        // Returns Identity if no camera is active.
        glm::mat4 GetPrimaryCameraViewProjection();

    private:
        // Lazy-loads systems from m_SystemConfigs
        void EnsureSystemsLoaded();

    private:
        Registry m_Registry;
        SystemScheduler m_Scheduler;

        std::vector<std::string> m_SystemConfigs;
        bool m_SystemsLoaded = false;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

}