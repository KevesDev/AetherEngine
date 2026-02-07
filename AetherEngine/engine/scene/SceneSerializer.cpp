#include "SceneSerializer.h"

#include "../ecs/Entity.h"
#include "../ecs/Components.h"
#include "../core/Log.h"
#include "../core/UUID.h"

#include "../vendor/json.hpp"

#include <fstream>
#include <filesystem>
#include <sstream>
#include <exception>

namespace aether {

    using json = nlohmann::json;

    SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene)
        : m_Scene(scene)
    {
    }

    /**
     * Serializes a single entity and its components to a JSON object.
     * Note: Entities must possess an IDComponent to be considered persistent.
     */
    static void SerializeEntity(json& out, Entity entity)
    {
        // 1. Identity (Mandatory for persistence)
        // need to use uint64_t to avoid JSON number precision issues
        if (entity.HasComponent<IDComponent>()) {
            out["EntityID"] = (uint64_t)entity.GetComponent<IDComponent>().ID;
        }
        else {
            return; // Transient entity, skip.
        }

        // 2. Metadata
        if (entity.HasComponent<TagComponent>()) {
            out["TagComponent"] = {
                { "Tag", entity.GetComponent<TagComponent>().Tag }
            };
        }

        // 3. Transform (Spatial State)
        if (entity.HasComponent<TransformComponent>()) {
            auto& tc = entity.GetComponent<TransformComponent>();
            out["TransformComponent"] = {
                { "Translation", { tc.X, tc.Y } },
                { "Rotation", tc.Rotation },
                { "Scale", { tc.ScaleX, tc.ScaleY } }
            };
        }

        // 4. Rendering
        if (entity.HasComponent<SpriteComponent>()) {
            auto& sc = entity.GetComponent<SpriteComponent>();
            out["SpriteComponent"] = {
                { "Color", { sc.R, sc.G, sc.B, sc.A } }
            };
        }

        // 5. Camera (Pure Data)
        if (entity.HasComponent<CameraComponent>()) {
            auto& cc = entity.GetComponent<CameraComponent>();
            out["CameraComponent"] = {
                { "ProjectionType", (int)cc.ProjectionType },
                { "PerspectiveFOV", cc.PerspFOV },
                { "PerspectiveNear", cc.PerspNear },
                { "PerspectiveFar", cc.PerspFar },
                { "OrthographicSize", cc.OrthoSize },
                { "OrthographicNear", cc.OrthoNear },
                { "OrthographicFar", cc.OrthoFar },
                { "Primary", cc.Primary },
                { "FixedAspectRatio", cc.FixedAspectRatio }
            };
        }

        // 6. Gameplay / Input
        if (entity.HasComponent<PlayerControllerComponent>()) {
            auto& pcc = entity.GetComponent<PlayerControllerComponent>();
            out["PlayerControllerComponent"] = {
                { "PlayerIndex", pcc.PlayerIndex },
                { "ActiveMappingContext", pcc.ActiveMappingContext }
            };
        }

        // 7. Networking / Replication
        if (entity.HasComponent<ReplicationComponent>()) {
            auto& rc = entity.GetComponent<ReplicationComponent>();
            out["ReplicationComponent"] = {
                { "Mode", static_cast<int>(rc.Mode) },
                { "UpdateRateHz", rc.UpdateRateHz }
            };
        }
    }

    void SceneSerializer::Serialize(const std::filesystem::path& filepath)
    {
        std::ofstream fout(filepath);
        if (fout.is_open()) {
            SerializeToStream(fout);
        }
        else {
            AETHER_CORE_ERROR("SceneSerializer: Could not open file '{0}' for writing.", filepath.string());
        }
    }

    bool SceneSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream fin(filepath);
        if (!fin.is_open()) {
            AETHER_CORE_ERROR("SceneSerializer: Could not open file '{0}' for reading.", filepath.string());
            return false;
        }

        // TRACE: Confirm if we are entering scene deserialization
        AETHER_CORE_TRACE("SceneSerializer: Deserializing scene file '{0}'", filepath.string());

        return DeserializeFromStream(fin);
    }

    void SceneSerializer::SerializeToStream(std::ostream& output)
    {
        json out = json::object();
        out["Scene"] = "Untitled";

        out["Systems"] = m_Scene->GetSystemConfigs();
        out["Entities"] = json::array();

        auto view = m_Scene->GetRegistry().view<IDComponent>();
        for (auto entityID : view)
        {
            Entity entity = { entityID, &m_Scene->GetRegistry() };
            if (!entity) continue;

            json e = json::object();
            SerializeEntity(e, entity);

            if (e.contains("EntityID")) {
                out["Entities"].push_back(e);
            }
        }

        output << out.dump(4);
    }

    bool SceneSerializer::DeserializeFromStream(std::istream& input)
    {
        json data;
        try {
            input >> data;
        }
        // Catch std::exception (including std::length_error)
        catch (const std::exception& e) {
            AETHER_CORE_ERROR("SceneSerializer: JSON Parse Error: {0}", e.what());
            return false;
        }

        // 1. Load System Configuration
        if (data.contains("Systems") && data["Systems"].is_array()) {
            for (auto& sys : data["Systems"]) {
                m_Scene->AddSystemConfig(sys.get<std::string>());
            }
        }

        // 2. Load Entities
        auto entities = data["Entities"];
        if (entities.is_array()) {
            for (auto& entityJson : entities) {
                uint64_t uuidInt = entityJson.value("EntityID", 0);
                if (uuidInt == 0) continue;

                std::string name;
                if (!entityJson["TagComponent"].is_null())
                    name = entityJson["TagComponent"]["Tag"];

                Entity deserializedEntity = m_Scene->CreateEntityWithUUID(UUID(uuidInt), name);

                if (!entityJson["TransformComponent"].is_null()) {
                    auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                    auto& t = entityJson["TransformComponent"];
                    tc.X = t["Translation"][0]; tc.Y = t["Translation"][1];
                    tc.Rotation = t["Rotation"];
                    tc.ScaleX = t["Scale"][0]; tc.ScaleY = t["Scale"][1];
                    tc.PrevX = tc.X; tc.PrevY = tc.Y; tc.PrevRotation = tc.Rotation;
                }

                if (!entityJson["SpriteComponent"].is_null()) {
                    auto& sc = deserializedEntity.AddComponent<SpriteComponent>();
                    auto& c = entityJson["SpriteComponent"]["Color"];
                    sc.R = c[0]; sc.G = c[1]; sc.B = c[2]; sc.A = c[3];
                }

                if (!entityJson["CameraComponent"].is_null()) {
                    auto& cc = deserializedEntity.AddComponent<CameraComponent>();
                    auto& c = entityJson["CameraComponent"];
                    cc.ProjectionType = (CameraComponent::Type)c["ProjectionType"];
                    cc.PerspFOV = c["PerspectiveFOV"];
                    cc.PerspNear = c["PerspectiveNear"];
                    cc.PerspFar = c["PerspectiveFar"];
                    cc.OrthoSize = c["OrthographicSize"];
                    cc.OrthoNear = c["OrthographicNear"];
                    cc.OrthoFar = c["OrthographicFar"];
                    cc.Primary = c["Primary"];
                    cc.FixedAspectRatio = c["FixedAspectRatio"];
                }

                if (!entityJson["PlayerControllerComponent"].is_null()) {
                    auto& pcc = deserializedEntity.AddComponent<PlayerControllerComponent>();
                    pcc.PlayerIndex = entityJson["PlayerControllerComponent"]["PlayerIndex"];
                    pcc.ActiveMappingContext = entityJson["PlayerControllerComponent"]["ActiveMappingContext"];
                }

                if (!entityJson["ReplicationComponent"].is_null()) {
                    auto& rc = deserializedEntity.AddComponent<ReplicationComponent>();
                    auto& r = entityJson["ReplicationComponent"];
                    int mode = r.value("Mode", static_cast<int>(ReplicationMode::Frequent));
                    rc.Mode = static_cast<ReplicationMode>(mode);
                    rc.UpdateRateHz = r.value("UpdateRateHz", 20.0f);
                    rc.Accumulator = 0.0f;
                }
            }
        }

        return true;
    }
}