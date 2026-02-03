#include "SceneSerializer.h"
#include "../ecs/Entity.h"
#include "../ecs/Components.h"
#include "../core/VFS.h"
#include "../core/Log.h"
#include "../asset/AssetMetadata.h" // Required for AssetHeader

// Vendor Include
#include "../vendor/json.hpp" 
#include <fstream>
#include <sstream>

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

        // 4. Camera Component
        if (entity.HasComponent<CameraComponent>()) {
            auto& cc = entity.GetComponent<CameraComponent>();
            outJson["CameraComponent"] = {
                { "ProjectionType", (int)cc.ProjectionType },
                { "PerspectiveFOV", cc.PerspectiveFOV },
                { "PerspectiveNear", cc.PerspectiveNear },
                { "PerspectiveFar", cc.PerspectiveFar },
                { "OrthographicSize", cc.OrthographicSize },
                { "OrthographicNear", cc.OrthographicNear },
                { "OrthographicFar", cc.OrthographicFar },
                { "Primary", cc.Primary },
                { "FixedAspectRatio", cc.FixedAspectRatio }
            };
        }

        // 5. Relationship Component
        if (entity.HasComponent<RelationshipComponent>()) {
            auto& rc = entity.GetComponent<RelationshipComponent>();
            outJson["Relationship"] = {
                { "Parent", rc.Parent },
                { "FirstChild", rc.FirstChild },
                { "NextSibling", rc.NextSibling },
                { "PrevSibling", rc.PreviousSibling },
                { "ChildrenCount", rc.ChildrenCount }
            };
        }
    }

    void SceneSerializer::Serialize(const std::string& filepath)
    {
        json sceneJson;
        sceneJson["Scene"] = "Untitled";
        sceneJson["Entities"] = json::array();

        Registry& registry = m_Scene->GetRegistry();

        auto& tags = registry.View<TagComponent>();
        auto& ownerMap = registry.GetOwnerMap<TagComponent>();

        for (size_t i = 0; i < tags.size(); i++)
        {
            EntityID id = ownerMap.at(i);

            // Pass the Registry pointer
            Entity entity(id, &registry);

            if (!entity) continue;

            json entityJson;
            SerializeEntity(entityJson, entity);
            sceneJson["Entities"].push_back(entityJson);
        }

        std::string jsonDump = sceneJson.dump(4); // Pretty print

        // --- SECURITY: Write Binary Header First ---
        std::ofstream fout(filepath, std::ios::binary);
        if (!fout)
        {
            AETHER_CORE_ERROR("SceneSerializer: Could not create file '{}'", filepath);
            return;
        }

        AssetHeader header;
        header.Type = AssetType::Scene; // Validates this file as a Scene

        // 1. Write the Header
        fout.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

        // 2. Write the JSON body
        fout.write(jsonDump.c_str(), jsonDump.size());

        fout.close();
        AETHER_CORE_INFO("Serialized Scene (Secured) to '{0}'", filepath);
    }

    void SceneSerializer::Deserialize(const std::string& filepath)
    {
        std::ifstream stream(filepath, std::ios::binary);
        if (!stream)
        {
            AETHER_CORE_ERROR("SceneSerializer: Could not open file '{}'", filepath);
            return;
        }

        // --- SECURITY: Verify Header ---
        AssetHeader header;
        stream.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

        bool isValid = true;
        if (stream.gcount() != sizeof(AssetHeader)) isValid = false;

        // Check Magic "AETH"
        if (header.Magic[0] != 'A' || header.Magic[1] != 'E' ||
            header.Magic[2] != 'T' || header.Magic[3] != 'H') isValid = false;

        // Check Type
        if (header.Type != AssetType::Scene) isValid = false;

        if (!isValid)
        {
            AETHER_CORE_ERROR("SceneSerializer: Security Violation - Invalid or Corrupted Scene File: '{}'", filepath);
            return;
        }

        // Read the rest of the file (JSON body)
        std::stringstream ss;
        ss << stream.rdbuf();

        json sceneJson;
        try {
            sceneJson = json::parse(ss.str());
        }
        catch (json::parse_error& e) {
            AETHER_CORE_ERROR("SceneSerializer: Failed to parse JSON body: {}", e.what());
            return;
        }

        auto entities = sceneJson["Entities"];
        if (entities.is_array())
        {
            for (auto& entityJson : entities) {
                std::string name = entityJson["Tag"];
                Entity deserializedEntity = m_Scene->CreateEntity(name);

                // Transform
                if (entityJson.contains("Transform"))
                {
                    auto& tJson = entityJson["Transform"];
                    if (tJson.is_object()) {
                        auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                        tc.X = tJson.value("X", 0.0f);
                        tc.Y = tJson.value("Y", 0.0f);
                        tc.Rotation = tJson.value("Rotation", 0.0f);
                        tc.ScaleX = tJson.value("ScaleX", 100.0f);
                        tc.ScaleY = tJson.value("ScaleY", 100.0f);
                    }
                }

                // Sprite
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
                }

                // Camera
                if (entityJson.contains("CameraComponent")) {
                    auto& cJson = entityJson["CameraComponent"];
                    if (cJson.is_object()) {
                        auto& cc = deserializedEntity.AddComponent<CameraComponent>();
                        cc.ProjectionType = (CameraComponent::Type)cJson.value("ProjectionType", (int)CameraComponent::Type::Orthographic);
                        cc.PerspectiveFOV = cJson.value("PerspectiveFOV", glm::radians(45.0f));
                        cc.PerspectiveNear = cJson.value("PerspectiveNear", 0.01f);
                        cc.PerspectiveFar = cJson.value("PerspectiveFar", 1000.0f);
                        cc.OrthographicSize = cJson.value("OrthographicSize", 10.0f);
                        cc.OrthographicNear = cJson.value("OrthographicNear", -1.0f);
                        cc.OrthographicFar = cJson.value("OrthographicFar", 1.0f);
                        cc.Primary = cJson.value("Primary", true);
                        cc.FixedAspectRatio = cJson.value("FixedAspectRatio", false);
                    }
                }

                // Hierarchy
                if (entityJson.contains("Relationship")) {
                    auto& rJson = entityJson["Relationship"];
                    if (rJson.is_object()) {
                        auto& rc = deserializedEntity.AddComponent<RelationshipComponent>();
                        rc.Parent = rJson.value("Parent", (EntityID)NULL_ENTITY);
                        rc.FirstChild = rJson.value("FirstChild", (EntityID)NULL_ENTITY);
                        rc.NextSibling = rJson.value("NextSibling", (EntityID)NULL_ENTITY);
                        rc.PreviousSibling = rJson.value("PrevSibling", (EntityID)NULL_ENTITY);
                        rc.ChildrenCount = rJson.value("ChildrenCount", (size_t)0);
                    }
                }
            }
        }
        AETHER_CORE_INFO("Deserialized Scene (Verified) from '{0}'", filepath);
    }
}