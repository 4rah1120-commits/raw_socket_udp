#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

// IP Header 구조체 정의
// IP Header
struct ipheader
{
	unsigned char iph_ihl : 4, iph_ver : 4; // IP header length, // IP version
	unsigned char iph_tos;           // Type of service
	unsigned short int iph_len;      // IP Packet length (data + header)
	unsigned short int iph_ident;    // Identification
	unsigned short int iph_flag : 3, // Fragmentation flags
		iph_offset : 13;             // Flags offset
	unsigned char iph_ttl;           // Time to Live
	unsigned char iph_protocol;      // Protocol type
	unsigned short int iph_chksum;   // IP datagram checksum
	struct in_addr iph_sourceip;     // Source IP address
	struct in_addr iph_destip;       // Destination IP address
};

struct udpheader
{
	u_short udp_sport; // source port
	u_short udp_dport; // destination port
	u_short total_length;
	u_short udp_sum; // checksum
};

#define SOURCE_IP "172.30.1.93"

#define DEST_IP "172.30.1.93"

#define SOURCE_PORT 3000
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


struct ipheader* ip_header_p = (struct ipheader*)&sendbuff;  //선언 ipheader
// ipheader구조체 안에 있는 멤버들을 sendbuff 배열의 주소로 연결






int buff_init()
{
	int udp_data_len = 4; // '하하' 가 4bytes이므로 4로 초기화. 여기가 0이면 보낼 내용이 없다는 말

	memset(sendbuff, 0, PACKET_LEN);  //패킷 렝스만큼 0으로 채우겠다. -> 0번부터

	// sendbuff : char 타입의 1500바이트 길이의 배열
	// ip_header_p : (struct ipheader*) 포인터인데, struct ipheader 타입의 주소를 담는 변수이다
	// 그래서 1byte씩 증가하는 sendbuff sendbuff에서 struct ipheade 구조체 멤버의 변수를 그대로 사용 가능
	
	//=================================================================
	// header 선언

	struct ipheader* ip_header_p = (struct ipheader*)&sendbuff;  //선언 ipheader


	struct udpheader* udp_header_p = (struct udpheader *)(sendbuff + sizeof(struct ipheader));
		// 센드버프 자체는 포인터. 배열의 주소. 여서, +1 하면 다음 캐릭터 주소
		// 그 주소에 struct ipheader 구조체 크기를 더한 위치 = udp헤더가 들어갈 주소.
		// (sendbuff + sizeof(struct ipheader) 이 위치에 (struct udpheader) 타입의 포인터를 캐스팅하게 되면
		// sendbuff 1byte씩 이용하는 struct udpheader 타입의 구조체 변수를 그대로 이용할 수 있게 됨


	// 여기서 이제 데이터 영역으로 넘어가보자


	char* udp_data = (char*)(sendbuff + sizeof(struct ipheader) + sizeof(struct udpheader));
	// sendbuff 주소에 + sizeof(struct ipheader) : 이거 크기 더하고,  sizeof(struct udpheader) 이거 크기 더해서
	// 데이터 영역의 시작 주소 (udp_data) fmf rkflzlrh
	// char* udp_data_p : char 타입의 포인터 변수 udp_data_p 선언.
	// udp 토탈 길이는 ip 빼고 


	int ip_total_length = sizeof(struct ipheader) + sizeof(struct udpheader) + udp_data_len; // ip header + udp header + data length


	//=================================================================
	// IP header
	// 포맷 테이블에 있는 거 다 정해주는 거
	//
	ip_header_p->iph_ver = 4; // IP version : IPv4
	ip_header_p->iph_ihl = 5; // IP header length : 5 * 4 = 20 bytes
	ip_header_p->iph_tos = 0; // Type of Service : 0
	ip_header_p->iph_len = ip_total_length; // Total Length : 이더넷 frame header를 제외한 모든 데이터 길이 (IP header + UDP header + data)
	ip_header_p->iph_ident = 0;
	ip_header_p->iph_flag = 0;
	ip_header_p->iph_offset = 0;
	ip_header_p->iph_ttl = 64;
	ip_header_p->iph_protocol = IPPROTO_UDP; // Protocol : UDP
	ip_header_p->iph_chksum = 0;
	ip_header_p->iph_sourceip.s_addr = inet_addr(DEST_IP);  //inet_addr은 DEST_IP 주소를 s_addr에 사용가능한 형태로 바꿔주는 것
	ip_header_p->iph_destip.s_addr = inet_addr(DEST_IP);

	// IP Header Checksum 계산.
	// ip_header_p : 구조체 변수 -> unsigned short* 로 캐스팅하게 되면, unsigned short 타입의 포인터 변수가 됨. (2바이트 단위로 주소가 증가)
	ip_header_p->iph_chksum = htons(checksum((unsigned short*)ip_header_p, 20 / 2));
		// htons : host to network short. 호스트 바이트 순서를 네트워크 바이트 순서로 변환
		// 왜냐면 htons host(PC 매모리에는 little-endian으로 저장) -> network(big-endian)

	//=================================================================
	// UDP header
	// 
	udp_header_p->udp_sport = htons(SOURCE_PORT); // 3000은 pc에서 리틀앤디언. htons으로 
	udp_header_p->udp_dport = htons(DEST_PORT); // 4000은 pc에서 리틀앤디언.

		// UDP total length = UDP header (8 bytes) + data (4 bytes)
		// sizeof(struct udpheader) : UDP header 구조체 크기 8 bytes

	udp_header_p->total_length = htons((unsigned short)(sizeof(struct udpheader) + udp_data_len)); // UDP header + data length
		// udp_total_length 도 PC메모리에 저장된 리틀앤디언이므로 htons 함수 이용해서 빅앤디언으로 바꿔야 함

	udp_header_p->udp_sum = 0; 
	
	
	//=================================================================
	// DATA
	// 

	udp_data[0] = 'h';
	udp_data[1] = 'a';
	udp_data[2] = 'h';
	udp_data[3] = 'a'; // 데이터 영역에 'haha' 문자열 넣기. 4bytes
	//지금 여기 data length은 4bytes여서,  data_len = 뮤조건 4로 초기화. haha 뒤에 뭐 와도 그냥 삭제됨


	////-----------------------------------
	//// IP
	//// 0x45 ==> 4 bit : version (4 : IPv4), 4 bit : header length (5-> 5*4=20 bytes)
	//sendbuff[0] = 0x45; // IP version and header length
	//// TOS
	//sendbuff[1] = 0x00; // Type of Service

	//// Total Length : 이더넷 frame header를 제외한 모든 데이터 길이 (IP header + UDP header + data)
	//// Little-endian : 0x1234 -> 메모리에 들어갈 때에는 34 12 순서로 들어감
	//// Big-endian : 0x1234 -> 네트워크로 나갈 때에는 패킷들이 12 34 순서로 나감 
	//short total_length = 20 + 8 + 4; // IP header (20 bytes) + UDP header (8 bytes) + data (4 bytes)
	//sendbuff[2] = (total_length >> 8) & 0xff; //비트를 오른쪽으로 8번 시프트. (&:비트연산.). 0xff : 하위는 1, 상위는 0. 8bit만 남기고 나머지는 0으로
	//sendbuff[3] = total_length & 0xff;

	//// IP Identification : IP 패킷을 구분하기 위한 ID
	//short ip_id = 0x00;
	//sendbuff[4] = (ip_id >> 8) & 0xff;
	//sendbuff[5] = ip_id & 0xff;

	//// Flag / Fragment Offset : 3 bit : flags, 13 bit : fragment offset
	//sendbuff[6] = 0x00; // Flags
	//sendbuff[7] = 0x00; // Fragment Offset

	//// TTL : Time To Live
	//sendbuff[8] = 128;

	//// protocol : UDP - 17, TCP - 6
	//sendbuff[9] = 17; // Protocol

	////// IP Header Checksum : IP header의 오류 검출을 위한 checksum
	////sendbuff[10] = 0;
	////sendbuff[11] = 0;

	//// IP Header Checksum 계산. 헤더첵섬은 2바이트. 
	////unsigned short header_checksum = checksum((unsigned short*)&sendbuff[0], 20 / 2); // 20/2 : 2바이트 단위로 10개. IP header 길이 20 bytes
	////sendbuff[10] = (header_checksum >> 8) & 0xff; // 상위 8bit
	////sendbuff[11] = header_checksum & 0xff; // 하위 8bit

	//// Source IP 
	//sendbuff[12] = 172;
	//sendbuff[13] = 30;
	//sendbuff[14] = 1;
	//sendbuff[15] = 93; //이건 빅엔디안이어서 그냥 차례대로 넣으면 됨

	//// destination IP  (친구 주소)
	//sendbuff[16] = 172;
	//sendbuff[17] = 30;
	//sendbuff[18] = 1;
	//sendbuff[19] = 60;  



	////-----------------------------------
	//// UDP Header
	//// Source Port
	//unsigned short udp_source_port = 3000;
	//sendbuff[20] = (udp_source_port >> 8) & 0xff; // 상위 8bit만
	//sendbuff[21] = udp_source_port & 0xff; // 하위 8bit

	//// Destination Port
	//unsigned short udp_destination_port = 3000;
	//sendbuff[22] = (udp_destination_port >> 8) & 0xff; // 상위 8bit
	//sendbuff[23] = udp_destination_port & 0xff; // 하위 8bit
//
//#include <iostream>
//#include <WinSock2.h>
//#include <ws2tcpip.h>
//
//#pragma comment(lib, "ws2_32.lib")
//
//		// IP Header 구조체 정의
//		struct ipheader
//	{
//		unsigned char iph_ver : 4;
//		unsigned char iph_ihl : 4;
//		unsigned char iph_tos;
//		unsigned short iph_len;
//		unsigned short iph_ident;
//		unsigned char iph_flag;
//		unsigned short iph_offset;
//		unsigned char iph_ttl;
//		unsigned char iph_protocol;
//		unsigned short iph_checksum;
//		struct in_addr iph_sourceip;
//		struct in_addr iph_destip;
//	};
//
//	// UDP Header 구조체 정의
//	struct udpheader
//	{
//		unsigned short udph_scport;
//		unsigned short udph_destport;
//		unsigned short total_length;
//		unsigned short udp_sum;
//	};
//
//#define SOURCE_IP "172.30.1.93"
//
//#define DEST_IP "172.30.1.93"
//
//#define SOURCE_PORT 3000
//#define DEST_PORT 3000
//
//#define PACKET_LEN 1500
//
//	void send_raw_ip_packet(char* buff, int buff_len);
//	int buff_init();
//
//	char sendbuff[PACKET_LEN];
//
//
//	int main()
//	{
//		WORD wVersionRequested;
//		WSADATA wsaData;
//
//		wVersionRequested = MAKEWORD(1, 1);
//
//		WSAStartup(wVersionRequested, &wsaData);
//
//		//-----------------------------------
//		int data_len = buff_init();
//		//-----------------------------------------------------------------
//		send_raw_ip_packet(sendbuff, data_len);
//		//-----------------------------------------------------------------
//
//		WSACleanup();
//	};


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



//	void send_raw_ip_packet(char* buff, int buff_len)
//	{
//		struct sockaddr_in dest_info;
//		int enable = 1;
//
//		//-----------------------------------------------------------------
//		// Step 1: Create a raw network socket.
//		int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);  // sock이라는 번호로 TCP/IP 소켓을 생성.
//		// 나중에 TCP UDP를 이걸로 열 수도 있음.
//// int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
//		if (sock < 0)
//		{
//			printf("socket failed: %d\n", WSAGetLastError());
//			return;
//		}
//
//		printf("raw socket opened\n");
//
//		//-----------------------------------------------------------------
//		// Step 2: Set socket option.
//		int result = setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (const char*)&enable, sizeof(enable));
//		if (sock < 0)
//		{
//			printf("setsockopt failed: %d\n", WSAGetLastError());
//			return;
//		}
//
//		//-----------------------------------------------------------------
//		// Step 3: Provide needed information about destination.
//		dest_info.sin_family = AF_INET; //인터넷 관련된 IP패킷. 지금은 UDP소켓이라 이 정도만 써줘도 됨
//
//		//-----------------------------------------------------------------
//		// Step 4: Send the packet out.
//		while (1)
//		{
//			/*result = sendto(sock, (const char*)ip, ntohs(ip->iph_len), 0, (struct sockaddr*)&dest_info, sizeof(dest_info));*/
//			result = sendto(sock, (const char*)buff, buff_len, 0, (struct sockaddr*)&dest_info, sizeof(dest_info));
//			printf("sendto: %d\n", result);
//
//			Sleep(1000);
//		}
//
//		//-----------------------------------------------------------------
//
//		closesocket(sock);
//	}
//
//	// checksum() 함수 인자 (unsigned short* buff)
//	// 처음 선언에  char buff[] 라는 배열로 선언했지만
//	// (unsigned short* buff)로 바꿔서 2바이트 단위의 u short 포인터로 캐스팅.
//	// -> 1바이트가 아닌 2바이트 단위로 checksum 계산의 주소가 증가
//	// len_16 : 2바이트 단위의 갯수. 
//	unsigned short checksum(unsigned short* buff, int len_16)
//	{
//		unsigned long sum = 0;
//
//		for (int i = 0; i < len_16; i++)
//		{
//			// *(buff)++ : buff는 2바이트 단위의 주소.
//			// ++ 의 의미 : 포인터를 하나씩 증가시켜서 주소를 다음으로 증가
//			// 메모리 공간은 2바이트 단위로 증가.
//			// buff++ (포인터 증가) 에 *를 붙이면 buff가 가리키는 주소의 값을 가져옴
//			// htons() : host to network short. 호스트 바이트 순서를 네트워크 바이트 순서로 변환
//			// host(PC: little-endian) -> network(big-endian)
//			sum += htons(*(buff)++);
//		}
//
//		do
//		{
//			sum = ((sum >> 16) + (sum & 0xFFFF));
//		} while (sum & 0xFFFF0000);
//
//		return (~sum);
//	}
//
//
//	struct ipheader* ip_header_p = (struct ipheader*)&sendbuff;  //선언 ipheader
//	// ipheader구조체 안에 있는 멤버들을 sendbuff 배열의 주소로 연결
//
//
//
//
//
//
//	int buff_init()
//	{
//		int udp_data_len = 4; // '하하' 가 4bytes이므로 4로 초기화. 여기가 0이면 보낼 내용이 없다는 말
//
//		memset(sendbuff, 0, PACKET_LEN);  //패킷 렝스만큼 0으로 채우겠다. -> 0번부터
//
//		// sendbuff : char 타입의 1500바이트 길이의 배열
//		// ip_header_p : (struct ipheader*) 포인터인데, struct ipheader 타입의 주소를 담는 변수이다
//		// 그래서 1byte씩 증가하는 sendbuff sendbuff에서 struct ipheade 구조체 멤버의 변수를 그대로 사용 가능
//
//		//=================================================================
//		// header 선언
//
//		struct ipheader* ip_header_p = (struct ipheader*)&sendbuff;  //선언 ipheader
//
//
//		struct udpheader* udp_header_p = (struct udpheader*)(sendbuff + sizeof(struct ipheader));
//		// 센드버프 자체는 포인터. 배열의 주소. 여서, +1 하면 다음 캐릭터 주소
//		// 그 주소에 struct ipheader 구조체 크기를 더한 위치 = udp헤더가 들어갈 주소.
//		// (sendbuff + sizeof(struct ipheader) 이 위치에 (struct udpheader) 타입의 포인터를 캐스팅하게 되면
//		// sendbuff 1byte씩 이용하는 struct udpheader 타입의 구조체 변수를 그대로 이용할 수 있게 됨
//
//
//	// 여기서 이제 데이터 영역으로 넘어가보자
//
//
//		char* udp_data_p = (sendbuff + sizeof(struct ipheader) + sizeof(struct udpheader));
//		// sendbuff 주소에 + sizeof(struct ipheader) : 이거 크기 더하고,  sizeof(struct udpheader) 이거 크기 더해서
//		// 데이터 영역의 시작 주소 (udp_data) fmf rkflzlrh
//		// char* udp_data_p : char 타입의 포인터 변수 udp_data_p 선언.
//		// udp 토탈 길이는 ip 빼고 
//
//
//		int ip_total_length = sizeof(struct ipheader) + sizeof(struct udpheader) + udp_data_len; // ip header + udp header + data length
//
//
//		//=================================================================
//		// IP header
//		// 포맷 테이블에 있는 거 다 정해주는 거
//		//
//		ip_header_p->iph_ver = 4; // IP version : IPv4
//		 // IP version : IPv4
//		ip_header_p->iph_ihl = 5; // IP header length : 5 * 4 = 20 bytes
//		ip_header_p->iph_tos = 0; // Type of Service : 0
//		ip_header_p->iph_len = htons(ip_total_length); // Total Length : 이더넷 frame header를 제외한 모든 데이터 길이 (IP header + UDP header + data)
//		ip_header_p->iph_ident = 0;
//		ip_header_p->iph_flag = 0;
//		ip_header_p->iph_offset = 0;
//		ip_header_p->iph_ttl = 64;
//		ip_header_p->iph_protocol = IPPROTO_UDP; // Protocol : UDP
//		ip_header_p->iph_checksum = 0;
//		ip_header_p->iph_sourceip.s_addr = inet_addr(DEST_IP);  //inet_addr은 DEST_IP 주소를 s_addr에 사용가능한 형태로 바꿔주는 것
//		ip_header_p->iph_destip.s_addr = inet_addr(DEST_IP);
//
//		// IP Header Checksum 계산.
//		// ip_header_p : 구조체 변수 -> unsigned short* 로 캐스팅하게 되면, unsigned short 타입의 포인터 변수가 됨. (2바이트 단위로 주소가 증가)
//		ip_header_p->iph_checksum = htons(checksum((unsigned short*)ip_header_p, 20 / 2));
//		// htons : host to network short. 호스트 바이트 순서를 네트워크 바이트 순서로 변환
//		// 왜냐면 htons host(PC 매모리에는 little-endian으로 저장) -> network(big-endian)
//
//	//=================================================================
//	// UDP header
//	// 
//		udp_header_p->udph_scport = htons(SOURCE_PORT); // 3000은 pc에서 리틀앤디언. htons으로 
//		udp_header_p->udph_destport = htons(DEST_PORT); // 4000은 pc에서 리틀앤디언.
//
//		// UDP total length = UDP header (8 bytes) + data (4 bytes)
//		// sizeof(struct udpheader) : UDP header 구조체 크기 8 bytes
//
//		udp_header_p->total_length = htons((unsigned short)(sizeof(struct udpheader) + udp_data_len)); // UDP header + data length
//		// udp_total_length 도 PC메모리에 저장된 리틀앤디언이므로 htons 함수 이용해서 빅앤디언으로 바꿔야 함
//
//		udp_header_p->udp_sum = 0;
//
//
//		//=================================================================
//		// DATA
//		// 
//		udp_data_p = (char*)"haha"; // 데이터 영역에 'haha' 문자열 넣기. 4bytes
//		//지금 여기 data length은 4bytes여서,  data_len = 뮤조건 4로 초기화. haha 뒤에 뭐 와도 그냥 삭제됨


	//	//-----------------------------------
	//	// IP
	//	// 0x45 ==> 4 bit : version (4 : IPv4), 4 bit : header length (5-> 5*4=20 bytes)
	//	sendbuff[0] = 0x45; // IP version and header length
	//	// TOS
	//	sendbuff[1] = 0x00; // Type of Service

	//	// Total Length : 이더넷 frame header를 제외한 모든 데이터 길이 (IP header + UDP header + data)
	//	// Little-endian : 0x1234 -> 메모리에 들어갈 때에는 34 12 순서로 들어감
	//	// Big-endian : 0x1234 -> 네트워크로 나갈 때에는 패킷들이 12 34 순서로 나감 
	//	short total_length = 20 + 8 + 4; // IP header (20 bytes) + UDP header (8 bytes) + data (4 bytes)
	//	sendbuff[2] = (total_length >> 8) & 0xff; //비트를 오른쪽으로 8번 시프트. (&:비트연산.). 0xff : 하위는 1, 상위는 0. 8bit만 남기고 나머지는 0으로
	//	sendbuff[3] = total_length & 0xff;

	//	// IP Identification : IP 패킷을 구분하기 위한 ID
	//	short ip_id = 0x00;
	//	sendbuff[4] = (ip_id >> 8) & 0xff;
	//	sendbuff[5] = ip_id & 0xff;

	//	// Flag / Fragment Offset : 3 bit : flags, 13 bit : fragment offset
	//	sendbuff[6] = 0x00; // Flags
	//	sendbuff[7] = 0x00; // Fragment Offset

	//	// TTL : Time To Live
	//	sendbuff[8] = 128;

	//	// protocol : UDP - 17, TCP - 6
	//	sendbuff[9] = 17; // Protocol

	//	//// IP Header Checksum : IP header의 오류 검출을 위한 checksum
	//	//sendbuff[10] = 0;
	//	//sendbuff[11] = 0;

	//	// IP Header Checksum 계산. 헤더첵섬은 2바이트. 
	//	//unsigned short header_checksum = checksum((unsigned short*)&sendbuff[0], 20 / 2); // 20/2 : 2바이트 단위로 10개. IP header 길이 20 bytes
	//	//sendbuff[10] = (header_checksum >> 8) & 0xff; // 상위 8bit
	//	//sendbuff[11] = header_checksum & 0xff; // 하위 8bit

	//	// Source IP 
	//	sendbuff[12] = 172;
	//	sendbuff[13] = 30;
	//	sendbuff[14] = 1;
	//	sendbuff[15] = 93; //이건 빅엔디안이어서 그냥 차례대로 넣으면 됨

	//	// destination IP  (친구 주소)
	//	sendbuff[16] = 172;
	//	sendbuff[17] = 30;
	//	sendbuff[18] = 1;
	//	sendbuff[19] = 60;



	//	//-----------------------------------
	//	// UDP Header
	//	// Source Port
	//	unsigned short udp_source_port = 3000;
	//	sendbuff[20] = (udp_source_port >> 8) & 0xff; // 상위 8bit만
	//	sendbuff[21] = udp_source_port & 0xff; // 하위 8bit

	//	// Destination Port
	//	unsigned short udp_destination_port = 4000;
	//	sendbuff[22] = (udp_destination_port >> 8) & 0xff; // 상위 8bit만
	//	sendbuff[23] = udp_destination_port & 0xff; // 하위 8bit

	//	// Length : UDP header + data length
	//	//unsigned short udp_total_length = 8 + 4; // UDP header (8 bytes) + data (4 bytes)
	//	//sendbuff[24] = (udp_total_length >> 8) & 0xff; // 상위 8bit
	//	//sendbuff[25] = udp_total_length & 0xff; // 하위 8bit

	//	// Checksum : UDP header + data의 오류 검출을 위한 checksum
	//	sendbuff[26] = 0;
	//	sendbuff[27] = 0;

	//	//-----------------------------------
	//	// data
	//	//sendbuff[28] = 'h';
	//	//sendbuff[29] = 'o';
	//	//sendbuff[30] = 'h';
	//	//sendbuff[31] = 'o';

	//	//-----------------------------------
	//	// int total_data_len = 32;
	//	int total_data_len = ip_total_length;


	//	//-----------------------------------

	//	return total_data_len;
	//}
	//(destination_port >> 8) & 0xff; // 상위 8bit만
	//sendbuff[23] = udp_destination_port & 0xff; // 하위 8bit

	//// Length : UDP header + data length
	////unsigned short udp_total_length = 8 + 4; // UDP header (8 bytes) + data (4 bytes)
	////sendbuff[24] = (udp_total_length >> 8) & 0xff; // 상위 8bit
	////sendbuff[25] = udp_total_length & 0xff; // 하위 8bit

	//// Checksum : UDP header + data의 오류 검출을 위한 checksum
	//sendbuff[26] = 0;
	//sendbuff[27] = 0;

	////-----------------------------------
	//// data
	////sendbuff[28] = 'h';
	////sendbuff[29] = 'o';
	////sendbuff[30] = 'h';
	////sendbuff[31] = 'o';

	//-----------------------------------
	// int total_data_len = 32;
	int total_data_len = ntohs(ip_total_length);


	//-----------------------------------

	return total_data_len;
}