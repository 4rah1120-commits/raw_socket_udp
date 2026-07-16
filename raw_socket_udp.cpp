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


int main()
{
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(1, 1);

    WSAStartup(wVersionRequested, &wsaData);

    //-----------------------------------
    int data_len = buff_init();
    //-----------------------------------------------------------------
    send_raw_ip_packet(sendbuff, data_len);
    //-----------------------------------------------------------------

    WSACleanup();
}


//int main(void) {
//
//    WORD wVersionRequested;
//    WSADATA wsaData;
//
//    wVersionRequested = MAKEWORD(1, 1);
//
//    WSAStartup(wVersionRequested, &wsaData);
//
//    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
//
//    char message[64];
//
//    if (sockfd < 0) {
//        perror("Error creating socket");
//        exit(1);
//    }
//
//    struct sockaddr_in sock_in;
//    sock_in.sin_family = AF_INET;
//    sock_in.sin_port = htons(3000);
//    sock_in.sin_addr.s_addr = INADDR_ANY;
//
//    if (bind(sockfd, (struct sockaddr*)&sock_in, sizeof(sock_in)) < 0) {
//        printf("Bind failed\n");
//        exit(1);
//    }
//
//    if (recv(sockfd, message, 64, 0) < 0) {
//        perror("Error");
//        exit(1);
//    }
//
//    printf("\n\n%s\n\n", message);
//
//    closesocket(sockfd);
//
//    WSACleanup();
//
//    return 0;
//}



void send_raw_ip_packet(char* buff, int buff_len)
{
    struct sockaddr_in dest_info;
    int enable = 1;

    //-----------------------------------------------------------------
    // Step 1: Create a raw network socket.
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);  // sock이라는 번호로 TCP/IP 소켓을 생성.
                                                        // 나중에 TCP UDP를 이걸로 열 수도 있음.
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
    dest_info.sin_family = AF_INET; //인터넷 관련된 IP패킷. 지금은 UDP소켓이라 이 정도만 써줘도 됨

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

// checksum() 함수 인자 (unsigned short* buff)
// 처음 선언에  char buff[] 라는 배열로 선언했지만
// (unsigned short* buff)로 바꿔서 2바이트 단위의 u short 포인터로 캐스팅.
// -> 1바이트가 아닌 2바이트 단위로 checksum 계산의 주소가 증가
// len_16 : 2바이트 단위의 갯수. 
unsigned short checksum(unsigned short* buff, int len_16)
{
    unsigned long sum = 0;

    for (int i = 0; i < len_16; i++)
    {
		// *(buff)++ : buff는 2바이트 단위의 주소.
		// ++ 의 의미 : 포인터를 하나씩 증가시켜서 주소를 다음으로 증가
		// 메모리 공간은 2바이트 단위로 증가.
		// buff++ (포인터 증가) 에 *를 붙이면 buff가 가리키는 주소의 값을 가져옴
		// htons() : host to network short. 호스트 바이트 순서를 네트워크 바이트 순서로 변환
		// host(PC: little-endian) -> network(big-endian)
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

    memset(sendbuff, 0, PACKET_LEN);  //패킷 렝스만큼 0으로 채우겠다. -> 0번부터 

    //-----------------------------------
    // IP
	// 0x45 ==> 4 bit : version (4 : IPv4), 4 bit : header length (5-> 5*4=20 bytes)
	sendbuff[0] = 0x45; // IP version and header length
    // TOS
	sendbuff[1] = 0x00; // Type of Service

	// Total Length : 이더넷 frame header를 제외한 모든 데이터 길이 (IP header + UDP header + data)
	// Little-endian : 0x1234 -> 메모리에 들어갈 때에는 32 12 순서로 들어감
	// Big-endian : 0x1234 -> 네트워크로 나갈 때에는 패킷들이 12 34 순서로 나감 
    short total_length = 20 + 8 + 4; // IP header (20 bytes) + UDP header (8 bytes) + data (4 bytes)
	sendbuff[2] = (total_length >> 8) & 0xFF; //비트를 오른쪽으로 8번 시프트. (&:비트연산.). 0xFF : 하위는 1, 상위는 0. 8bit만 남기고 나머지는 0으로
    sendbuff[3] = total_length & 0xFF;
    
	// IP Identification : IP 패킷을 구분하기 위한 ID
    short ip_id = 0x00;
	sendbuff[4] = (ip_id >> 8) & 0xff;
	sendbuff[5] = ip_id & 0xff;
    
	// Flag / Fragment Offset : 3 bit : flags, 13 bit : fragment offset
	sendbuff[6] = 0x00; // Flags
	sendbuff[7] = 0x00; // Fragment Offset

	// TTL : Time To Live
    sendbuff[8] = 128;

	// protocol : UDP - 17, TCP - 6
	sendbuff[9] = 17; // Protocol]

	// IP Header Checksum : IP header의 오류 검출을 위한 checksum
    sendbuff[10] = 0;
	sendbuff[11] = 0;

	// Source IP 
    sendbuff[12] = 172;
    sendbuff[13] = 30;
    sendbuff[14] = 1;
    sendbuff[15] = 93; //이건 빅엔디안이어서 그냥 차례대로 넣으면 됨

	// destination IP  (친구 주소)
    sendbuff[16] = 172;
    sendbuff[17] = 30;
    sendbuff[18] = 1;
    sendbuff[19] = 60;  

    // IP Header Checksum 계산. 헤더첵섬은 2바이트. 
	unsigned short header_checksum = checksum((unsigned short*)&sendbuff[0], 20 / 2); // 20/2 : 2바이트 단위로 10개. IP header 길이 20 bytes
	sendbuff[10] = (header_checksum >> 8) & 0xff; // 상위 8bit
	sendbuff[11] = header_checksum & 0xff; // 하위 8b

    //-----------------------------------
	// UDP Header
	// Source Port
	unsigned short udp_source_port = 3000;
	sendbuff[20] = (udp_source_port >> 8) & 0xff; // 상위 8bit만
    sendbuff[21] = udp_source_port & 0xff; // 하위 8bit

	// Destination Port
    unsigned short udp_destination_port = 4000;
    sendbuff[22] = (udp_destination_port >> 8) & 0xff; // 상위 8bit만
    sendbuff[23] = udp_destination_port & 0xff; // 하위 8bit
    
	// Length : UDP header + data length
	unsigned short udp_total_length = 8 + 4; // UDP header (8 bytes) + data (4 bytes)
    sendbuff[24] = (udp_total_length >> 8) & 0xff; // 상위 8bit
    sendbuff[25] = udp_total_length & 0xff; // 하위 8bit

	// Checksum : UDP header + data의 오류 검출을 위한 checksum
	sendbuff[26] = 0;
	sendbuff[27] = 0;

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