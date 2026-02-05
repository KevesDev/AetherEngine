#pragma once

#include <cstdint>

namespace aether {

    using SessionID = std::uint64_t;

    enum class SessionState : std::uint8_t {
        Disconnected = 0,
        Authenticating,
        Active
    };

    struct SessionInfo {
        SessionID ID = 0;
        SessionState State = SessionState::Disconnected;
        std::uint32_t PlayerIndex = 0; // Maps to PlayerControllerComponent
    };

} // namespace aether

