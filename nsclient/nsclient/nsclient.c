#define _WINSOCK_DEPRECATED_NO_WARNINGS // Used for enabling usage of functions like gethostbyname in vs
#define _CRT_SECURE_NO_WARNINGS // Used for enabling usage of functions like fopen in vs
#include <stdio.h>
#include <math.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "dnsQueryPacket.c"
#pragma comment(lib, "ws2_32.lib") // Used to link the library Ws2_32.lib for windows socketing

WSADATA wsaData;
struct sockaddr_in serverAddr;
char* domainName, *resultIPAddress;
int retVal, sockfd, queryPacketId = 1;


void createSocketAndServerAddr(char *DnsServerIpAddress) {
	// Creating socket 
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("Can't create socket to server");
		exit(1);
	}

	// Creating server address struct
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, (PCSTR)DnsServerIpAddress, &(serverAddr.sin_addr.s_addr));
	serverAddr.sin_port = htons(53);
}

struct dnsQueryPacket* CreateDnsQueryPacket() {
	struct dnsQueryPacket* queryPacket = (struct dnsQueryPacket*)calloc(1, sizeof(struct dnsQueryPacket*));
	if (queryPacket == NULL) {
		perror("Can't allocate memory for queryPacket.");
	}

	// Creating query header
	struct header* header = (struct header*)calloc(1, sizeof(struct header*));
	if (header == NULL) {
		perror("Can't allocate memory for header.");
	}
	header->ID = queryPacketId++;
	header->QR = 0;
	header->OPCODE = 0;
	header->AA = 0;
	header->TC = 0;
	header->RD = 0;
	header->RA = 0;
	header->Z = 0;
	header->RCODE = 0;
	header->QDCOUNT = htons(1);
	header->ANCOUNT = 0;
	header->NSCOUNT = 0;
	header->ARCOUNT = 0;

	// Creating qname
	
	// Creating query question (after qname)

}

struct hostent* dnsQuery(char *domainName) {
	// Allocating memory for result struct
	struct hostent* queryResult = (struct hostent*)calloc(1, sizeof(struct hostent*));
	if (queryResult == NULL) {
		perror("Can't allocate memory for dnsQuery.");
	}

	// Creating dns query packet
	struct dnsQueryPacket * queryPacket = CreateDnsQueryPacket();

	// Send dns query to server
	int queryPacketLength = sizeof(struct dnsQueryPacket);
	int count = sendto(sockfd, &queryPacket, queryPacketLength, 0, &serverAddr, sizeof(struct sockaddr_in));
	if (count != queryPacketLength) { // There was an error
		perror("Couldn't send dns query packet to socket");
		exit(1);
	}

	// Recieve dns result from server
	count = recvfrom(sockfd, &queryResult, sizeof(struct hostent), 0, &serverAddr, sizeof(struct sockaddr_in));
	if (count < 0) { // There was an error
		perror("Couldn't receieve dns query result from socket");
		exit(1);
	}

	// queryResult = gethostbyname(domainName);
	return queryResult;
}

int main(int argc, char* argv[]) {
	char* DnsServerIpAddress = argv[1];

	// Initializing Winsock
	retVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (retVal != 0) {
		perror("Error at WSAStartup");
		exit(1);
	}

	// Allocating memory for domain name and IPAddress
	domainName = (char*)calloc(1024, sizeof(char));
	if (domainName == NULL) {
		perror("Can't allocate memory for domainName.");
	}
	resultIPAddress = (char*)calloc(1024, sizeof(char));
	if (resultIPAddress == NULL) {
		perror("Allocation for IP failed");
		exit(1);
	}

	// Creating socket and server address
	createSocketAndServerAddr(DnsServerIpAddress);

	// Asking user to enter domain name
	printf("nsclient> ");
	retVal = scanf("%s", domainName);
	while (strcmp(domainName, "quit") != 0) {
		// Running DNS query using socket
		struct hostent* queryResult = dnsQuery(domainName);
		resultIPAddress = inet_ntoa(*((struct in_addr*)queryResult->h_addr_list[0]));
		printf("%s\n", resultIPAddress);
		printf("nsclient> ");
		retVal = scanf("%s", domainName);
	}

	// Closing socket and cleaning up WSA
	closesocket(sockfd);
	WSACleanup();
	return 0;
}