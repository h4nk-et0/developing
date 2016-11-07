#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define APPNAME_LEN	BUF_LEN
#define BUF_LEN		20
#define CODE_LEN	1
#define HASH_LEN	5
#define IP_LEN		16
#define PORT_NUM	4444
#define SEQ_LEN		5
#define PASS_LEN	52
#define USERNAME_LEN	PASS_LEN
#define VER_LEN		BUF_LEN

/* codes for communication */

#define FAIL_LOGIN	0x10	/* login failed */
#define SUCCESS_LOGIN	0x11	/* login success */
#define USER_FAIL	0x12	/* invalid username */
#define USER_OK		0x13	/* valid username */
#define USER_PROMPT	0x14	/* prompt to give username */
#define	USER_CREATE	0x16	/* sign-up session */
#define	USER_LOGIN	0x17	/* login session */

#include "sha_1.h"
#include "encrypt.h"

const char arg_sign[] = "sign-up";

void main(int argc,char *argv[])
{

 struct sockaddr_in	server_addr,
			client_addr;
 char	ip_addr[IP_LEN],
	client_pass[PASS_LEN],
	app_name[APPNAME_LEN],
	version[VER_LEN],
	username[USERNAME_LEN];
 unsigned char	client_buf[BUF_LEN],
		sign_up;
 int	server_sock;
 unsigned int	client_hash[HASH_LEN],
		client_seq_no[SEQ_LEN],
		*encrypt_seq,
		rot_seq_no[SEQ_LEN];
 unsigned short int	loop,
			in_loop,
			send_byte,
			recv_byte;
 socklen_t	addr_len;

 /* if no arguments, then it is a login session */

 if(argc < 2)
 {
	sign_up = 0;
 }

 /* expects only one argument (sign-up) */

 else if(argc == 2)
 {
	sign_up = 0;
	for(loop = 0;loop < strlen(arg_sign) + 1;loop++)
	{
		if(arg_sign[loop] == argv[1][loop])
		{
			if(arg_sign[loop] == '\0' && argv[1][loop] == '\0')
			{
				sign_up = 1;
				break;
			}

		}

		else
		{
			printf("invalid argument '%s' ...\n",argv[1]);
			printf("usage : hank-chat_server [sign-up]\n");
			exit(1);
		}
	}
 }

 else
 {
	printf("invalid arguments ...\n");
	printf("usage : hank-chat_server [sign-up]\n");
	exit(1);
 }

 printf("enter the IP address : ");
 for(loop = 0;loop < IP_LEN;loop++)
 {
	if((ip_addr[loop] = getc(stdin)) == EOF)
		break;

	if(ip_addr[loop] == '\n')
		break;
 }

 if(loop == IP_LEN)
	ip_addr[loop - 1] = '\0';

 else
	ip_addr[loop] = '\0';

 server_addr.sin_family = AF_INET;
 server_addr.sin_port = htons(PORT_NUM);
 inet_aton(ip_addr,&(server_addr.sin_addr));

 if((server_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
 {
	perror("socket ");
	exit(1);
 }

 if(connect(server_sock,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
 {
	perror("connect ");
	exit(1);
 }

 encrypt_seq = (unsigned int *)client_buf;

 /* receive the version and username prompt from server */

 addr_len = sizeof(struct sockaddr);
 if((recv_byte = recvfrom(server_sock,client_buf,BUF_LEN,0,(struct sockaddr *)&server_addr,&addr_len)) == -1)
 {
	perror("recvfrom ");
	exit(1);
 }

 if(client_buf[0] != USER_PROMPT)
 {
	printf("unknown code(%d) from server\nexiting ...\n",client_buf[0]);
	exit(1);
 }

 else if(client_buf[0] == USER_PROMPT)
 {
	for(loop = 1;loop < recv_byte;loop++)
	{
		if((loop - 1) < APPNAME_LEN)
		{
			if(client_buf[loop] == ' ')
			{
				app_name[loop - 1] = '\0';
				break;
			}

			else
				app_name[loop - 1] = client_buf[loop];
		}
	}

	if(loop == BUF_LEN)
	{
		printf("buf_overflow\nexiting ...\n");
		exit(1);
	}

	in_loop = 0;
	for(loop += 1;loop < recv_byte;loop++,in_loop++)
	{
		if(in_loop < VER_LEN)
		{
			if(client_buf[loop] == '\0')
			{
				version[in_loop] = '\0';
				break;
			}

			else
				version[in_loop] = client_buf[loop];
		}
	}

	if(in_loop < VER_LEN && loop < BUF_LEN)
	{
		if(loop == recv_byte)
			version[in_loop] = client_buf[loop];
	}

	printf("%s %s\n",app_name,version);
 }

 if(sign_up == 0)
 {
	printf("\nenter the username : ");
	for(loop = 0;loop < USERNAME_LEN;loop++)
	{
		if((username[loop] = getc(stdin)) == EOF)
			break;

		if(username[loop] == '\n')
			break;
	}

	if(loop == USERNAME_LEN)
		username[loop - 1] = '\0';

	else
		username[loop] = '\0';

	/* send username to server */

	send_byte = CODE_LEN;
	client_buf[0] = USER_LOGIN;

	for(loop = 1;loop < BUF_LEN;loop++)
	{
		if((loop - 1) < USERNAME_LEN)
		{
			if(username[loop - 1] == '\0')
				break;
			else
			{
				client_buf[loop] = username[loop - 1];
				send_byte++;
			}
		}
	}

	if(sendto(server_sock,client_buf,send_byte,0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
	{
		perror("sendto ");
		exit(1);
	}

	/* reply from server about user query */

	if(recvfrom(server_sock,client_buf,CODE_LEN,0,(struct sockaddr *)&server_addr,&addr_len) == -1)
	{
		perror("recvfrom ");
		exit(1);
	}

	/* user doesn't exist */

	if(client_buf[0] == USER_FAIL)
	{
		printf("%s doesnot exist\nlogin failed\n",username);
		exit(1);
	}

	/* user exists */

	else if(client_buf[0] != USER_OK)
	{
		printf("unknown code from server\nexiting ...\n");
		exit(1);
	}

	printf("user ok ...\n");

	/* enter the user password */

	printf("enter the password : ");
	for(loop = 0;loop < PASS_LEN;loop++)
	{
		if((client_pass[loop] = getc(stdin)) == EOF)
			break;

		if(client_pass[loop] == '\n')
			break;
	}

	if(loop == PASS_LEN)
		client_pass[loop - 1] = '\0';

	else
		client_pass[loop] = '\0';

	/* create the sha-1 hash of the password */

	sha_1(client_pass,client_hash);

	/* recieve the random sequence number encrypted with password from server */

	if((recv_byte = recvfrom(server_sock,client_buf,BUF_LEN,0,(struct sockaddr *)&server_addr,&addr_len)) == -1)
	{
		perror("recvfrom ");
		exit(1);
	}

	/* decrypt the seq_no using the password hash */

	decrypt_init(client_hash,client_seq_no,encrypt_seq);

	/* third step of the handshake */

	rotate_seq_init(client_seq_no,rot_seq_no);

	encrypt_init(client_hash,rot_seq_no,encrypt_seq);
	if(sendto(server_sock,client_buf,BUF_LEN,0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
	{
		perror("sendto ");
		exit(1);
	}

	/* fourth step of the handshake */

	if(recvfrom(server_sock,client_buf,BUF_LEN,0,(struct sockaddr *)&server_addr,&addr_len) == -1)
	{
		perror("recvfrom ");
		exit(1);
	}

	if(client_buf[0] == FAIL_LOGIN)
	{
		printf("authentication failed\n");
		exit(1);
	}

	else if(client_buf[0] == SUCCESS_LOGIN)
	{
		printf("login successful\n");
		exit(1);
	}

	else
		printf("unknown code(%d)\n",client_buf[0]);
 }

 else if(sign_up == 1)
 {
	printf("creating user ...\n");
	send_byte = CODE_LEN;
	client_buf[0] = USER_CREATE;

	if(sendto(server_sock,client_buf,send_byte,0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
	{
		perror("sendto ");
		exit(1);
	}
 }

}
