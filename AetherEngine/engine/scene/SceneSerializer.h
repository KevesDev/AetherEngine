#pragma once

#include "Scene.h"
#include <memory>
#include <string>
#include <filesystem>
#include <iostream>

namespace aether {

    /**
     * SceneSerializer
     * Handles the serialization of a Scene and its entities to/from data formats.
     * * IMPORTANT:
     * Now using Stream-based API (SerializeToStream, DeserializeFromStream).
     * This enables:
     * 1. File I/O (Saving/Loading levels)
     * 2. Memory I/O (Runtime Scene Copying/Forking for Play Mode)
     * 3. Network I/O (Future state replication)
     * * This adheres to the "Generic Runtime Wrapper" principle by keeping
     * the serialization logic decoupled from the OS file system.
     */
    class SceneSerializer
    {
    public:
        SceneSerializer(const std::shared_ptr<Scene>& scene);

        // File System API
        void Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);

        // Stream API (Core Implementation)
        // Used by File API and Scene::Copy()
        void SerializeToStream(std::ostream& output);
        bool DeserializeFromStream(std::istream& input);

    private:
        std::shared_ptr<Scene> m_Scene;
    };
}