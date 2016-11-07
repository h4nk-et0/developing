/*

	title		-> hank-server
	description	-> a simple tcp based server used to download simple text files from the hosted computer, to know battery level of the hosted computer 				   and to start a chat session with a client
	author		-> hank

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<time.h>

#define PASSWD_LEN	500
#define DATA_LEN	40
#define SEQ_LEN		5
#define ACK		0x08
#define FAIL		0x09

void encrypt_init(unsigned int *seq_no, unsigned int *hash, char *send_data)
{
	unsigned int *encrypt;
	unsigned short int loop;

	encrypt = (unsigned int *)send_data;
	for(loop = 0;loop < SEQ_LEN;loop++)
		encrypt[loop] = seq_no[loop] ^ hash[loop];

}

void decrypt_init(unsigned int *seq_no, unsigned int *hash, char *recv_data)
{
	unsigned int *decrypt;
	unsigned short int loop;

	decrypt = (unsigned int *)recv_data;
	for(loop = 0;loop < SEQ_LEN;loop++)
		seq_no[loop] = decrypt[loop] ^ hash[loop];
}

void sha_1(char *passwd, unsigned int *hash)
{
	unsigned short int loop,in_loop;
	unsigned int inter_hash[SEQ_LEN],inter_const[3],msg_digest[PASSWD_LEN],size = 0;

	for(loop = strlen(passwd);loop < PASSWD_LEN;loop++)
		passwd[loop]='\0',msg_digest[loop] = 0;

	in_loop = 0;
	loop = 0;
	while(1)
	{
		msg_digest[in_loop] = (msg_digest[in_loop] << 8) + (int)passwd[loop];

		loop++;
		if(loop % 4 == 0)
			in_loop++;
		if(loop == strlen(passwd))
		{
			int temp = 0;
			if(loop % 4 == 0)
				in_loop++;
			temp = loop;
			loop = 0;
			msg_digest[in_loop] = (msg_digest[in_loop] << 4) + 8;
			loop = loop + 4;

			while((temp * 8 + loop) % 32 != 0)
			{
				msg_digest[in_loop] = (msg_digest[in_loop] << 1) + 0;
				loop++;
			}
			break;
		}
	}

	for(loop = in_loop + 1;loop < 13;loop++)
		msg_digest[loop] = 0;

	size = 8 * strlen(passwd);
	msg_digest[14] = (0xffff0000) & (size);
	msg_digest[15] = (0x0ffff) & (size);

	for(loop = 16;loop < 80;loop++)
	{
		msg_digest[loop] = ((msg_digest[loop-3]) ^ (msg_digest[loop - 8]) ^ (msg_digest[loop - 14]) ^ (msg_digest[loop - 16]));
		msg_digest[loop] = ((msg_digest[loop] << 1) | (msg_digest[loop] >> 31));
	}

	hash[0]=0x67452301;
	hash[1]=0xefcdab89;
	hash[2]=0x98badcfe;
	hash[3]=0x10325476;
	hash[4]=0xc3d2e1f0;

	inter_hash[0] = hash[0];
	inter_hash[1] = hash[1];
	inter_hash[2] = hash[2];
	inter_hash[3] = hash[3];
	inter_hash[4] = hash[4];

	for(loop = 0;loop < 80;loop++)
	{
		if(loop <= 19)
		{
			inter_const[0] = (inter_hash[1] & inter_hash[2]) | ((~inter_hash[1]) & inter_hash[3]);
			inter_const[1] = 0x5a827999;
		}
		else if(loop <= 39)
		{
			inter_const[0] = inter_hash[1] ^ inter_hash[2] ^ inter_hash[3];
			inter_const[1] = 0x6ed9eba1;
		}
		else if(loop <= 59)
		{
			inter_const[0] = (inter_hash[1] & inter_hash[2]) | (inter_hash[1] & inter_hash[3]) | (inter_hash[2] & inter_hash[3]);
			inter_const[1] = 0x8f1bbcdc;
		}
		else if(loop <= 79)
		{
			inter_const[0] = inter_hash[1] ^ inter_hash[2] ^ inter_hash[3];
			inter_const[1] = 0xca62c1d6;
		}
		inter_const[2] = (((inter_hash[0] << 5) | (inter_hash[0] >> 27)) + inter_const[0] + inter_hash[4] + inter_const[1] + msg_digest[loop]);
		inter_hash[4] = inter_hash[3];
		inter_hash[3] = inter_hash[2];
		inter_hash[2] = ((inter_hash[1] << 30) | (inter_hash[1] >> 2));
		inter_hash[1] = inter_hash[0];
		inter_hash[0] = inter_const[2];
		}

	hash[0] += inter_hash[0];
	hash[1] += inter_hash[1];
	hash[2] += inter_hash[2];
	hash[3] += inter_hash[3];
	hash[4] += inter_hash[4];

}

void encrypt()
{
}

void decrypt()
{
}

void main()
{

 struct sockaddr_in server,client;
 time_t current_time_raw;
 char ip_address[15],send_data[DATA_LEN],recv_data[DATA_LEN],current_time[24],passwd[PASSWD_LEN];
 unsigned short int msg_len,connect = 1;
 int server_fd,client_fd,size = 0,recv_data_len,send_data_len,port;
 unsigned int loop,in_loop,seq_no[SEQ_LEN],hash[SEQ_LEN],seq_no_rot[SEQ_LEN];
 pid_t pid_con_hand,pid_chat_hand,pid_auth_hand;
 FILE *fp,*server_log,*conf;

/* if(INADDR_ANY==0)
 {
	printf("device not connected to a network\n");
	exit(1);
 }*/

 if((server_log = fopen("/etc/project_hank/hank_server.log","a"))==NULL)
 {
	if(errno == 2)
	{
		if((conf = fopen("/etc/project_hank/hank_server.log","w+")) == NULL)
		{
			perror("cannot open server log file");
			exit(1);
		}
	}
	else
	{
		perror("cannot open server log file");
		exit(1);
	}
 }

 if((conf = fopen("/etc/project_hank/hank_server.conf","r+")) == NULL)
 {
	if(errno == 2)
	{
		if((conf = fopen("/etc/project_hank/hank_server.conf","w+")) == NULL)
		{
			perror("cannot open /etc/project_hank/hank_server.conf file");
			exit(1);
		}
		printf("one time configuration\n");
		printf("enter a number (within 10 digits) : ");
		scanf("%d",&seq_no[0]);

		fprintf(conf,"seq_no =");
		for(loop = 0;loop < 5;loop++)
			fprintf(conf," %d",seq_no[0]);
	}
	else
	{
		perror("cannot open /etc/project_hank/hank_server.conf file");
		exit(1);
	}
 }

