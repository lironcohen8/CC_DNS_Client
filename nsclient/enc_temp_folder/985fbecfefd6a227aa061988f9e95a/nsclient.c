#define _WINSOCK_DEPRECATED_NO_WARNINGS // Used for enabling usage of functions like gethostbyname in vs
#define _CRT_SECURE_NO_WARNINGS // Used for enabling usage of functions like fopen in vs
#define DOMAIN_NAME_LENGTH = 1024
#include <stdio.h>
#include <math.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // Used to link the library Ws2_32.lib for windows socketing

WSADATA wsaData;
char* domainName;
int retVal;

struct hostent* dnsQuery(char *domainName){
	struct hostent* query = (struct hostent*)calloc(1, sizeof(struct hostent*));
	if (query == NULL) {
		perror("Can't allocate memory for dnsQuery.");
	}
	query = gethostbyname(domainName);
	return query;
};

int main(int argc, char* argv[]) {
	char* DnsServerIpAddress = argv[1];

	// Initializing Winsock
	retVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (retVal != 0) {
		perror("Error at WSAStartup");
		exit(1);
	}

	// Allocating memory for domain name
	domainName = (char*)calloc(1024, sizeof(char));
	if (domainName == NULL) {
		perror("Can't allocate memory for domainName.");
	}

	// Ask user to enter domain name
	retVal = scanf("%s", domainName);
	while (strcmp(domainName, "quit") != 0) {
		// calling function 
		struct hostent* queryResult = dnsQuery(domainName);
	}
}