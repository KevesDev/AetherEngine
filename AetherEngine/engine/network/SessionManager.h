#pragma once

#include "SessionTypes.h"
#include "../persistence/Database.h"
#include <unordered_map>

namespace aether {

    /**
     * SessionManager
     *
     * Engine-level tracking for authenticated sessions.
     * Uses an injected IDatabase implementation for authentication and
     * persistence, keeping the core logic independent of the DB backend.
     */
    class SessionManager {
    public:
        explicit SessionManager(IDatabase* database)
            : m_Database(database) {}

        SessionInfo* GetSession(SessionID id) {
            auto it = m_Sessions.find(id);
            if (it == m_Sessions.end())
                return nullptr;
            return &it->second;
        }

        SessionInfo& CreateSession(SessionID id) {
            SessionInfo& info = m_Sessions[id];
            info.ID = id;
            info.State = SessionState::Authenticating;
            return info;
        }

        void DestroySession(SessionID id) {
            m_Sessions.erase(id);
        }

        bool Authenticate(SessionID id,
                          const std::string& username,
                          const std::string& passwordHash) {
            if (!m_Database)
                return false;

            SessionInfo* session = GetSession(id);
            if (!session)
                return false;

            if (m_Database->ValidateCredentials(username, passwordHash)) {
                session->State = SessionState::Active;
                return true;
            }

            session->State = SessionState::Disconnected;
            return false;
        }

    private:
        std::unordered_map<SessionID, SessionInfo> m_Sessions;
        IDatabase* m_Database = nullptr;
    };

} // namespace aether

