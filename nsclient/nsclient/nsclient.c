#include "nsclient.h"

WSADATA wsaData;
struct sockaddr_in serverAddr;
unsigned char buf[65536];
char* domainName, *resultIPAddress;
int retVal, sockfd, qNameLength, queryPacketLength, addressSize = sizeof(struct sockaddr_in), queryPacketId = 1;

void domainToLowercase() {
	// Converting domain name to lowercase 
	int domainLength = strlen(domainName);
	for (int i = 0; i < domainLength; i++) {
		domainName[i] = tolower(domainName[i]);
	}
}

int isDomainNameValid() {
	// Validation is based on rules from https://www.geeksforgeeks.org/how-to-validate-a-domain-name-using-regular-expression/

	int domainLength = strlen(domainName);
	// Check domain name length
	if (domainLength < 1 || domainLength > 63) {
		return FALSE;
	}

	// Check that first and last chars are not -
	if (domainName[0] == '-' || domainName[domainLength - 1] == '-') {
		return FALSE;
	}

	for (int i = 0; i < domainLength; i++) {
		char c = domainName[i];
		// Check that char is alphanumeric or . or -
		if (c != '.' && c != '-' && !isdigit(c) && !isalpha(c)) {
			return FALSE;
		}
	}

	// Check that last part is 2 to 6 characters
	char *lastDot = strrchr(domainName, '.');
	if (lastDot == NULL) {
		lastDot = domainName;
	}
	int lastLength = strlen(lastDot);
	if (lastLength < 2 || lastLength > 6) {
		return FALSE;
	}

	return TRUE;
}

void createSocketAndServerAddr(char *DnsServerIpAddress) {
	// Creating socket 
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("Can't create socket to server");
		exit(1);
	}

	// Setting timeout to socket to 2 seconds
	/*struct timeval tv;
	tv.tv_sec = TIMEOUT_IN_SECS;
	retVal = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
	if (retVal < 0) {
		perror("Can't set timeout to socket");
		exit(1);
	}*/

	// Creating server address struct
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, (PCSTR)DnsServerIpAddress, &(serverAddr.sin_addr.s_addr));
	serverAddr.sin_port = htons(53);
}

void createQueryHeader(struct header* header) {
	// Creating query header struct
	header->ID = queryPacketId++;
	header->QR = 0;
	header->OPCODE = 0;
	header->AA = 0;
	header->TC = 0;
	header->RD = htons(1); // TODO validate this change
	header->RA = 0;
	header->Z = 0;
	header->RCODE = 0;
	header->QDCOUNT = htons(1);
	header->ANCOUNT = 0;
	header->NSCOUNT = 0;
	header->ARCOUNT = 0;
}

void createQueryQname(char *qNameStart) {
	// Creating query name according to domain names encoding
	int letterIndex = 0;
	char* qName = qNameStart;
	strcat((char*)domainName, ".");

	// Counting characters until dot and adding the number upfront
	for (int dotIndex = 0; dotIndex < (int)strlen((char*)domainName); dotIndex++) {
		if (domainName[dotIndex] == '.')
		{
			*qName++ = dotIndex - letterIndex;
			for (; letterIndex < dotIndex; letterIndex++)
			{
				*qName++ = domainName[letterIndex];
			}
			letterIndex++; // Skipping dot
		}
	}

	*qName++ = '\0';
	qNameLength = strlen(qNameStart) + 1;
}

void createQueryQuestion(struct question *question) {
	// Creating query question struct
	question->QCLASS = htons(1);
	question->QTYPE = htons(1);
}

void createDnsQueryPacket() {
	// Creating dns query packet
	struct header* header = NULL;

	// Creating query header
	header = (struct header*)&buf;
	createQueryHeader(header);
	
	// Creating query qName
	char *qName = (unsigned char*)&buf[sizeof(struct header)];
	createQueryQname(qName);

	// Creating query question
	struct question* question = (struct question*)&buf[sizeof(struct header) + qNameLength];
	createQueryQuestion(question);
}

