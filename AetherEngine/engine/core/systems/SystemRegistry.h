#pragma once

#include "System.h"
#include "../Log.h"

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>

namespace aether {

    using SystemFactory = std::function<std::unique_ptr<ISystem>()>;

    /**
     * SystemRegistry
     * * A thread-safe(ish) singleton factory for registering and instantiating Engine Systems by string name.
     * This decouples the Scene (Data) from the Systems (Logic), facilitating:
     * 1. Data-Driven Scene Loading (Scenes define which systems they need in .aeth files)
     * 2. Circular Dependency Resolution (Scene.cpp doesn't need to include InputSystem.h)
     * TODO: Look into more thread-safe method. This one isn't perfect.
     */
    class SystemRegistry {
    public:
        /**
         * Registers a System type T under a string key.
         * Must be called during Engine initialization.
         * @param name The unique identifier for the system (e.g., "InputSystem")
         */
        template<typename T>
        static void Register(const std::string& name) {
            GetFactories()[name] = []() { return std::make_unique<T>(); };
        }

        /**
         * Creates a new instance of the requested system.
         * @param name The unique identifier of the system to create.
         * @return A unique_ptr to the new system, or nullptr if not found.
         */
        static std::unique_ptr<ISystem> Create(const std::string& name) {
            auto& factories = GetFactories();
            if (factories.find(name) != factories.end()) {
                return factories[name]();
            }
            AETHER_CORE_ERROR("SystemRegistry: Attempted to create unknown system '{0}'", name);
            return nullptr;
        }

    private:
        // Static singleton map storage
        static std::unordered_map<std::string, SystemFactory>& GetFactories() {
            static std::unordered_map<std::string, SystemFactory> s_Factories;
            return s_Factories;
        }
    };
}