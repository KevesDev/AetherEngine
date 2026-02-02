#include "ProjectSerializer.h"
#include "../core/Log.h"
#include "../vendor/json.hpp"
#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace aether {
    /* This implements the JSON parsing. 
    *  It uses strict nlohmann::json lookups to ensure the file is valid.
    */
    ProjectSerializer::ProjectSerializer(std::shared_ptr<Project> project)
        : m_Project(project)
    {
    }

    bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
    {
        const auto& config = m_Project->GetActiveConfig();

        json out;
        out["Project"] = {
            { "Name", config.Name },
            { "StartScene", config.StartScene },
            { "AssetDirectory", config.AssetDirectory.string() },
            { "ScriptModulePath", config.ScriptModulePath.string() }
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
            auto& config = m_Project->GetActiveConfig();

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
}