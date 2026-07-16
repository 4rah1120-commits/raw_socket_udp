#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")


#define DEST_IP "192.168.0.75"
#define DEST_PORT 3000
#define PACKET_LEN 1500

void send_raw_ip_packet(char* buff, int buff_len);
int buff_init();

char sendbuff[PACKET_LEN];

//
//int main()
//{
//    WORD wVersionRequested;
//    WSADATA wsaData;
//
//    wVersionRequested = MAKEWORD(1, 1);
//
//    WSAStartup(wVersionRequested, &wsaData);
//
//    //-----------------------------------
//    int data_len = buff_init();
//    //-----------------------------------------------------------------
//    send_raw_ip_packet(sendbuff, data_len);
//    //-----------------------------------------------------------------
//
//    WSACleanup();
//}


int main(void) {

    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(1, 1);

    WSAStartup(wVersionRequested, &wsaData);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

    char message[64];

    if (sockfd < 0) {
        perror("Error creating socket");
        exit(1);
    }

    struct sockaddr_in sock_in;
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons(3000);
    sock_in.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&sock_in, sizeof(sock_in)) < 0) {
        printf("Bind failed\n");
        exit(1);
    }

    if (recv(sockfd, message, 64, 0) < 0) {
        perror("Error");
        exit(1);
    }

    printf("\n\n%s\n\n", message);

    closesocket(sockfd);

    WSACleanup();

    return 0;
}



void send_raw_ip_packet(char* buff, int buff_len)
{
    struct sockaddr_in dest_info;
    int enable = 1;

    //-----------------------------------------------------------------
    // Step 1: Create a raw network socket.
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    // int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0)
    {
        printf("socket failed: %d\n", WSAGetLastError());
        return;
    }

    printf("raw socket opened\n");

    //-----------------------------------------------------------------
    // Step 2: Set socket option.
    int result = setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (const char*)&enable, sizeof(enable));
    if (sock < 0)
    {
        printf("setsockopt failed: %d\n", WSAGetLastError());
        return;
    }

    //-----------------------------------------------------------------
    // Step 3: Provide needed information about destination.
    dest_info.sin_family = AF_INET;

    //-----------------------------------------------------------------
    // Step 4: Send the packet out.
    while (1)
    {
        /*result = sendto(sock, (const char*)ip, ntohs(ip->iph_len), 0, (struct sockaddr*)&dest_info, sizeof(dest_info));*/
        result = sendto(sock, (const char*)buff, buff_len, 0, (struct sockaddr*)&dest_info, sizeof(dest_info));
        printf("sendto: %d\n", result);

        Sleep(1000);
    }

    //-----------------------------------------------------------------

    closesocket(sock);
}


unsigned short checksum(unsigned short* buff, int len_16)
{
    unsigned long sum = 0;

    for (int i = 0; i < len_16; i++)
    {
        sum += htons(*(buff)++);
    }

    do
    {
        sum = ((sum >> 16) + (sum & 0xFFFF));
    } while (sum & 0xFFFF0000);

    return (~sum);
}



int buff_init()
{
    int data_len = 0;

    memset(sendbuff, 0, PACKET_LEN);

    // IP

    //-----------------------------------
    // UDP

    //-----------------------------------
    // data
    sendbuff[28] = 'h';
    sendbuff[29] = 'o';
    sendbuff[30] = 'h';
    sendbuff[31] = 'o';

    //-----------------------------------
    data_len = 32;
    //-----------------------------------

    return data_len;
}