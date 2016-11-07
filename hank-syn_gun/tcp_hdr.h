struct tcp_hdr {
unsigned short int src_port,dst_port;
unsigned int seq_no,ack_no;
unsigned short int offset,window,chksm,u_ptr;
};
