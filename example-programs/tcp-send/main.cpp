#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
    printf("Own pid: %u\n", getpid());

    getchar();

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(!fd)
    {
        perror("socket");
        exit(1);
    }

    getchar();

    sockaddr_in addr{};
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = htonl(0x578E48F4);
    addr.sin_family = AF_INET;

    if(0 != connect(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)))
    {
        perror("connect");
        exit(2);
    }

    getchar();

    if(13 != send(fd, "GET /seafile\n", 13, 0))
    {
        perror("write");
        exit(3);
    }

    getchar();
}
