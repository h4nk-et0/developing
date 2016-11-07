#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<fcntl.h>

void main()
{
 struct	sockaddr_in server;
 char	ip_server[11],
	send_data[200],
	recv_data[200],
	command[15];
 int	server_fd,
	send_byte,
	recv_byte,
	send_data_length,
	size,
	loop,
	port;

 printf("enter server IP address : ");
 scanf("%[^\n]",ip_server);

// printf("enter the server port : ");
// scanf("%d",&port);
// strcpy(ip_server,"127.1.1.1");
// strcpy(ip_server,"192.168.0.144");
 port=4444;

 server.sin_family=AF_INET;
 server.sin_port=htons(4444);
 inet_aton(ip_server,&(server.sin_addr));

 if((server_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
 {
	perror("socket");
	exit(1);
 }

 if(bind(server_fd,(struct sockaddr *)&server,sizeof(struct sockaddr))==-1)
 {
	//perror("bind");
	//exit(1);
 }
 printf("connecting to %s:%d ....\n",inet_ntoa(server.sin_addr),port);
 if(connect(server_fd,(struct sockaddr *)&server,sizeof(struct sockaddr))==-1)
 {
	perror("connect");
	exit(1);
 }
 printf("....connected\n");

 while(1)
 {
	printf("\r\x1b[30mlocalhost@command : \x1b[0m");
	scanf(" %s",command);
	if(!strcmp("help",command))
	{
		printf("commands\n");
		printf("\thelp\t\t- print this help\n\tbattery_status\t- obtain battery status of server\n\tchat\t\t- start a chat session with the server\n");
		printf("\tdownload \t- download a file from server\n\tquit\t\t- exit the command mode\n");
	}
	else if(!strcmp("battery_status",command))
	{
		send_data_length=strlen(command);
		if((send_byte=send(server_fd,&command,send_data_length,0))==-1)
			perror("send");
		if((recv_byte=recv(server_fd,&recv_data,200,0))==-1)
		{
			perror("recv");
			continue;
		}
		for(loop=recv_byte;loop<200;loop++)
			recv_data[loop]='\0';
		if(recv_data[0]=='\0')
		{
			printf("server is down\n");
			exit(0);
		}
		printf("%s : %s\n",inet_ntoa(server.sin_addr),recv_data);
	}
	else if(!strcmp("chat",command))
	{
		if((send_byte=send(server_fd,&command,4,0))==-1)
		{
			perror("chat");
			continue;
		}
		if(!fork())
		{
			while(1)
			{
				printf("\x1b[031mlocalhost : \x1b[0m");
				scanf(" %[^\n]",send_data);
				send_data_length=strlen(send_data);
				if((send_byte=send(server_fd,&send_data,send_data_length,0))==-1)
				{
					perror("send");
					close(server_fd);
					exit(0);
				}
				if(!strcmp("quit",send_data))
				{
					printf("\rclosing send ....\n");
					close(server_fd);
					exit(0);
				}
			}
		}
		while(1)
		{
			if((recv_byte=recv(server_fd,&recv_data,200,0))==-1)
			{
				perror("recv");
				close(server_fd);
				exit(0);
			}
			for(loop=recv_byte;loop<200;loop++)
				recv_data[loop]='\0';
			if(!strcmp("quit",recv_data)||recv_data[0]=='\0')
			{
				printf("\rclosing recv ....\n\x1b[31mlocalhost : \x1b[0m");
				fflush(stdout);
//				send_data_length=strlen(recv_data);
//				if(send(server_fd,&recv_data,send_data_length,0)==-1)
//					perror("send");
//				close(server_fd);
				break;
			}
			printf("\r%s : %s\n\x1b[031mlocalhost : \x1b[0m",inet_ntoa(server.sin_addr),recv_data);
			fflush(stdout);
		}
		wait();
		printf("\rclosing chat ....\n\n\r\x1b[30mlocalhost@command : \x1b[0m");
		fflush(stdout);
	}
	else if(!strcmp("download",command))
	{
		char new_file[100],file_data[1000];
		FILE *fp;
		send_data_length=strlen(command);
		if(send(server_fd,&command,send_data_length,0)==-1)
		{
			perror("download");
			continue;
		}
		printf("enter the source file path : ");
		scanf(" %[^\n]",send_data);
		printf("opening \"%s\" ....",send_data);
		send_data_length=strlen(send_data);
		if((send_byte=send(server_fd,&send_data,send_data_length,0))==-1)
			perror("send");
		int file_size,error_no,downloaded_size=0;
		float percent=0;
		if((recv_byte=recv(server_fd,&error_no,sizeof(error_no),0))==-1)
		{
			perror("recv");
			close(server_fd);
			exit(0);
		}
		error_no=ntohl(error_no);
		if(!error_no)
		{
			printf("\t\t\t[ok]\n");
			if(recv(server_fd,&file_size,sizeof(file_size),0)==-1)
				perror("recv");
			file_size=ntohl(file_size);
		}
		else
		{
			printf("\"%s\" : %s\n",send_data,strerror(error_no));
			continue;
		}
		printf("enter the dest file path : ");
		scanf(" %[^\n]",new_file);
		if((fp=fopen(new_file,"w"))==NULL)
		{
			perror("download");
			if(send(server_fd,"FAIL",4,0)==-1)
				perror("send");
			continue;
		}
		else
		{
			if(send(server_fd,"SUCCESS",7,0)==-1)
				perror("send");
			printf("downloading ....");
			while(1)
			{
				if((recv_byte=recv(server_fd,&size,sizeof(size),0))==-1)
					perror("recv");
				size=ntohl(size);
				if((recv_byte=recv(server_fd,&file_data,size,0))==-1)
					perror("recv");
				for(loop=recv_byte;loop<1000;loop++)
					file_data[loop]='\0';
				if(!strcmp("EOF",file_data))
					break;
				fprintf(fp,"%s",file_data);
				downloaded_size+=size;
				percent=(float)downloaded_size*100/file_size;
				printf("\r\t\t\t%f%%",percent);
			}
			fclose(fp);
			printf("\n%d Bytes downloaded\nwritten to %s\n",downloaded_size,new_file);
		}


	}
	else if(!strcmp("quit",command))
	{
		send_data_length=strlen(command);
		if(send(server_fd,&command,send_data_length,0)==-1)
			perror("send quit");
		printf("bye....\n");
		close(server_fd);
		exit(0);
	}
	else
		printf("\"%s\" : command not found\ntype help to list the commands\n",command);
 }
}
