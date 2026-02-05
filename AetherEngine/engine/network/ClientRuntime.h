#pragma once

#include "NetworkTransport.h"
#include "ClientSnapshotBuffer.h"

namespace aether {

    /**
     * ClientRuntime
     *
     * Consumes packets from INetworkTransport, parses NetworkMessageHeader,
     * and stores authoritative snapshots in a ClientSnapshotBuffer.
     * Higher-level systems use this buffer for interpolation and, later,
     * prediction/reconciliation.
     */
    class ClientRuntime {
    public:
        ClientRuntime(INetworkTransport* transport, std::size_t snapshotCapacity = 64)
            : m_Transport(transport)
            , m_Snapshots(snapshotCapacity) {}

        void Update() {
            if (!m_Transport)
                return;

            ReceivedPacket packet{};
            while (m_Transport->Poll(packet)) {
                if (!packet.Payload.Data || packet.Payload.Size < sizeof(NetworkMessageHeader))
                    continue;

                const auto* header = reinterpret_cast<const NetworkMessageHeader*>(packet.Payload.Data);
                const std::uint8_t* payloadStart = packet.Payload.Data + sizeof(NetworkMessageHeader);
                std::size_t payloadSize = packet.Payload.Size - sizeof(NetworkMessageHeader);

                m_Snapshots.Push(*header, payloadStart, payloadSize);
            }
        }

        const ClientSnapshotBuffer& GetSnapshots() const { return m_Snapshots; }

    private:
        INetworkTransport* m_Transport = nullptr;
        ClientSnapshotBuffer m_Snapshots;
    };

} // namespace aether

