#include "ProjectSerializer.h"
#include "../core/Log.h"
#include "../vendor/json.hpp"
#include "../core/EngineVersion.h"
#include "Project.h" 
#include <fstream>
#include <vector>

using json = nlohmann::json;

namespace aether {

    ProjectSerializer::ProjectSerializer(std::shared_ptr<Project> project)
        : m_Project(project)
    {
    }

    bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
    {
        // Direct access via friend class
        const auto& config = m_Project->m_Config;

        // Validation check before save
        if (!Project::IsValidName(config.Name)) {
            AETHER_CORE_ERROR("ProjectSerializer: Cannot save project with invalid name: '{0}'", config.Name);
            return false;
        }

        // 1. Build JSON Object
        json out;
        out["Project"] = {
            { "Name", config.Name },
            { "StartScene", config.StartScene },
            { "AssetDirectory", config.AssetDirectory.string() },
            { "ScriptModulePath", config.ScriptModulePath.string() },
            { "EngineVersion", EngineVersion::ToString() },
            { "ServerPort", config.ServerPort },
            { "MaxPlayers", config.MaxPlayers },
            { "StaticReplicationRateHz", config.StaticReplicationRateHz },
            { "FrequentReplicationRateHz", config.FrequentReplicationRateHz }
        };

        // 2. CONVERT TO BINARY (BSON)
        std::vector<uint8_t> binaryData = json::to_bson(out);

        // 3. Write Binary File
        std::ofstream fout(filepath, std::ios::binary);
        if (!fout) {
            AETHER_CORE_ERROR("ProjectSerializer: Could not open file for writing: {0}", filepath.string());
            return false;
        }

        fout.write(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
        fout.close();

        return true;
    }

    bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream stream(filepath, std::ios::binary);
        if (!stream) {
            AETHER_CORE_ERROR("ProjectSerializer: Could not open file: {0}", filepath.string());
            return false;
        }

        // Read entire file into a binary buffer
        std::vector<uint8_t> binaryData((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        try {
            // 1. PARSE FROM BINARY (BSON)
            json data = json::from_bson(binaryData);
            auto& projectData = data["Project"];

            // Version Check
            std::string projVersion = projectData.value("EngineVersion", "");
            if (projVersion != EngineVersion::ToString()) {
                AETHER_CORE_ERROR("Project Load Failed: Version Mismatch. Project: '{0}', Engine: '{1}'", projVersion, EngineVersion::ToString());
                return false;
            }

            // Direct access via friend class
            auto& config = m_Project->m_Config;

            config.Name = projectData.value("Name", "Untitled");
            config.StartScene = projectData.value("StartScene", "");
            config.AssetDirectory = projectData.value("AssetDirectory", "Assets");
            config.ScriptModulePath = projectData.value("ScriptModulePath", "Scripts/Binaries");
            config.ServerPort = projectData.value("ServerPort", static_cast<uint16_t>(7777));
            config.MaxPlayers = projectData.value("MaxPlayers", static_cast<uint32_t>(64));
            config.StaticReplicationRateHz = projectData.value("StaticReplicationRateHz", 5.0f);
            config.FrequentReplicationRateHz = projectData.value("FrequentReplicationRateHz", 20.0f);
        }
        catch (json::exception& e) {
            AETHER_CORE_ERROR("ProjectSerializer: BSON Parsing Error: {0}. File may be corrupted.", e.what());
            return false;
        }

        return true;
    }

    bool ProjectSerializer::GetProjectVersion(const std::filesystem::path& filepath, std::string& outVersion)
    {
        std::ifstream stream(filepath, std::ios::binary);
        if (!stream) return false;

        std::vector<uint8_t> binaryData((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        try {
            json data = json::from_bson(binaryData);
            if (data.contains("Project") && data["Project"].contains("EngineVersion")) {
                outVersion = data["Project"]["EngineVersion"];
                return true;
            }
        }
        catch (...) { return false; }

        return false;
    }
}