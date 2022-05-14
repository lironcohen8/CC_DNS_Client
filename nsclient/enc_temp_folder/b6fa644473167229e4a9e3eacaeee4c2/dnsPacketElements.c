#include <stdint.h>

struct header {
	uint16_t ID;
	unsigned char QR : 1;
	unsigned char OPCODE : 4;
	unsigned char AA : 1;
	unsigned char TC : 1;
	unsigned char RD : 1;
	unsigned char RA : 1;
	unsigned char Z : 3;
	unsigned char RCODE : 4;
	uint16_t QDCOUNT;
	uint16_t ANCOUNT;
	uint16_t NSCOUNT;
	uint16_t ARCOUNT;
};

struct question {
	uint16_t QTYPE;
	uint16_t QCLASS;
};

struct r_data {
	uint16_t TYPE;
	uint16_t CLASS;
	uint32_t TTL;
	uint16_t RDLENGTH;
};

struct res_record {
	uint16_t NAME_REF;
	struct r_data RESOURCE;
	unsigned char* RDATA;
};

struct query {
	unsigned char* NAME;
	struct question* QUESTION;
};

//struct dnsQueryPacket {
//	struct header header;
//	struct question question;
//};
//
//struct dnsAnswerPacket {
//	struct header header;
//	struct answer answer;
//};