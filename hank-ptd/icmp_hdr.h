struct icmp_hdr {
unsigned char type;
unsigned char code;
unsigned short int checksum;
unsigned short int id;
unsigned short int seq;
};
