#pragma once

#include "Scene.h"
#include <string>

// Serializer for saving/loading Scenes to/from disk.
namespace aether {

    class SceneSerializer
    {
    public:
        SceneSerializer(Scene* scene);

        void Serialize(const std::string& filepath);
        void Deserialize(const std::string& filepath);

    private:
        Scene* m_Scene;
    };

}