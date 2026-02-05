#include "UdpTransport.h"

#include "../core/Log.h"

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using SocketHandle = SOCKET;
    static constexpr SocketHandle INVALID_SOCKET_HANDLE = INVALID_SOCKET;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <unistd.h>
    using SocketHandle = int;
    static constexpr SocketHandle INVALID_SOCKET_HANDLE = -1;
#endif

namespace aether {

    namespace {

        bool SetNonBlocking(SocketHandle socket) {
#if defined(_WIN32)
            u_long mode = 1;
            return ioctlsocket(socket, FIONBIO, &mode) == 0;
#else
            int flags = fcntl(socket, F_GETFL, 0);
            if (flags == -1) return false;
            return fcntl(socket, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
        }

        void CloseSocket(SocketHandle socket) {
            if (socket == INVALID_SOCKET_HANDLE)
                return;
#if defined(_WIN32)
            closesocket(socket);
#else
            close(socket);
#endif
        }

    } // namespace

    UdpTransport::UdpTransport() = default;

    UdpTransport::~UdpTransport() {
        Shutdown();
    }

    bool UdpTransport::StartServer(const Endpoint& endpoint) {
        Shutdown();

#if defined(_WIN32)
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            AETHER_CORE_ERROR("UdpTransport: WSAStartup failed.");
            return false;
        }
#endif

        m_Socket = static_cast<int>(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
        if (m_Socket == INVALID_SOCKET_HANDLE) {
            AETHER_CORE_ERROR("UdpTransport: Failed to create UDP socket.");
            return false;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(endpoint.Port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (::bind(m_Socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            AETHER_CORE_ERROR("UdpTransport: Failed to bind UDP socket on port {0}.", endpoint.Port);
            CloseSocket(m_Socket);
            m_Socket = INVALID_SOCKET_HANDLE;
            return false;
        }

        if (!SetNonBlocking(m_Socket)) {
            AETHER_CORE_ERROR("UdpTransport: Failed to set non-blocking mode.");
            CloseSocket(m_Socket);
            m_Socket = INVALID_SOCKET_HANDLE;
            return false;
        }

        m_IsServer = true;
        return true;
    }

    bool UdpTransport::StartClient(const Endpoint& endpoint) {
        Shutdown();

#if defined(_WIN32)
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            AETHER_CORE_ERROR("UdpTransport: WSAStartup failed.");
            return false;
        }
#endif

        m_Socket = static_cast<int>(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
        if (m_Socket == INVALID_SOCKET_HANDLE) {
            AETHER_CORE_ERROR("UdpTransport: Failed to create UDP socket.");
            return false;
        }

        if (!SetNonBlocking(m_Socket)) {
            AETHER_CORE_ERROR("UdpTransport: Failed to set non-blocking mode.");
            CloseSocket(m_Socket);
            m_Socket = INVALID_SOCKET_HANDLE;
            return false;
        }

        static sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(endpoint.Port);
        serverAddr.sin_addr.s_addr = inet_addr(endpoint.Address.c_str());
        m_ServerEndpoint = &serverAddr;

        m_IsServer = false;
        return true;
    }

    void UdpTransport::Shutdown() {
        if (m_Socket != INVALID_SOCKET_HANDLE) {
            CloseSocket(m_Socket);
            m_Socket = INVALID_SOCKET_HANDLE;
        }
        m_Connections.clear();
        m_ServerEndpoint = nullptr;

#if defined(_WIN32)
        WSACleanup();
#endif
    }

    bool UdpTransport::Send(ConnectionID connection,
                            TransportChannel /*channel*/,
                            const std::uint8_t* data,
                            std::size_t size) {
        if (m_Socket == INVALID_SOCKET_HANDLE || !data || size == 0)
            return false;

        sockaddr_in target{};

        if (m_IsServer) {
            auto it = m_Connections.find(connection);
            if (it == m_Connections.end())
                return false;
            target = it->second;
        }
        else {
            if (!m_ServerEndpoint)
                return false;
            target = *m_ServerEndpoint;
        }

        int sent = ::sendto(m_Socket,
                            reinterpret_cast<const char*>(data),
                            static_cast<int>(size),
                            0,
                            reinterpret_cast<sockaddr*>(&target),
                            sizeof(target));

        if (sent < 0) {
            AETHER_CORE_WARN("UdpTransport: sendto failed.");
            return false;
        }

        return true;
    }

    bool UdpTransport::Poll(ReceivedPacket& outPacket) {
        if (m_Socket == INVALID_SOCKET_HANDLE)
            return false;

        sockaddr_in fromAddr{};
#if defined(_WIN32)
        int fromLen = sizeof(fromAddr);
#else
        socklen_t fromLen = sizeof(fromAddr);
#endif

        std::uint8_t buffer[2048];

        int received = ::recvfrom(m_Socket,
                                  reinterpret_cast<char*>(buffer),
                                  static_cast<int>(sizeof(buffer)),
                                  0,
                                  reinterpret_cast<sockaddr*>(&fromAddr),
                                  &fromLen);

        if (received <= 0)
            return false; // No data available (non-blocking)

        ConnectionID id = 0;
        if (m_IsServer) {
            // Simple connection tracking: assign IDs as new remote endpoints appear.
            bool found = false;
            for (const auto& [connId, addr] : m_Connections) {
                if (addr.sin_addr.s_addr == fromAddr.sin_addr.s_addr &&
                    addr.sin_port == fromAddr.sin_port) {
                    id = connId;
                    found = true;
                    break;
                }
            }
            if (!found) {
                id = m_NextConnectionID++;
                m_Connections[id] = fromAddr;
            }
        }
        else {
            id = 1; // Single server connection
        }

        outPacket.Connection = id;
        outPacket.Payload.Data = buffer;
        outPacket.Payload.Size = static_cast<std::size_t>(received);

        return true;
    }

    void UdpTransport::Disconnect(ConnectionID connection) {
        if (m_IsServer) {
            m_Connections.erase(connection);
        }
    }

} // namespace aether

