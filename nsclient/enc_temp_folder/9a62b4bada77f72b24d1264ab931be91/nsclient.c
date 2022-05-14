#define _WINSOCK_DEPRECATED_NO_WARNINGS // Used for enabling usage of functions like gethostbyname in vs
#define _CRT_SECURE_NO_WARNINGS // Used for enabling usage of functions like fopen in vs
#include <stdio.h>
#include <math.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "dnsPacketElements.c"
#pragma comment(lib, "ws2_32.lib") // Used to link the library Ws2_32.lib for windows socketing

WSADATA wsaData;
struct sockaddr_in serverAddr;
unsigned char buf[65536];
char* domainName, *resultIPAddress;
int retVal, sockfd, qNameLength, addressSize = sizeof(struct sockaddr_in), queryPacketId = 1;

void parseDomainNameIntoParts() {
	// TODO delete
	char* result = (char*)calloc(1024, sizeof(char));
	char* token = strtok(domainName, ".");
	while (token) {
		printf("%s\n", token);
		token = strtok(NULL, ".");
	}
}

void createSocketAndServerAddr(char *DnsServerIpAddress) {
	// Creating socket 
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("Can't create socket to server");
		exit(1);
	}

	// Setting timeout to socket to 2 seconds
	struct timeval tv;
	tv.tv_sec = 2;
	retVal = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	if (retVal < 0) {
		perror("Can't set timeout to socket");
		exit(1);
	}

	// Creating server address struct
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, (PCSTR)DnsServerIpAddress, &(serverAddr.sin_addr.s_addr));
	serverAddr.sin_port = htons(53);
}

void createQueryHeader(struct header* header) {
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
}

void createQueryQname(char *qName) {
	int originalLength = strlen(domainName); // www.google.com -> 3www6google3com0
	qName = "3www6google3com0";
	qNameLength = strlen(qName) + 1;
}

void createQueryQuestion(struct question *question) {
	question->QCLASS = htons(1);
	question->QTYPE = htons(1);
}
void createDnsQueryPacket() {
	struct header* header = NULL;

	// Creating query header
	header = (struct header*)&buf;
	createQueryHeader(header);
	
	// Creating query qName
	char *qName = (unsigned char*)&buf[sizeof(struct header)];
	createQueryQname(qName);

	// Creating query question
	struct question* question = (struct question*)&buf[sizeof(header) + qNameLength];
	createQueryQuestion(question);
}

//struct hostent* parseAnswerFromAnswerPacket() {
//	// Allocating memory for result struct
//	struct hostent* queryResult = (struct hostent*)calloc(1, sizeof(struct hostent*));
//	if (queryResult == NULL) {
//		perror("Can't allocate memory for dnsQuery.");
//	}
//	// TODO continue
//	return queryResult;
//}

struct hostent* dnsQuery(char *domainName) {
	// Creating dns query packet
	createDnsQueryPacket();

	// Sending dns query packet to server
	int queryPacketLength = sizeof(struct header) + qNameLength + sizeof(struct question);
	retVal = sendto(sockfd, &buf, queryPacketLength, 0, &serverAddr, addressSize);
	if (retVal == SOCKET_ERROR) { // There was an error
		perror("Couldn't send dns query packet to socket");
		exit(1);
	}

	// Recieving dns result from server
	//struct res_record answers[20], auth[20], addit[20]; // the replies from the DNS server
	//int answerPacketLength = sizeof(struct dnsAnswerPacket);
	retVal = recvfrom(sockfd, (char *)buf, 65536, 0, &serverAddr, &addressSize);
	if (retVal == SOCKET_ERROR) { // There was an error
		perror("Couldn't receieve dns query result from socket");
		exit(1);
	}
	struct header* answerPacket = (struct header*)buf;


	// Parsing queryResult from answer packet
	struct hostent* queryResult = NULL; // ParseAnswerFromAnswerPacket(answerPacket);
	char* answerReader = &buf[queryPacketLength];

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