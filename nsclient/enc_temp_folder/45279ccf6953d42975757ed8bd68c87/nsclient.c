#define _WINSOCK_DEPRECATED_NO_WARNINGS // Used for enabling usage of functions like gethostbyname in vs
#define _CRT_SECURE_NO_WARNINGS // Used for enabling usage of functions like fopen in vs
#define DOMAIN_NAME_LENGTH = 1024
#include <stdio.h>
#include <math.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // Used to link the library Ws2_32.lib for windows socketing

WSADATA wsaData;
struct sockaddr_in serverAddr;
char* domainName, *resultIPAddress;
int retVal, sockfd;


void connectToSocket(char *DnsServerIpAddress) {
	// Creating socket 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("Can't create socket to server");
		exit(1);
	}

	// Creating server address struct
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, (PCSTR)DnsServerIpAddress, &(serverAddr.sin_addr.s_addr));

	// Connecting to server
	retVal = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (retVal < 0) {
		perror("Can't connect to server");
		exit(1);
	}
}


struct hostent* dnsQuery(char *domainName){
	struct hostent* queryResult = (struct hostent*)calloc(1, sizeof(struct hostent*));
	if (queryResult == NULL) {
		perror("Can't allocate memory for dnsQuery.");
	}
	// queryResult = gethostbyname(domainName);
	return queryResult;
};

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

	// Connecting to server via UDP socket
	connectToSocket(DnsServerIpAddress);

	// Asking user to enter domain name
	retVal = scanf("%s", domainName);
	while (strcmp(domainName, "quit") != 0) {
		// Running DNS query
		struct hostent* queryResult = dnsQuery(domainName);
		resultIPAddress = inet_ntoa(*((struct in_addr*)queryResult->h_addr_list[0]));
		printf("%s\n", resultIPAddress);
		retVal = scanf("%s", domainName);
	}
}