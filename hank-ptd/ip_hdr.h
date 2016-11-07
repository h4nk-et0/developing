struct ip_hdr {
unsigned char ip_hl:4,ip_ver:4;
unsigned char tos;
unsigned short int length;
unsigned short int id;
unsigned short int offset;
unsigned char ttl;
unsigned char protocol;
unsigned short int chksm;
unsigned int ip_src,ip_dst;
};