// printf("enter the IP address : ");
// scanf("%[^\n]",ip_address);
 strcpy(ip_address,"127.1.1.1");

 port = 4444;

 server.sin_family = AF_INET;
 server.sin_port = htons(port);
 inet_aton(ip_address,&(server.sin_addr));

 printf("enter a password for server : ");
 scanf(" %[^\n]",passwd);

 sha_1(passwd,hash);

 current_time_raw = time(NULL);
 strncpy(current_time,ctime(&current_time_raw),24);
 printf("\n[%s] starting server at %s:%d  ...",current_time,inet_ntoa(server.sin_addr),port);
 fprintf(server_log,"\n        ***************************************************************\n");
 fprintf(server_log,"[%s] starting server at %s:%d  ....",current_time,inet_ntoa(server.sin_addr),port);

 if((server_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
 {
	printf("\x1b[31m[fail]\x1b[0m\n");
	fprintf(server_log,"\x1b[31m[fail]\x1b[0m\nsocket: %s\n",strerror(errno));
	perror("socket");
	exit(1);
 }

 if((bind(server_fd,(struct sockaddr *)&server,sizeof(struct sockaddr))) == -1)
 {
        printf("\x1b[31m[fail]\x1b[0m\n");
        fprintf(server_log,"\x1b[31m[fail]\x1b[0m\nbind: %s\n",strerror(errno));
	perror("bind");
	exit(1);
 }

 printf("[ok]\n");
 fprintf(server_log,"[ok]\n");

 printf("\n\n\x1b[32m\t -----------------------------------------------\x1b[0m\n");
 printf("\t\x1b[32m|\x1b[1m     \x1b[32mIf thEy d0nt want pe0plE insidE ...\x1b[0m\t\x1b[32m|\n");
 printf("\t\x1b[32m|\x1b[1m     \t\x1b[31mThEy 0uGht t0 build it bEttEr ...\x1b[0m\t\x1b[32m|\n");
 printf("\x1b[32m\t -----------------------------------------------\x1b[0m\n\n\n");

 current_time_raw = time(NULL);
 strncpy(current_time,ctime(&current_time_raw),24);
 printf("[%s] listening for connection ...\n",current_time);
 fprintf(server_log,"[%s] listening for connection ...\n",current_time);
 fflush(server_log);

 if(listen(server_fd,5) == -1)
 {
	perror("listen");
	exit(1);
 }

 pid_con_hand = getpid();

 if(!(pid_chat_hand = fork()))
 {

pause();

		while(1)
		{
			if((recv_data_len = recv(client_fd,&recv_data,DATA_LEN,0))==-1)
			{
				perror("recv");
				close(client_fd);
				exit(1);
			}
			for(loop = recv_data_len;loop < DATA_LEN;loop++)
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

				send_data_len=strlen(send_data);
				if(send(client_fd,&send_data,send_data_len,0)==-1)
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
						send_data_len=strlen(send_data);

						if(send(client_fd,&send_data,send_data_len,0)==-1)
						{
							perror("send");
							current_time_raw=time(NULL);
							strncpy(current_time,ctime(&current_time_raw),24);
							printf("\r[%s] closing send ....\n",current_time);
							fprintf(server_log,"\r[%s] closing send ....\n",current_time);
							fflush(server_log);
							close(client_fd);
								exit(1);
						}

						if(!strcmp("quit",send_data))
						{
							current_time_raw=time(NULL);
							strncpy(current_time,ctime(&current_time_raw),24);
							printf("\r[%s] closing send ....\n",current_time);
							close(client_fd);
							exit(1);
						}
					}
				}

				while(1)
				{
					if((recv_data_len=recv(client_fd,&recv_data,200,0))==-1)
					{
						perror("recv");
						close(client_fd);
						exit(1);
					}
					for(loop=recv_data_len;loop<200;loop++)
						recv_data[loop]='\0';

					if(!strcmp("quit",recv_data) || recv_data[0]=='\0')
					{
						current_time_raw=time(NULL);
						strncpy(current_time,ctime(&current_time_raw),24);
						printf("\r[%s] closing recv ....\n\x1b[31mserver : \x1b[0m",current_time);
						fflush(stdout);
						fprintf(server_log,"\r[%s] closing recv ....\n",current_time);
						fflush(server_log);
//						send_data_len=strlen(recv_data);
//						if(send(client_fd,&recv_data,send_data_len,0)==-1)
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
				char input_file[100],file_data[1000],data[20000],file_name[100];
				int file_size,error_no;

				if((recv_data_len=recv(client_fd,&recv_data,200,0))==-1)
					perror("recv");
				for(loop=recv_data_len;loop<200;loop++)
					recv_data[loop]='\0';

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
						exit(1);
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
						exit(1);
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
						exit(1);
					}
				}

				if((recv_data_len=recv(client_fd,&recv_data,200,0))==-1)
				{
					perror("recv");
					current_time_raw=time(NULL);
					strncpy(current_time,ctime(&current_time_raw),24);
					printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
					fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
					fflush(server_log);

					if(close(client_fd)==-1)
						perror("close");
					exit(1);
				}
				for(loop=recv_data_len;loop<200;loop++)
					recv_data[loop]='\0';

				if(!strcmp("FAIL",recv_data))
					continue;

				else if(!strcmp("SUCCESS",recv_data))
				{
					int file_size_sent=ntohl(file_size);

					fseek(fp,0,SEEK_SET);
					file_size=0;

					while(1)
					{
						fgets(file_data,1000,fp);
						//if(feof(fp))
						if(file_size==file_size_sent)
						{
							send_data_len=htonl(3);
							if(send(client_fd,&send_data_len,sizeof(send_data_len),0)==-1)
							{
								perror("send");
								current_time_raw=time(NULL);
								strncpy(current_time,ctime(&current_time_raw),24);
								printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
								fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
								fflush(server_log);

								if(close(client_fd)==-1)
									perror("close");
								exit(1);
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
								exit(1);
							}
							break;
						}

						send_data_len=strlen(file_data);
						file_size+=send_data_len;
						send_data_len=htonl(send_data_len);

						if(send(client_fd,&send_data_len,sizeof(send_data_len),0)==-1)
						{
							perror("send");
							current_time_raw=time(NULL);
							strncpy(current_time,ctime(&current_time_raw),24);
							printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
							fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
							fflush(server_log);

							if(close(client_fd)==-1)
								perror("close");
							exit(1);
						}

						send_data_len=ntohl(send_data_len);
						if(send(client_fd,&file_data,send_data_len,0)==-1)
						{
							perror("send");
							current_time_raw=time(NULL);
							strncpy(current_time,ctime(&current_time_raw),24);
							printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
							fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
							fflush(server_log);

							if(close(client_fd)==-1)
								perror("close");
							exit(1);
						}
					}

					fclose(fp);
					current_time_raw=time(NULL);
					strncpy(current_time,ctime(&current_time_raw),24);
					printf("[%s] \"%s\" (%d Bytes) sent to %s\n",current_time,file_name,file_size,inet_ntoa(client.sin_addr));
					fprintf(server_log,"[%s] \"%s\" (%d Bytes) sent to %s\n",current_time,file_name,file_size,inet_ntoa(client.sin_addr));
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
				exit(1);
			}
/*char a[5];
strncpy(a,recv_data,3);
if(!strcmp("GET",a))
printf("%s\n",recv_data);*/

			if(recv_data[0]=='\0')
			{
				current_time_raw = time(NULL);
				strncpy(current_time,ctime(&current_time_raw),24);
				printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
				fprintf(server_log,"\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
				fflush(server_log);

				if(close(client_fd)==-1)
					perror("close");
				exit(1);
			}
		}
//	wait();

	if((server_log=fopen("/etc/project_hank/hank_server.log","a"))==NULL)
		perror("cannot reopen server log file");


 }
 while(1)
 {
	size = sizeof(struct sockaddr_in);
	if((client_fd = accept(server_fd,(struct sockaddr *)&client,&size)) == -1)
	{
		perror("accept");
		continue;
	}

	current_time_raw=time(NULL);
	strncpy(current_time,ctime(&current_time_raw),24);
	printf("\n\r[%s] Got a connection from %s\n",current_time,inet_ntoa(client.sin_addr));
	fprintf(server_log,"\n\r[%s] Got a connection from %s\n",current_time,inet_ntoa(client.sin_addr));
 	fclose(server_log);

	fseek(conf,9,SEEK_SET);
	for(loop = 0;loop < SEQ_LEN;loop++)
	{
		fscanf(conf,"%d",&seq_no[loop]);
		seq_no[loop] += rand();
	}

	remove("/etc/project_hank/hank_server.conf");
	if((conf = fopen("/etc/project_hank/hank_server.conf","w+")) == NULL)
	{
		perror("cannot open /etc/project_hank/hank_server.conf file");
		exit(1);
	}

	fprintf(conf,"seq_no =");
	for(loop = 0;loop < SEQ_LEN;loop++)
		fprintf(conf," %d",seq_no[loop]);

	if(!(pid_auth_hand = fork()))
	{
		if((server_log = fopen("/etc/project_hank/hank_server.log","a")) == NULL)
			perror("cannot reopen server log file");

		close(server_fd);
		fclose(conf);

		for(loop = 0;loop < DATA_LEN;loop++)
			send_data[loop] = 0;

		encrypt_init(seq_no,hash,send_data);

		send_data_len = strlen(send_data);
		if(send(client_fd,&send_data,send_data_len,0)==-1)
			perror("send");

		for(loop = 0;loop < SEQ_LEN;loop++)
			seq_no_rot[loop] = seq_no[SEQ_LEN -1 - loop];

		if((recv_data_len = recv(client_fd,&recv_data,DATA_LEN,0)) == -1)
		{
			perror("failed to establish a secure connection (recv) ");
			close(client_fd);
			exit(1);
		}

		decrypt_init(seq_no,hash,recv_data);

		for(loop = 0;loop < SEQ_LEN;loop++)
		{
			if(seq_no_rot[loop] != seq_no[loop])
			{
				connect = 0;
				break;
			}
		}

		if(connect != 1)
		{
			printf("failed login attempt from %s\n",inet_ntoa(client.sin_addr));
			send_data[0] = FAIL;
			if(send(client_fd,&send_data,1,0)==-1)
				perror("send");
			printf("\r[%s] closing connection to %s ....\n",current_time,inet_ntoa(client.sin_addr));
			exit(1);
		}

		else
		{
			send_data[0] = ACK;
			if(send(client_fd,&send_data,1,0)==-1)
			{
				perror("send");
				exit(1);
			}
		}

	}

 }
}
