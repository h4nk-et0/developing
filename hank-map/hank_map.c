#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

void disp_usage();

void main(int argc,char *argv[])
{
 struct sockaddr_in host;
 int	p_flag=0,
	c_flag=0,
	s_flag=0,
	host_fd,
	port=0;

 if(argc < 2 || argc > 5)
 {
	disp_usage();
	exit(0);
 }

 while(--argc)
 {
	if(argv[argc][0]=='-')
	{
		if(strlen(argv[argc])!=2)
			goto error;
		else if(argv[argc][1]=='p' && !p_flag && !c_flag && !s_flag)
			p_flag=1;
		else if(argv[argc][1]=='c' && !p_flag && !c_flag && !s_flag)
			c_flag=1;
		else if(argv[argc][1]=='s' && !p_flag && !c_flag && !s_flag)
			s_flag=1;
		else
		{
			error:
			disp_usage();
			exit(0);
		}
	}
	else
	{
		disp_usage();
		exit(0);
	}
 }

 host.sin_family=AF_INET;
 inet_aton("127.0.0.1",&(host.sin_addr));
 if((host_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
 {
	perror("socket");
	exit(1);
 }
 if(bind(host_fd,(struct sockaddr *)&host,sizeof(struct sockaddr))==-1)
 {
	perror("bind");
	exit(1);
 }

 if(c_flag)
 {
	for(port=1;port<1000;port++)
	{
		host.sin_port=htons(port);
		if(connect(host_fd,(struct sockaddr *)&host,sizeof(struct sockaddr))==0)
		{
			perror("connect");
			printf("%d\n",port);
		}
	}
 }
}

void disp_usage()
{
	printf("usage: hank-map [options] [IP Address]\n\n");
	printf("-p\t- ping scan for host discovery (noisy)\n-c\t- complete connect scan\n-s\t- stealth scan (syn,syn/ack,rst)\n");
}
