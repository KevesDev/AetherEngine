#include "DeleteAssetCommand.h"
#include "../../engine/core/Log.h"
#include "../../engine/project/Project.h"
#include "../../engine/asset/AssetManager.h"
#include "../../engine/asset/AssetMetadata.h" // Required for AssetHeader
#include "../../engine/vendor/json.hpp"       // Required for parsing metadata
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>

using json = nlohmann::json;

namespace aether {

    DeleteAssetCommand::DeleteAssetCommand(const std::filesystem::path& assetPath)
    {
        // 1. Resolve Asset Path relative to Project
        m_AssetOriginalPath = Project::GetAssetDirectory() / assetPath;

        // 2. Determine Trash Path
        m_AssetTrashPath = GenerateTrashPath(m_AssetOriginalPath);

        // 3. Resolve Source File from Metadata
        // We do strictly verify the file exists before claiming we have a source file.
        if (std::filesystem::exists(m_AssetOriginalPath))
        {
            std::ifstream stream(m_AssetOriginalPath, std::ios::binary);
            if (stream)
            {
                // A. Skip the Binary Header
                AssetHeader header;
                stream.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

                // B. Parse the JSON Metadata
                json meta;
                try {
                    stream >> meta;

                    // C. Check for "Source" field (Standard for Texture2D/Audio/etc)
                    if (meta.contains("Source"))
                    {
                        std::string sourceRel = meta["Source"];
                        if (!sourceRel.empty())
                        {
                            m_SourceOriginalPath = Project::GetAssetDirectory() / sourceRel;

                            // Only mark for deletion if it actually exists on disk
                            if (std::filesystem::exists(m_SourceOriginalPath)) {
                                m_HasSourceFile = true;
                                m_SourceTrashPath = GenerateTrashPath(m_SourceOriginalPath);
                            }
                        }
                    }
                }
                catch (const json::parse_error&) {
                    // If JSON fails, we might be dealing with a corrupted file. 
                    // We proceed with deleting just the .aeth file to allow cleanup.
                    AETHER_CORE_WARN("DeleteAssetCommand: Failed to parse metadata for {}", m_AssetOriginalPath.string());
                }
            }
        }
    }

    std::filesystem::path DeleteAssetCommand::GenerateTrashPath(const std::filesystem::path& originalPath)
    {
        std::filesystem::path trashDir = Project::GetProjectDirectory() / ".trash";
        if (!std::filesystem::exists(trashDir)) {
            std::filesystem::create_directories(trashDir);
        }

        // Create a unique name: Filename_Timestamp.ext
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << originalPath.stem().string() << "_" << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S") << originalPath.extension().string();

        return trashDir / ss.str();
    }

    bool DeleteAssetCommand::Execute()
    {
        bool success = true;

        // 1. Move Asset File (.aeth)
        if (std::filesystem::exists(m_AssetOriginalPath)) {
            try {
                std::filesystem::rename(m_AssetOriginalPath, m_AssetTrashPath);
                AETHER_CORE_INFO("Soft Deleted Asset: {}", m_AssetOriginalPath.filename().string());
            }
            catch (std::filesystem::filesystem_error& e) {
                AETHER_CORE_ERROR("Delete Failed (Asset): {}", e.what());
                success = false;
            }
        }

        // 2. Move Source File (Safe logic using Metadata)
        if (m_HasSourceFile && std::filesystem::exists(m_SourceOriginalPath)) {
            try {
                std::filesystem::rename(m_SourceOriginalPath, m_SourceTrashPath);
                AETHER_CORE_INFO("Soft Deleted Source: {}", m_SourceOriginalPath.filename().string());
            }
            catch (std::filesystem::filesystem_error& e) {
                AETHER_CORE_ERROR("Delete Failed (Source): {}", e.what());
                success = false;
            }
        }

        return success;
    }

    void DeleteAssetCommand::Undo()
    {
        // 1. Restore Asset File
        if (std::filesystem::exists(m_AssetTrashPath)) {
            try {
                std::filesystem::rename(m_AssetTrashPath, m_AssetOriginalPath);
                AETHER_CORE_INFO("Restored Asset: {}", m_AssetOriginalPath.filename().string());
            }
            catch (std::filesystem::filesystem_error& e) {
                AETHER_CORE_ERROR("Undo Failed (Asset): {}", e.what());
            }
        }

        // 2. Restore Source File
        if (m_HasSourceFile && std::filesystem::exists(m_SourceTrashPath)) {
            try {
                std::filesystem::rename(m_SourceTrashPath, m_SourceOriginalPath);
                AETHER_CORE_INFO("Restored Source: {}", m_SourceOriginalPath.filename().string());
            }
            catch (std::filesystem::filesystem_error& e) {
                AETHER_CORE_ERROR("Undo Failed (Source): {}", e.what());
            }
        }
    }
}