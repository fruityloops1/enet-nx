/**
 @file  nx.h
 @brief ENet NX (NintendoSDK) header
*/
#ifndef __ENET_NX_H__
#define __ENET_NX_H__

#include <stddef.h>

typedef int ENetSocket;

#define ENET_SOCKET_NULL -1

#define ENET_HOST_TO_NET_16(value) (nnsocketInetHtons(value)) /**< macro that converts host to net byte-order of a 16-bit value */
#define ENET_HOST_TO_NET_32(value) (nnsocketInetHtonl(value)) /**< macro that converts host to net byte-order of a 32-bit value */

#define ENET_NET_TO_HOST_16(value) (nnsocketInetNtohs(value)) /**< macro that converts net to host byte-order of a 16-bit value */
#define ENET_NET_TO_HOST_32(value) (nnsocketInetNtohl(value)) /**< macro that converts net to host byte-order of a 32-bit value */

typedef struct
{
    void* data;
    size_t dataLength;
} ENetBuffer;

#define ENET_CALLBACK

#define ENET_API extern

typedef void ENetSocketSet;

#endif /* __ENET_NX_H__ */
