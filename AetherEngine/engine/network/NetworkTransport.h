#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace aether {

    using ConnectionID = std::uint32_t;

    enum class TransportChannel : std::uint8_t {
        ReliableOrdered = 0,
        UnreliableOrdered = 1,
        Unreliable = 2
    };

    struct Endpoint {
        std::string Address;
        std::uint16_t Port = 0;
    };

    struct PacketView {
        const std::uint8_t* Data = nullptr;
        std::size_t Size = 0;
    };

    struct ReceivedPacket {
        ConnectionID Connection = 0;
        PacketView Payload;
    };

    /**
     * INetworkTransport
     *
     * Platform-agnostic abstraction over low-level networking.
     * Concrete implementations (e.g., UDP) live behind this interface.
     *
     * Responsibilities:
     *  - Connection management (listen/connect/disconnect)
     *  - Sending and receiving opaque byte buffers
     *  - Supporting basic QoS via logical channels
     */
    class INetworkTransport {
    public:
        virtual ~INetworkTransport() = default;

        virtual bool StartServer(const Endpoint& endpoint) = 0;
        virtual bool StartClient(const Endpoint& endpoint) = 0;

        virtual void Shutdown() = 0;

        virtual bool Send(ConnectionID connection,
                          TransportChannel channel,
                          const std::uint8_t* data,
                          std::size_t size) = 0;

        // Polls for received packets. Implementations may batch internally.
        virtual bool Poll(ReceivedPacket& outPacket) = 0;

        virtual void Disconnect(ConnectionID connection) = 0;
    };

} // namespace aether

