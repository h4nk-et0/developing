/*

	title		-> hank-client
	description	-> simple client used to connect to hank-server. used to download simple files, to pull the battery status and also used to start a chat 				   session with the server
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
#include<fcntl.h>

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

void decrypt_init(unsigned int *seq_no,unsigned int *hash,char *recv_data)
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

void main()
{

 struct sockaddr_in server;
 char ip_server[11],send_data[DATA_LEN],recv_data[DATA_LEN],command[15],passwd[PASSWD_LEN];
 int server_fd,send_byte,recv_byte,send_data_length,size,port;
 unsigned int hash[SEQ_LEN],loop,seq_no[SEQ_LEN],seq_no_rot[SEQ_LEN];
 unsigned short int connect_stat = 1;

// printf("enter server IP address : ");
// scanf("%[^\n]",ip_server);

// printf("enter the server port : ");
// scanf("%d",&port);
 strcpy(ip_server,"127.1.1.1");
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

 printf("\nenter a password for server : ");
 scanf(" %[^\n]",passwd);

 sha_1(passwd,hash);

 if((recv_byte = recv(server_fd,&recv_data,200,0))==-1)
 {
	perror("recv");
 }
 decrypt_init(seq_no,hash,recv_data);

 for(loop = 0;loop < SEQ_LEN;loop++)
	seq_no_rot[loop] = seq_no[SEQ_LEN - 1 - loop];

 encrypt_init(seq_no_rot,hash,send_data);

 send_data_length = strlen(send_data);
 if((send_byte = send(server_fd,&send_data,send_data_length,0))==-1)
	perror("send");

 if((recv_byte=recv(server_fd,&recv_data,1,0))==-1)
 {
	perror("recv");
 }
 if(recv_data[0] == ACK)
	connect_stat = 1;
 else if(recv_data[0] == FAIL)
	connect_stat = 0;

 if(connect_stat != 1)
 {
	printf("failed to login\nclosing the connection\n");
	exit(1);
 }

 printf("\n\n\x1b[32m\t -----------------------------------------------\x1b[0m\n");
 printf("\t\x1b[32m|\x1b[1m     \x1b[32mIf thEy d0nt want pe0plE insidE ...\x1b[0m\t\x1b[32m|\n");
 printf("\t\x1b[32m|\x1b[1m     \t\x1b[31mThEy 0uGht t0 build it bEttEr ...\x1b[0m\t\x1b[32m|\n");
 printf("\x1b[32m\t -----------------------------------------------\x1b[0m\n\n\n");

 while(1)
 {
	printf("\x1b[32m\x1b[1m\x1b[32mlocalhost@command : \x1b[0m");
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
			exit(1);
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
					exit(1);
				}
				if(!strcmp("quit",send_data))
				{
					printf("\rclosing send ....\n");
					close(server_fd);
					exit(1);
				}
			}
		}
		while(1)
		{
			if((recv_byte=recv(server_fd,&recv_data,200,0))==-1)
			{
				perror("recv");
				close(server_fd);
				exit(1);
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
			exit(1);
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
		exit(1);
	}
	else
		printf("\"%s\" : command not found\ntype help to list the commands\n",command);
 }
}
