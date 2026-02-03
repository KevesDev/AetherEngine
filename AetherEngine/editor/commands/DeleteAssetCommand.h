#pragma once
#include "EditorCommand.h"
#include <filesystem>
#include <string>

namespace aether {

    /**
     * Concrete command to handle safe asset deletion.
     * Implements "Soft Delete" by moving files to a hidden .trash directory
     * inside the Project folder, allowing for full restoration.
     */
    class DeleteAssetCommand : public EditorCommand
    {
    public:
        /**
         * @param assetPath The relative or absolute path to the asset being deleted (e.g., "Assets/Textures/Hero.aeth")
         */
        DeleteAssetCommand(const std::filesystem::path& assetPath);

        virtual bool Execute() override;
        virtual void Undo() override;
        virtual std::string GetName() const override { return "Delete Asset"; }

    private:
        // Helper to generate a unique, timestamped path in the .trash folder
        std::filesystem::path GenerateTrashPath(const std::filesystem::path& originalPath);

    private:
        // --- The Metadata Asset (.aeth) ---
        std::filesystem::path m_AssetOriginalPath;
        std::filesystem::path m_AssetTrashPath;

        // --- The Source Asset (e.g., .png, .obj) ---
        // Not all assets have source files (e.g., Scenes), so we check this flag.
        bool m_HasSourceFile = false;
        std::filesystem::path m_SourceOriginalPath;
        std::filesystem::path m_SourceTrashPath;
    };
}