struct header {
	unsigned short ID;
	unsigned char QR : 1;
	unsigned char OPCODE : 4;
	unsigned char AA : 1;
	unsigned char TC : 1;
	unsigned char RD : 1;
	unsigned char RA : 1;
	unsigned char Z : 3;
	unsigned char RCODE : 4;
	unsigned short QDCOUNT;
	unsigned short ANCOUNT;
	unsigned short NSCOUNT;
	unsigned short ARCOUNT;
};

struct question {
	char* QNAME;
	unsigned short QTYPE;
	unsigned short QCLASS;
};

struct answer {
	char* NAME;
	unsigned short TYPE;
	unsigned short CLASS;
	unsigned int TTL;
	unsigned short RDLENGTH;
	char* RDATA;
};

struct dnsQueryPacket {
	struct header header;
	struct question question;
};

struct dnsAnswerPacket {
	struct header header;
	struct answer answer;
};