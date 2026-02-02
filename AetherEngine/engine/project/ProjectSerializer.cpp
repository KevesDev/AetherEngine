#include "ProjectSerializer.h"
#include "../core/Log.h"
#include "../vendor/json.hpp"
#include "../core/EngineVersion.h"
#include "Project.h" 
#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace aether {

    ProjectSerializer::ProjectSerializer(std::shared_ptr<Project> project)
        : m_Project(project)
    {
    }

    bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
    {
        // Access member directly (Friend Access)
        const auto& config = m_Project->m_Config;

        if (!Project::IsValidName(config.Name)) {
            AETHER_CORE_ERROR("ProjectSerializer: Cannot save project with invalid name: '{0}'", config.Name);
            return false;
        }

        json out;
        out["Project"] = {
            { "Name", config.Name },
            { "StartScene", config.StartScene },
            { "AssetDirectory", config.AssetDirectory.string() },
            { "ScriptModulePath", config.ScriptModulePath.string() },
            { "EngineVersion", EngineVersion::ToString() } // Write Version
        };

        std::ofstream fout(filepath);
        if (!fout) {
            AETHER_CORE_ERROR("ProjectSerializer: Could not open file for writing: {0}", filepath.string());
            return false;
        }

        fout << out.dump(4);
        fout.close();
        return true;
    }

    bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream stream(filepath);
        if (!stream) {
            AETHER_CORE_ERROR("ProjectSerializer: Could not open file: {0}", filepath.string());
            return false;
        }

        std::stringstream strStream;
        strStream << stream.rdbuf();

        try {
            json data = json::parse(strStream.str());
            auto& projectData = data["Project"];

            // Version Check
            std::string projVersion = projectData.value("EngineVersion", "");
            if (projVersion != EngineVersion::ToString()) {
                AETHER_CORE_ERROR("Project Load Failed: Version Mismatch. Project: '{0}', Engine: '{1}'", projVersion, EngineVersion::ToString());
                return false;
            }

            // Access member directly (Friend Access)
            auto& config = m_Project->m_Config;

            config.Name = projectData.value("Name", "Untitled");
            config.StartScene = projectData.value("StartScene", "");
            config.AssetDirectory = projectData.value("AssetDirectory", "Assets");
            config.ScriptModulePath = projectData.value("ScriptModulePath", "Scripts/Binaries");
        }
        catch (json::exception& e) {
            AETHER_CORE_ERROR("ProjectSerializer: JSON Parsing Error: {0}", e.what());
            return false;
        }

        return true;
    }

    // Helper for UI to peek at version
    bool ProjectSerializer::GetProjectVersion(const std::filesystem::path& filepath, std::string& outVersion)
    {
        std::ifstream stream(filepath);
        if (!stream) return false;

        try {
            json data = json::parse(stream);
            if (data.contains("Project") && data["Project"].contains("EngineVersion")) {
                outVersion = data["Project"]["EngineVersion"];
                return true;
            }
        }
        catch (...) { return false; }

        return false;
    }
}