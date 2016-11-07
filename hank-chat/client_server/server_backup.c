#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<time.h>

void main()
{
 struct sockaddr_in server,client;
 time_t	current_time_raw;
 char	ip_address[11],
	send_data[200],
	recv_data[200],
	current_time[24];
 int	server_fd,
	client_fd,
	size,send_byte,
	recv_byte,
	loop,
	send_data_length,
	recv_data_length,
	port;
 FILE	*fp,
	*server_log;

 printf("enter the IP address : ");
 scanf("%[^\n]",ip_address);

// printf("enter the port to listen : ");
// scanf("%d",&port);
// strcpy(ip_address,"127.1.1.1");
// strcpy(ip_address,"192.168.0.144");
 port=4444;

 server.sin_family=AF_INET;
 server.sin_port=htons(port);
 inet_aton(ip_address,&(server.sin_addr));

 if((server_log=fopen("/var/log/hank_server.log","a"))==NULL)
 {
	perror("cannot open server_log file");
	exit(0);
 }
 current_time_raw=time(NULL);
 strncpy(current_time,ctime(&current_time_raw),24);
 printf("\n[%s] starting server at %s:%d  ....",current_time,inet_ntoa(server.sin_addr),port);
 fprintf(server_log,"\n        ***************************************************************                              \n");
 fprintf(server_log,"[%s] starting server at %s:%d  ....",current_time,inet_ntoa(server.sin_addr),port);
 if((server_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
 {
	printf("\t\t\t\t\t\tx1b[31m[fail]\x1b[0m\n");
	fprintf(server_log,"\t\t\t\t\t\tx1b[31m[fail]\x1b[0m\nsocket: %s\n",strerror(errno));
	perror("socket");
	exit(1);
 }

 if((bind(server_fd,(struct sockaddr *)&server,sizeof(struct sockaddr)))==-1)
 {
        printf("\t\t\t\t\t\t\x1b[31m[fail]\x1b[0m\n");
        fprintf(server_log,"\t\t\t\t\t\t\x1b[31m[fail]\x1b[0m\nbind: %s\n",strerror(errno));
	perror("bind");
	exit(1);
 }
 printf("\t\t\t\t\t\t[ok]\n");
 fprintf(server_log,"\t\t\t\t\t\t[ok]\n");
 current_time_raw=time(NULL);
 strncpy(current_time,ctime(&current_time_raw),24);
 printf("[%s] listening for connection ....\n",current_time);
 fprintf(server_log,"[%s] listening for connection ....\n",current_time);
 fflush(server_log);
 if(listen(server_fd,1)==-1)
 {
	perror("listen");
	exit(1);
 }

 while(1)
 {
	size=sizeof(struct sockaddr_in);
	if((client_fd=accept(server_fd,(struct sockaddr *)&client,&size))==-1)
	{
		perror("accept");
		continue;
	}
	current_time_raw=time(NULL);
	strncpy(current_time,ctime(&current_time_raw),24);
	printf("\n\r[%s] Got a connection from %s\n",current_time,inet_ntoa(client.sin_addr));
	fprintf(server_log,"\n\r[%s] Got a connection from %s\n",current_time,inet_ntoa(client.sin_addr));
 	fclose(server_log);

	if(!fork())
	{
		if((server_log=fopen("/var/log/hank_server.log","a"))==NULL)
			perror("cannot reopen server log file");
		close(server_fd);
		while(1)
		{
			if((recv_byte=recv(client_fd,&recv_data,200,0))==-1)
			{
				perror("recv");
				close(client_fd);
				exit(0);
			}
			for(loop=recv_byte;loop<200;loop++)
				recv_data[loop]='\0';
			if(!strcmp("battery_status",recv_data))
			{
				FILE *battery;
				char next_line[40],power_percentage[3];
				if((battery=fopen("/sys/class/power_supply/BAT1/uevent","r"))==NULL)
				{
					perror("error");
					continue;
				}
				fseek(battery,43,SEEK_SET);
				fscanf(battery,"%s",send_data);
				for(loop=0;loop<9;loop++)
					fscanf(battery,"%s",next_line);
				fseek(battery,23,SEEK_CUR);
				fscanf(battery,"%[^\n]",power_percentage);
				fclose(battery);
				strcat(send_data," ");
				strcat(send_data,power_percentage);
				strcat(send_data,"%");
				send_data_length=strlen(send_data);
				if(send(client_fd,&send_data,send_data_length,0)==-1)
					perror("send");
				current_time_raw=time(NULL);
				strncpy(current_time,ctime(&current_time_raw),24);
				printf("[%s] battery status sent to %s\n",current_time,inet_ntoa(client.sin_addr));
				fprintf(server_log,"[%s] battery status sent to %s\n",current_time,inet_ntoa(client.sin_addr));
				fflush(server_log);
			}
			else if(!strcmp("chat",recv_data))
			{
				if(!fork())
				{
					close(server_fd);
					while(1)
					{
						printf("\x1b[31mserver : \x1b[0m");
						scanf(" %[^\n]",send_data);
						send_data_length=strlen(send_data);
						if(send(client_fd,&send_data,send_data_length,0)==-1)
						{
							perror("send");
							current_time_raw=time(NULL);
							strncpy(current_time,ctime(&current_time_raw),24);
							printf("\r[%s] closing send ....\n",current_time);
							fprintf(server_log,"\r[%s] closing send ....\n",current_time);
							fflush(server_log);
							close(client_fd);
								exit(0);
						}
						if(!strcmp("quit",send_data))
						{
							current_time_raw=time(NULL);
							strncpy(current_time,ctime(&current_time_raw),24);
							printf("\r[%s] closing send ....\n",current_time);
							close(client_fd);
							exit(0);
						}
					}
				}
				while(1)
				{
					if((recv_byte=recv(client_fd,&recv_data,200,0))==-1)
					{
						perror("recv");
						close(client_fd);
						exit(0);
					}
					for(loop=recv_byte;loop<200;loop++)
						recv_data[loop]='\0';
					if(!strcmp("quit",recv_data) || recv_data[0]=='\0')
					{
						current_time_raw=time(NULL);
						strncpy(current_time,ctime(&current_time_raw),24);
						printf("\r[%s] closing recv ....\n\x1b[31mserver : \x1b[0m",current_time);
						fflush(stdout);
						fprintf(server_log,"\r[%s] closing recv ....\n",current_time);
						fflush(server_log);
//						send_data_length=strlen(recv_data);
//						if(send(client_fd,&recv_data,send_data_length,0)==-1)
//							perror("send");
						break;
					}
					printf("\r%s : %s\n\x1b[31mserver : \x1b[0m",inet_ntoa(client.sin_addr),recv_data);
					fflush(stdout);
				}
				if(recv_data[0]!='\0')
					wait();
				current_time_raw=time(NULL);
				strncpy(current_time,ctime(&current_time_raw),24);
				printf("\r[%s] closing chat ....\n",current_time);
				fprintf(server_log,"\r[%s] closing chat ....\n",current_time);
				fflush(server_log);
			}
			else if(!strcmp("download",recv_data))
			{
				char input_file[100],file_data[1000];
				if((recv_byte=recv(client_fd,&recv_data,200,0))==-1)
					perror("recv");
				for(loop=recv_byte;loop<200;loop++)
					recv_data[loop]='\0';
				char data[20000],file_name[100];
				int file_size,error_no;
				strcpy(file_name,recv_data);
				errno=0;
				fp=fopen(recv_data,"r");
				error_no=htonl(errno);
				if(error_no)
				{
					if(send(client_fd,&error_no,sizeof(error_no),0)==-1)
					{
						perror("send");
						current_time_raw=time(NULL);
						strncpy(current_time,ctime(&current_time_raw),24);
						printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
						fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
						fflush(server_log);
						if(close(client_fd)==-1)
							perror("close");
						exit(0);
					}
					continue;
				}
				else if(!error_no)
				{
					if(send(client_fd,&error_no,sizeof(error_no),0)==-1)
					{
						perror("send");
						current_time_raw=time(NULL);
						strncpy(current_time,ctime(&current_time_raw),24);
						printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
						fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
						fflush(server_log);
						if(close(client_fd)==-1)
							perror("close");
						exit(0);
					}
					fseek(fp,0,SEEK_END);
					file_size=htonl(ftell(fp));
					if(send(client_fd,&file_size,sizeof(file_size),0)==-1)
					{
						perror("send");
						current_time_raw=time(NULL);
						strncpy(current_time,ctime(&current_time_raw),24);
						printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
						fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
						fflush(server_log);
						if(close(client_fd)==-1)
							perror("close");
						exit(0);
					}
				}
				if((recv_byte=recv(client_fd,&recv_data,200,0))==-1)
				{
					perror("recv");
					current_time_raw=time(NULL);
					strncpy(current_time,ctime(&current_time_raw),24);
					printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
					fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
					fflush(server_log);
					if(close(client_fd)==-1)
						perror("close");
					exit(0);
				}
				for(loop=recv_byte;loop<200;loop++)
					recv_data[loop]='\0';
				if(!strcmp("FAIL",recv_data))
					continue;
				else if(!strcmp("SUCCESS",recv_data))
				{
					fseek(fp,0,SEEK_SET);
					int file_size_sent=ntohl(file_size);
					file_size=0;
					while(1)
					{
						fgets(file_data,1000,fp);
						//if(feof(fp))
						if(file_size==file_size_sent)
						{
							send_data_length=htonl(3);
							if(send(client_fd,&send_data_length,sizeof(send_data_length),0)==-1)
							{
								perror("send");
								current_time_raw=time(NULL);
								strncpy(current_time,ctime(&current_time_raw),24);
								printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
								fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
								fflush(server_log);
								if(close(client_fd)==-1)
									perror("close");
								exit(0);
							}
							if(send(client_fd,"EOF",3,0)==-1)
							{
								perror("send");
								current_time_raw=time(NULL);
								strncpy(current_time,ctime(&current_time_raw),24);
								printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
								fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
								fflush(server_log);
								if(close(client_fd)==-1)
									perror("close");
								exit(0);
							}
							break;
						}
						send_data_length=strlen(file_data);
						file_size+=send_data_length;
						send_data_length=htonl(send_data_length);
						if(send(client_fd,&send_data_length,sizeof(send_data_length),0)==-1)
						{
							perror("send");
							current_time_raw=time(NULL);
							strncpy(current_time,ctime(&current_time_raw),24);
							printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
							fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
							fflush(server_log);
							if(close(client_fd)==-1)
								perror("close");
							exit(0);
						}
						send_data_length=ntohl(send_data_length);
						if(send(client_fd,&file_data,send_data_length,0)==-1)
						{
							perror("send");
							current_time_raw=time(NULL);
							strncpy(current_time,ctime(&current_time_raw),24);
							printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
							fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
							fflush(server_log);
							if(close(client_fd)==-1)
								perror("close");
							exit(0);
						}
					}
					fclose(fp);
					current_time_raw=time(NULL);
					strncpy(current_time,ctime(&current_time_raw),24);
					printf("[%s] \"%s\" (%d Bytes) sent to %s\n",current_time,file_name,file_size,inet_ntoa(client.sin_addr));
					fprintf(server_log,"[%s] \"%s\" (%d Bytes) sent to %s\n",current_time,file_name,ntohl(file_size),inet_ntoa(client.sin_addr));
					fflush(server_log);
				}
			}
			else if(!strcmp("quit",recv_data))
			{
				current_time_raw=time(NULL);
				strncpy(current_time,ctime(&current_time_raw),24);
				printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
				fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
				fflush(server_log);
				if(close(client_fd)==-1)
					perror("close");
				exit(0);
			}
			if(recv_data[0]=='\0')
			{
				current_time_raw=time(NULL);
				strncpy(current_time,ctime(&current_time_raw),24);
				printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
				fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
				fflush(server_log);
				if(close(client_fd)==-1)
					perror("close");
				exit(0);
			}
		}
	}
	wait();
	if((server_log=fopen("/var/log/hank_server.log","a"))==NULL)
		perror("cannot reopen server log file");
 }
}
