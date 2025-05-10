/**
 @file  nx.c
 @brief ENet NX (NintendoSDK) system specific functions
*/

#include <cstring>

#include <errno.h>

#include <fcntl.h>

#include "nn/os.h"
#include "nn/socket.h"
#include "nn/time.h"
#include <sys/time.h>

#define ENET_BUILDING_LIB 1
#include "enet/enet.h"

#ifdef __cplusplus
extern "C" {
#endif

static nn::os::Tick timeBase = 0;
const nn::os::Tick ticksPerMillisecond = 19200000ULL / 1000ULL;

int enet_initialize(void)
{
    timeBase = nn::os::GetSystemTick();

    return 0;
}

void enet_deinitialize(void)
{
}

enet_uint32
enet_host_random_seed(void)
{
    nn::time::PosixTime posixTime;
    nn::time::StandardUserSystemClock::GetCurrentTime(&posixTime);

    return (enet_uint32)posixTime.time;
}

enet_uint32
enet_time_get(void)
{
    nn::os::Tick elapsedTicks = (nn::os::GetSystemTick() - timeBase) / ticksPerMillisecond;

    return (enet_uint32)(elapsedTicks);
}

void enet_time_set(enet_uint32 newTimeBase)
{
    // Unsupported
    return;
}

int enet_address_set_host_ip(ENetAddress* address, const char* name)
{
    if (!nn::socket::InetAton(name, (in_addr*)&address->host)) {
        return -1;
    }

    return 0;
}

int enet_address_set_host(ENetAddress* address, const char* name)
{
    // Unsupported
    return -1;
}

int enet_address_get_host_ip(const ENetAddress* address, char* name, size_t nameLength)
{
    // Unsupported
    return -1;
}

int enet_address_get_host(const ENetAddress* address, char* name, size_t nameLength)
{
    // Unsupported
    return -1;
}

int enet_socket_bind(ENetSocket socket, const ENetAddress* address)
{
    // Unsupported
    return -1;
}

int enet_socket_get_address(ENetSocket socket, ENetAddress* address)
{
    // Unsupported
    return -1;
}

int enet_socket_listen(ENetSocket socket, int backlog)
{
    // Unsupported
    return -1;
}

ENetSocket
enet_socket_create(ENetSocketType type)
{
    return nn::socket::Socket(PF_INET, type == ENET_SOCKET_TYPE_DATAGRAM ? SOCK_DGRAM : SOCK_STREAM, 0);
}

int enet_socket_set_option(ENetSocket socket, ENetSocketOption option, int value)
{
    int result = -1;
    switch (option) {
    case ENET_SOCKOPT_NONBLOCK:
        // result = nn::socket::Fcntl(socket, F_SETFL, (value ? O_NONBLOCK : 0) | (nn::socket::Fcntl(socket, F_GETFL) & ~O_NONBLOCK));
        result = 0;
        break;

    case ENET_SOCKOPT_BROADCAST:
        result = nn::socket::SetSockOpt(socket, SOL_SOCKET, SO_BROADCAST, (void*)&value, sizeof(int));
        break;

    case ENET_SOCKOPT_REUSEADDR:
        result = nn::socket::SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, (void*)&value, sizeof(int));
        break;

    case ENET_SOCKOPT_RCVBUF:
        result = nn::socket::SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, (void*)&value, sizeof(int));
        break;

    case ENET_SOCKOPT_SNDBUF:
        result = nn::socket::SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, (void*)&value, sizeof(int));
        break;

    case ENET_SOCKOPT_RCVTIMEO: {
        struct timeval timeVal;
        timeVal.tv_sec = value / 1000;
        timeVal.tv_usec = (value % 1000) * 1000;
        result = nn::socket::SetSockOpt(socket, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeVal, sizeof(struct timeval));
        break;
    }

    case ENET_SOCKOPT_SNDTIMEO: {
        struct timeval timeVal;
        timeVal.tv_sec = value / 1000;
        timeVal.tv_usec = (value % 1000) * 1000;
        result = nn::socket::SetSockOpt(socket, SOL_SOCKET, SO_SNDTIMEO, (void*)&timeVal, sizeof(struct timeval));
        break;
    }

    case ENET_SOCKOPT_NODELAY:
        result = nn::socket::SetSockOpt(socket, IPPROTO_TCP, TCP_NODELAY, (void*)&value, sizeof(int));
        break;

    default:
        break;
    }
    return result == -1 ? -1 : 0;
}

int enet_socket_get_option(ENetSocket socket, ENetSocketOption option, int* value)
{
    int result = -1;
    socklen_t len;
    switch (option) {
    case ENET_SOCKOPT_ERROR:
        len = sizeof(int);
        result = nn::socket::GetSockOpt(socket, SOL_SOCKET, SO_ERROR, value, &len);
        break;

    default:
        break;
    }
    return result == -1 ? -1 : 0;
}

