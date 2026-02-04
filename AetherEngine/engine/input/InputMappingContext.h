#pragma once

#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>

#include "InputActions.h"
#include "../core/Log.h"
#include "../asset/AssetMetadata.h"
#include "../vendor/json.hpp"

namespace aether {

    using json = nlohmann::json;

    struct FEnhancedActionKeyMapping {
        int KeyCode;
        uint32_t ActionID;
        float Scale = 1.0f;
    };

    class InputMappingContext {
    public:
        InputMappingContext() = default;

        void AddMapping(int keyCode, uint32_t actionID, float scale = 1.0f) {
            m_Mappings.push_back({ keyCode, actionID, scale });
        }

        const std::vector<FEnhancedActionKeyMapping>& GetMappings() const { return m_Mappings; }

        static AssetType GetStaticType() { return AssetType::InputMappingContext; }

        /**
         * Load
         * * Implementation:
         * 1. Opens the file in binary mode.
         * 2. Reads and validates the AssetHeader.
         * 3. Parses the remaining JSON payload.
         */
        static std::shared_ptr<InputMappingContext> Load(const std::filesystem::path& path) {
            auto context = std::make_shared<InputMappingContext>();

            std::ifstream file(path, std::ios::binary);
            if (!file.is_open()) {
                AETHER_CORE_ERROR("InputMappingContext: Failed to open file '{0}'", path.string());
                return nullptr;
            }

            // 1. Skip Binary Header
            // .aeth files are prefixed with an AssetHeader struct. 
            // We must advance the stream past this to reach the JSON data.
            file.seekg(sizeof(AssetHeader), std::ios::beg);

            // 2. Parse JSON Payload
            try {
                json data = json::parse(file);

                if (data.contains("Mappings") && data["Mappings"].is_array()) {
                    for (auto& item : data["Mappings"]) {
                        int keyCode = item.value("KeyCode", 0);
                        uint32_t actionID = item.value("ActionID", 0);
                        float scale = item.value("Scale", 1.0f);

                        if (keyCode != 0) {
                            context->AddMapping(keyCode, actionID, scale);
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                AETHER_CORE_ERROR("InputMappingContext: JSON Parse Error in '{0}': {1}", path.string(), e.what());
                return nullptr;
            }

            return context;
        }

    private:
        std::vector<FEnhancedActionKeyMapping> m_Mappings;
    };
}