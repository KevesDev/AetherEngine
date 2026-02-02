#include "SceneSerializer.h"
#include "../ecs/Entity.h"
#include "../ecs/Components.h"
#include "../core/VFS.h"
#include "../core/Log.h"

// Vendor Include
#include "../vendor/json.hpp" 

using json = nlohmann::json;

namespace aether {

    SceneSerializer::SceneSerializer(Scene* scene) : m_Scene(scene) {}

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

        // 4. Camera Component
        if (entity.HasComponent<CameraComponent>()) {
            auto& cc = entity.GetComponent<CameraComponent>();
            outJson["CameraComponent"] = {
                { "Size", cc.Size },
                { "Near", cc.Near },
                { "Far", cc.Far },
                { "Primary", cc.Primary }
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
            for (auto& entityJson : entities) {
                std::string name = entityJson["Tag"];
                Entity deserializedEntity = m_Scene->CreateEntity(name);

                if (entityJson.contains("Transform"))
                {
                    auto& tJson = entityJson["Transform"];
                    if (tJson.is_object()) {
                        auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                        tc.X = tJson.value("X", 0.0f);
                        tc.Y = tJson.value("Y", 0.0f);
                        tc.Rotation = tJson.value("Rotation", 0.0f);
                        tc.ScaleX = tJson.value("ScaleX", 1.0f);
                        tc.ScaleY = tJson.value("ScaleY", 1.0f);
                    }
                    else {
                        AETHER_CORE_ERROR("Transform found but is NOT an object!");
                    }
                }

                if (entityJson.contains("Sprite"))
                {
                    auto& sJson = entityJson["Sprite"];
                    if (sJson.is_object()) {
                        auto& sc = deserializedEntity.AddComponent<SpriteComponent>();
                        sc.R = sJson.value("R", 1.0f);
                        sc.G = sJson.value("G", 1.0f);
                        sc.B = sJson.value("B", 1.0f);
                        sc.A = sJson.value("A", 1.0f);
                    }
                    else {
                        AETHER_CORE_ERROR("Sprite found but is NOT an object!");
                    }
                }

                if (entityJson.contains("CameraComponent")) {
                    auto& cJson = entityJson["CameraComponent"];
                    if (cJson.is_object()) {
                        auto& cc = deserializedEntity.AddComponent<CameraComponent>();
                        cc.Size = cJson.value("Size", 720.0f);
                        cc.Near = cJson.value("Near", -1.0f);
                        cc.Far = cJson.value("Far", 1.0f);
                        cc.Primary = cJson.value("Primary", true);
                        AETHER_CORE_INFO("Loaded Camera: Size={0}, Primary={1}", cc.Size, cc.Primary);
                    }
                    else {
                        AETHER_CORE_ERROR("CameraComponent found but is NOT an object!");
                    }
                }
            }
        }
        AETHER_CORE_INFO("Deserialized Scene from '{0}'", filepath);
    }
}