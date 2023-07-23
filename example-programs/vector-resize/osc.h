#ifndef OSC_H
#define OSC_H


#include <cstdint>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>

class UdpClient
{
    int fd;

public:
    UdpClient(uint32_t address = htonl(0x7F000001), uint16_t port = htons(9000))
    {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == -1)
        {
            perror("UdpClient:socket");
            exit(1);
        }

        struct sockaddr_in addr
                {
                        .sin_family = AF_INET,
                        .sin_port = port,
                        .sin_addr = { address },
                };

        if (0 != connect(fd, (const struct sockaddr*)&addr, sizeof(addr)))
        {
            perror("UdpClient:connect");
            exit(1);
        }
    }

    void send(const void* ptr, size_t len)
    {
        ssize_t res = ::send(fd, ptr, len, MSG_DONTWAIT);
        if (res != len && (res != -1 || errno != ECONNREFUSED))
        {
            perror("UdpClient:send");
            exit(1);
        }
    }

    ~UdpClient()
    {
        close(fd);
    }
};

static UdpClient udp;

#define EVENT_PACKET(address) ((sizeof(address) % 4 == 1) ? (address "\0\0\0\0,\0\0\0") : (sizeof(address) % 4 == 2) ? (address "\0\0\0,\0\0\0") : (sizeof(address) % 4 == 3) ? (address "\0\0,\0\0\0") : (address "\0,\0\0\0"))
#define SEND_EVENT(address) udp.send(EVENT_PACKET(address), sizeof(address) - 1 + 8 - ((sizeof(address) - 1) % 4))

#endif // OSC_H
