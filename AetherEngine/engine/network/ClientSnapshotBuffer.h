#pragma once

#include "NetworkTypes.h"
#include <vector>

namespace aether {

    struct SnapshotRecord {
        NetworkMessageHeader Header{};
        std::vector<std::uint8_t> Payload;
    };

    /**
     * ClientSnapshotBuffer
     *
     * Lightweight ring buffer for recent authoritative snapshots.
     * Used by client-side systems to interpolate/extrapolate state.
     */
    class ClientSnapshotBuffer {
    public:
        explicit ClientSnapshotBuffer(std::size_t capacity = 64)
            : m_Capacity(capacity) {}

        void Push(const NetworkMessageHeader& header,
                  const std::uint8_t* data,
                  std::size_t size) {
            if (m_Capacity == 0)
                return;

            SnapshotRecord record;
            record.Header = header;
            record.Payload.assign(data, data + size);

            if (m_Buffer.size() < m_Capacity) {
                m_Buffer.push_back(std::move(record));
            }
            else {
                m_Buffer[m_Head] = std::move(record);
                m_Head = (m_Head + 1) % m_Capacity;
            }
        }

        const std::vector<SnapshotRecord>& GetBuffer() const { return m_Buffer; }

    private:
        std::size_t m_Capacity;
        std::size_t m_Head = 0;
        std::vector<SnapshotRecord> m_Buffer;
    };

} // namespace aether