int enet_socket_connect(ENetSocket socket, const ENetAddress* address)
{
    sockaddr sin;
    int result;

    memset(&sin, 0, sizeof(sockaddr));

    sin.family = AF_INET;
    sin.port = ENET_HOST_TO_NET_16(address->port);
    sin.address.data = address->host;

    result = nn::socket::Connect(socket, &sin, sizeof(sockaddr));
    if (result == -1 && nn::socket::GetLastErrno() == EINPROGRESS)
        return 0;

    return result;
}

ENetSocket
enet_socket_accept(ENetSocket socket, ENetAddress* address)
{
    // Unsupported
    return -1;
}

int enet_socket_shutdown(ENetSocket socket, ENetSocketShutdown how)
{
    return nn::socket::Shutdown(socket, how);
}

void enet_socket_destroy(ENetSocket socket)
{
    if (socket != -1) {
        nn::socket::Close(socket);
    }
}

int enet_socket_send(ENetSocket socket,
    const ENetAddress* address,
    const ENetBuffer* buffers,
    size_t bufferCount)
{
    sockaddr sin;
    int sentLength;

    if (address != NULL) {
        memset(&sin, 0, sizeof(sockaddr));

        sin.family = AF_INET;
        sin.port = ENET_HOST_TO_NET_16(address->port);
        sin.address.data = address->host;
    }

    // This API uses msgsend() on unix - yuzu doesn't support this as of now.
    // Because there may be more than one buffer, we have to concatenate them
    // into one big buffer before we can pass it to sendto.

    int totalSize = 0;
    for (size_t i = 0; i < bufferCount; i++) {
        totalSize += (buffers + i)->dataLength;
    }

    void* data = enet_malloc(totalSize);

    char* currentPtr = (char*)data;
    for (size_t i = 0; i < bufferCount; i++) {
        const ENetBuffer* buffer = buffers + i;

        memcpy(currentPtr, buffer->data, buffer->dataLength);
        currentPtr += buffer->dataLength;
    }

    sentLength = nn::socket::SendTo(socket, data, totalSize, MSG_DONTWAIT, &sin, sizeof(sockaddr));

    enet_free(data);

    if (sentLength == -1) {
        if (nn::socket::GetLastErrno() == EWOULDBLOCK)
            return 0;

        return -1;
    }

    return sentLength;
}

int enet_socket_receive(ENetSocket socket,
    ENetAddress* address,
    ENetBuffer* buffers,
    size_t bufferCount)
{
    sockaddr sin;
    u32 sin_length = sizeof(sockaddr);
    int recvLength;

    // Buffer count is guaranteed to always be 1 here.
    // See call for enet_socket_receive in protocol.c.
    recvLength = nn::socket::RecvFrom(socket, buffers->data, buffers->dataLength, MSG_DONTWAIT, &sin, &sin_length);

    if (recvLength == -1) {
        if (nn::socket::GetLastErrno() == EWOULDBLOCK)
            return 0;

        return -1;
    }

    if (address != NULL) {
        address->host = (enet_uint32)sin.address.data;
        address->port = ENET_NET_TO_HOST_16(sin.port);
    }

    return recvLength;
}

int enet_socketset_select(ENetSocket maxSocket, ENetSocketSet* readSet, ENetSocketSet* writeSet, enet_uint32 timeout)
{
    // Unsupported
    return -1;
}

int enet_socket_wait(ENetSocket socket, enet_uint32* condition, enet_uint32 timeout)
{
    pollfd pollSocket;
    int pollCount;

    pollSocket.fd = socket;
    pollSocket.events = 0;

    if (*condition & ENET_SOCKET_WAIT_SEND)
        pollSocket.events |= POLLOUT;

    if (*condition & ENET_SOCKET_WAIT_RECEIVE)
        pollSocket.events |= POLLIN;

    pollCount = nn::socket::Poll(&pollSocket, 1, timeout);

    if (pollCount < 0) {
        if (nn::socket::GetLastErrno() == EINTR && *condition & ENET_SOCKET_WAIT_INTERRUPT) {
            *condition = ENET_SOCKET_WAIT_INTERRUPT;

            return 0;
        }

        return -1;
    }

    *condition = ENET_SOCKET_WAIT_NONE;

    if (pollCount == 0)
        return 0;

    if (pollSocket.revents & POLLOUT)
        *condition |= ENET_SOCKET_WAIT_SEND;

    if (pollSocket.revents & POLLIN)
        *condition |= ENET_SOCKET_WAIT_RECEIVE;

    return 0;
}

#ifdef __cplusplus
}
#endif
