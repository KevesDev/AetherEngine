#pragma once

#include <string>

namespace aether {

    /**
     * IDatabase
     *
     * Abstract interface for the embedded database backend (e.g. SQLite).
     * Concrete implementations manage connection lifecycle and schema.
     */
    class IDatabase {
    public:
        virtual ~IDatabase() = default;

        virtual bool Open(const std::string& path) = 0;
        virtual void Close() = 0;

        // Minimal authentication API. Extended operations (character/world
        // persistence) will be defined alongside concrete game systems.
        virtual bool ValidateCredentials(const std::string& username,
                                         const std::string& passwordHash) = 0;
    };

} // namespace aether

