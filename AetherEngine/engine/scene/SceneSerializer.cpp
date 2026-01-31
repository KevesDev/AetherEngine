#include "SceneSerializer.h"
#include "../ecs/Entity.h"
#include "../ecs/Components.h"
#include "../core/VFS.h"
#include "../core/Log.h"

// Vendor Include
#include "../vendor/json.hpp" 

using json = nlohmann::json;

namespace aether {

    SceneSerializer::SceneSerializer(Scene* scene)
        : m_Scene(scene)
    {
    }

    // Helper to serialize an individual entity safely
    static void SerializeEntity(json& outJson, Entity entity)
    {
        // 1. Tag Component (Required)
        if (entity.HasComponent<TagComponent>())
        {
            outJson["Tag"] = entity.GetComponent<TagComponent>().Tag;
        }

        // 2. Transform Component
        if (entity.HasComponent<TransformComponent>())
        {
            auto& tc = entity.GetComponent<TransformComponent>();
            outJson["Transform"] = {
                { "X", tc.X },
                { "Y", tc.Y },
                { "Rotation", tc.Rotation },
                { "ScaleX", tc.ScaleX },
                { "ScaleY", tc.ScaleY }
            };
        }

        // 3. Sprite Component
        if (entity.HasComponent<SpriteComponent>())
        {
            auto& sc = entity.GetComponent<SpriteComponent>();
            outJson["Sprite"] = {
                { "R", sc.R },
                { "G", sc.G },
                { "B", sc.B },
                { "A", sc.A }
            };
        }
    }

    void SceneSerializer::Serialize(const std::string& filepath)
    {
        json sceneJson;
        sceneJson["Scene"] = "Untitled";
        sceneJson["Entities"] = json::array();

        Registry& registry = m_Scene->GetRegistry();

        // Iterate all entities that have a Tag (which should be all of them)
        // Note: Registry::View returns the raw vector.
        // Registry::GetOwnerMap returns the mapping of Index -> EntityID.
        auto& tags = registry.View<TagComponent>();
        auto& ownerMap = registry.GetOwnerMap<TagComponent>();

        for (size_t i = 0; i < tags.size(); i++)
        {
            EntityID id = ownerMap.at(i);
            Entity entity(id, &registry);

            if (!entity) continue;

            json entityJson;
            SerializeEntity(entityJson, entity);
            sceneJson["Entities"].push_back(entityJson);
        }

        std::string dump = sceneJson.dump(4); // Pretty print
        VFS::WriteText(filepath, dump);
        AETHER_CORE_INFO("Serialized Scene to '{0}'", filepath);
    }

    void SceneSerializer::Deserialize(const std::string& filepath)
    {
        std::string fileContent = VFS::ReadText(filepath);
        if (fileContent.empty()) {
            AETHER_CORE_ERROR("Failed to load scene: {0}", filepath);
            return;
        }

        json sceneJson = json::parse(fileContent);

        auto entities = sceneJson["Entities"];
        if (entities.is_array())
        {
            for (auto& entityJson : entities)
            {
                std::string name = entityJson["Tag"];
                Entity deserializedEntity = m_Scene->CreateEntity(name);

                if (entityJson.contains("Transform"))
                {
                    auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                    auto& tJson = entityJson["Transform"];
                    tc.X = tJson["X"];
                    tc.Y = tJson["Y"];
                    tc.Rotation = tJson["Rotation"];
                    tc.ScaleX = tJson["ScaleX"];
                    tc.ScaleY = tJson["ScaleY"];
                }

                if (entityJson.contains("Sprite"))
                {
                    auto& sc = deserializedEntity.AddComponent<SpriteComponent>();
                    auto& sJson = entityJson["Sprite"];
                    sc.R = sJson["R"];
                    sc.G = sJson["G"];
                    sc.B = sJson["B"];
                    sc.A = sJson["A"];
                }
            }
        }
        AETHER_CORE_INFO("Deserialized Scene from '{0}'", filepath);
    }

}