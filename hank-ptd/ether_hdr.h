struct ether_hdr {
unsigned char	dst_mac_addr[6];
unsigned char	src_mac_addr[6];
unsigned short int eth_type;
};
