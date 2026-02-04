#pragma once
#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include "InputActions.h"
#include "../core/Log.h"
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
        void AddMapping(int keyCode, uint32_t actionID, float scale = 1.0f) {
            m_Mappings.push_back({ keyCode, actionID, scale });
        }

        const std::vector<FEnhancedActionKeyMapping>& GetMappings() const { return m_Mappings; }

        /**
         * Loads the Input Context from disk.
         * Parses JSON to populate mappings.
         */
        static std::shared_ptr<InputMappingContext> Load(const std::filesystem::path& path) {
            auto context = std::make_shared<InputMappingContext>();

            std::ifstream file(path);
            if (!file.is_open()) {
                AE_CORE_ERROR("Failed to load InputMappingContext: {0}", path.string());
                return nullptr;
            }

            try {
                json data = json::parse(file);
                if (data.contains("Mappings")) {
                    for (auto& item : data["Mappings"]) {
                        context->AddMapping(
                            item["KeyCode"],
                            item["ActionID"],
                            item.value("Scale", 1.0f)
                        );
                    }
                }
            }
            catch (const std::exception& e) {
                AE_CORE_ERROR("JSON Parse Error in IMC: {0}", e.what());
                return nullptr;
            }

            return context;
        }

    private:
        std::vector<FEnhancedActionKeyMapping> m_Mappings;
    };
}