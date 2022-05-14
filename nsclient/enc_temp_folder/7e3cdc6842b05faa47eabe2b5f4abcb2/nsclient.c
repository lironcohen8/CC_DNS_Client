#define _WINSOCK_DEPRECATED_NO_WARNINGS // Used for enabling usage of functions like gethostbyname in vs
#define _CRT_SECURE_NO_WARNINGS // Used for enabling usage of functions like fopen in vs
#define DOMAIN_NAME_LENGTH = 1024
#include <stdio.h>
#include <math.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // Used to link the library Ws2_32.lib for windows socketing

WSADATA wsaData;
char* domainName, * IPAddress;
int retVal;

struct hostent* dnsQuery(char *domainName){
	struct hostent* queryResult = (struct hostent*)calloc(1, sizeof(struct hostent*));
	if (queryResult == NULL) {
		perror("Can't allocate memory for dnsQuery.");
	}
	queryResult = gethostbyname(domainName);
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
	IPAddress = (char*)calloc(1024, sizeof(char));
	if (IPAddress == NULL) {
		perror("Allocation for IP failed");
		exit(1);
	}

	// Ask user to enter domain name
	retVal = scanf("%s", domainName);
	while (strcmp(domainName, "quit") != 0) {
		// Running DNS query
		struct hostent* queryResult = dnsQuery(domainName);
		IPAddress = inet_ntoa(*((struct in_addr*)queryResult->h_addr_list[0]));
		printf(IPAddress);
		retVal = scanf("%s", domainName);
	}
}