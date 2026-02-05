#pragma once

#include <cstdint>

namespace aether {

    // Common header prepended to all engine-level network messages.
    // Ensures every packet is explicitly tied to a simulation tick.
    struct NetworkMessageHeader {
        std::uint32_t Magic = 0xA37HERu;    // Engine protocol identifier
        std::uint8_t  Version = 1;          // Protocol version
        std::uint8_t  MessageType = 0;      // Application-defined message type
        std::uint16_t Reserved = 0;         // Reserved for future flags
        std::uint64_t SimulationTick = 0;   // Tick from AetherTime::GetSimTick()
    };

} // namespace aether