void parseAnswerFromAnswerPacket(struct hostent* queryResult) {
	struct header* answerHeader = (struct header*)buf;

	// Parsing the addresses from packet
	if (ntohl(answerHeader->RCODE) == 0) { // No error occurred

		// Setting up records array
		int recordsCount = ntohs(answerHeader->ANCOUNT);
		struct res_record *answersRecords = (struct res_record*)calloc(recordsCount, sizeof(struct res_record));
		if (answersRecords == NULL) {
			perror("Can't allocate memory for records' array");
			exit(1);
		}

		// Answer pointer starts from end of query
		unsigned char *answerPointer = &buf[queryPacketLength];

		// Going over records
		for (int i = 0; i < recordsCount; i++) {
			answerPointer += 2; // 2 bytes for name reference
			answersRecords[i].RESOURCE = (struct r_data*)(answerPointer);
			answerPointer += sizeof(struct r_data);

			uint16_t recordType = ntohs(answersRecords[i].RESOURCE->TYPE);
			if (recordType == 1) { // host address record
				uint16_t recordDataLength = ntohs(answersRecords[i].RESOURCE->RDLENGTH);

				// Setting up answer data
				answersRecords[i].RDATA = (unsigned char*)calloc(1, recordDataLength);
				if (answersRecords[i].RDATA == NULL) {
					perror("Can't allocate memory for record's data");
					exit(1);
				}

				// Reading data
				for (int j = 0; j < recordDataLength; j++) {
					answersRecords[i].RDATA[j] = answerPointer[j];
				}
				answersRecords[i].RDATA[recordDataLength] = '\0';
				answerPointer += recordDataLength;

				// Parsing address
				struct sockaddr_in resultAddress;
				resultAddress.sin_addr.s_addr = (*(long*)answersRecords[i].RDATA);
				char* resultIPAddress = inet_ntoa(resultAddress.sin_addr);
				queryResult->h_name = resultIPAddress;
			}
			else { // other record type
				uint16_t recordDataLength = ntohs(answersRecords[i].RESOURCE->RDLENGTH);
				answerPointer += recordDataLength;
			}
		}
	}
	else {
		perror("Got error response code form dns server");
	}
}

struct hostent* dnsQuery(char *domainName) {
	// Creating dns query packet
	createDnsQueryPacket();

	// Sending dns query packet to server
	queryPacketLength = sizeof(struct header) + qNameLength + sizeof(struct question);
	retVal = sendto(sockfd, (char *)buf, queryPacketLength, 0, &serverAddr, addressSize);
	if (retVal == SOCKET_ERROR) { // There was an error
		perror("Couldn't send dns query packet to socket");
		exit(1);
	}

	// Recieving dns result from server
	retVal = recvfrom(sockfd, (char *)buf, 65536, 0, &serverAddr, &addressSize);
	if (retVal == SOCKET_ERROR) { // There was an error
		perror("Couldn't receieve dns query result from socket");
		exit(1);
	}
	struct header* answerPacket = (struct header*)buf;

	// Parsing queryResult from answer packet
	struct hostent* queryResult = (struct hostent*)calloc(1, sizeof(struct hostent));
	if (queryResult == NULL) {
		perror("Can't allocate memory for query Result");
		exit(1);
	}
	parseAnswerFromAnswerPacket(queryResult);
	// queryResult = gethostbyname(domainName); TODO delete
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
		perror("Can't allocate memory for domainName");
		exit(1);
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
	domainToLowercase();

	while (strcmp(domainName, "quit") != 0) {
		// Validating domain name
		if (!isDomainNameValid()) {
			perror("Bad name");
		}
		else {
			// Running DNS query using socket
			struct hostent* queryResult = dnsQuery(domainName);
			resultIPAddress = queryResult->h_name;
			printf("%s\n", resultIPAddress);
		}
		printf("nsclient> ");
		retVal = scanf("%s", domainName);
		domainToLowercase();
	}

	// Closing socket and cleaning up WSA
	retVal = closesocket(sockfd);
	if (retVal != 0) {
		perror("Error at closesocket");
		exit(1);
	}

	retVal = WSACleanup();
	if (retVal != 0) {
		perror("Error at WSACleanup");
		exit(1);
	}

	return 0;
}