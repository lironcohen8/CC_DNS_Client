#define _WINSOCK_DEPRECATED_NO_WARNINGS // Used for enabling usage of functions like gethostbyname in vs
#define _CRT_SECURE_NO_WARNINGS // Used for enabling usage of functions like fopen in vs
#define TRUE 1
#define FALSE 0

#include <stdio.h>
#include <math.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <ctype.h>

#pragma comment(lib, "ws2_32.lib") // Used to link the library Ws2_32.lib for windows socketing
#pragma pack(push, 1)

struct header {
	uint32_t ID : 16;

	uint32_t RD : 1;
	uint32_t TC : 1;
	uint32_t AA : 1;
	uint32_t OPCODE : 4;
	uint32_t QR : 1;

	uint32_t RCODE : 4;
	uint32_t Z : 3;
	uint32_t RA : 1;

	/*unsigned char QR : 1;
	unsigned char OPCODE : 4;
	unsigned char AA : 1;
	unsigned char TC : 1;
	unsigned char RD : 1;
	unsigned char RA : 1;
	unsigned char Z : 3;
	unsigned char RCODE : 4;*/

	/*unsigned char RCODE : 4;
	unsigned char Z : 3;
	unsigned char RA : 1;
	unsigned char RD : 1;
	unsigned char TC : 1;
	unsigned char AA : 1;
	unsigned char OPCODE : 4;
	unsigned char QR : 1;*/

	uint16_t QDCOUNT;
	uint16_t ANCOUNT;
	uint16_t NSCOUNT;
	uint16_t ARCOUNT;
};

struct question {
	uint16_t QTYPE : 16;
	uint16_t QCLASS : 16;
};

struct r_data {
	uint16_t TYPE;
	uint16_t CLASS;
	uint32_t TTL;
	uint16_t RDLENGTH;
};

struct res_record {
	unsigned char* NAME;
	struct r_data* RESOURCE;
	unsigned char* RDATA;
};

struct query {
	unsigned char* NAME;
	struct question* QUESTION;
};

#pragma pack(pop)