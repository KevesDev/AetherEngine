#pragma once

#include "NetworkTransport.h"

#include <unordered_map>

// Forward-declare OS-specific socket types to keep this header portable.
struct sockaddr_in;

namespace aether {

    /**
     * UdpTransport
     *
     * UDP-based implementation of INetworkTransport.
     * Socket details are hidden behind this interface so higher layers
     * remain platform-agnostic.
     */
    class UdpTransport final : public INetworkTransport {
    public:
        UdpTransport();
        ~UdpTransport() override;

        bool StartServer(const Endpoint& endpoint) override;
        bool StartClient(const Endpoint& endpoint) override;

        void Shutdown() override;

        bool Send(ConnectionID connection,
                  TransportChannel channel,
                  const std::uint8_t* data,
                  std::size_t size) override;

        bool Poll(ReceivedPacket& outPacket) override;

        void Disconnect(ConnectionID connection) override;

    private:
        int m_Socket = -1;
        bool m_IsServer = false;

        // Server: map logical ConnectionID to remote endpoints.
        std::unordered_map<ConnectionID, sockaddr_in> m_Connections;
        ConnectionID m_NextConnectionID = 1;

        // Client: single remote endpoint.
        sockaddr_in* m_ServerEndpoint = nullptr;
    };

} // namespace aether

