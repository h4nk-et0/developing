#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ADDR_QUEUE	100	/* maximum number of connections at a time */
#define BUF_LEN		20	/* maximum buffer length for send() */
#define CHAT_CONF_PATH	"/etc/project_hank/hank-chat/chat_sequence.conf"
#define CODE_LEN	1
#define HASH_LEN	SEQ_LEN
#define IP_LEN		16	/* ip address length */
#define LISTEN_CON	10	/* maximum number of connections in the queue for the listen() */
#define PASS_LEN	52
#define PORT_NUM	4444	/* port number for communication */
#define SEQ_LEN		5
#define SERVER_NAME	51
#define USER_DB		"/etc/project_hank/hank-chat/user_db"
#define	USERNAME_LEN	PASS_LEN

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

const char app_name[] = "hank-chat";
const unsigned char version[] = "0.0.2";

void kill_zombies(int sig_num)
{
	unsigned int errno_backup;

	errno_backup = errno;
	wait(0);
	errno = errno_backup;
}

void main()
{

 struct sockaddr_in	server_addr,
			client_addr;
 char	server_name[SERVER_NAME],
	ip_addr[IP_LEN],
	username[USERNAME_LEN];
 unsigned char	server_buf[BUF_LEN];
 int	server_sock,
	client_sock,
	client_addr_size,
	pipe_auth[2];
 pid_t	pid_main,
	pid_auth,
	pid_manage;
 unsigned short int	forked = 0,
			loop,
			in_loop,
			send_byte,
			recv_byte;
 unsigned int	pass_hash[HASH_LEN],
		server_seq_no[SEQ_LEN],
		*encrypt_seq,
		rot_seq_no[SEQ_LEN],
		rand_seq;
 socklen_t	addr_len;
 FILE	*server_seq,
	*user_db;

 /* check for root */

 if(getuid())
 {
	printf("not enough privileges. run it as root\n");
	exit(1);
 }

 /* open the sequence file to store the randomized sequence number */

 if((server_seq = fopen(CHAT_CONF_PATH,"r+")) == NULL)
 {
	if(errno == 2)
	{
		if((server_seq = fopen(CHAT_CONF_PATH,"w+")) == NULL)
		{
			perror("chat_sequence.conf ");
			exit(1);
		}

		printf("\t\t**** one time setup ****\nenter a random number : ");
		scanf("%d",&rand_seq);
		getc(stdin);

		for(loop = 0;loop < SEQ_LEN;loop++)
		{
			server_seq_no[loop] = rand() + rand_seq;
			fprintf(server_seq,"%08x\t",server_seq_no[loop]);
		}
	}
 }

 /* close the sequence file. It will be openend in the main loop later */

 fclose(server_seq);

 /* open the database file for storing userid and password */

 if((user_db = fopen(USER_DB,"r+")) == NULL)
 {
	if(errno == 2)
	{
		if((user_db = fopen(USER_DB,"w+")) == NULL)
		{
			perror("cannot open user_db ");
			exit(1);
		}
	}
 }

 /* close the database file. It will be opened in the authentication process */

 fclose(user_db);

 /* setup a name for the server */

 printf("enter the server name (max 50 characters) : ");
 for(loop = 0;loop < SERVER_NAME;loop++)
 {
	if((server_name[loop] = getc(stdin)) == EOF)
		break;

	if(server_name[loop] == '\n')
		break;
 }

 if(loop == SERVER_NAME)
	server_name[loop - 1] = '\0';

 else
	server_name[loop] = '\0';

 /* configure the IP address for the server. The port 4444 will be used for communication */

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

 /* create a socket for TCP */

 if((server_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
 {
	perror("server_sock ");
	exit(1);
 }

 /* bind the socket to the IP address */

 if(bind(server_sock,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
 {
	perror("bind ");
	exit(1);
 }

 /* listen for connections from clients. The max number of connections in queue can be LISTEN_CON */

 if(listen(server_sock,LISTEN_CON) == -1)
 {
	perror("listen ");
	exit(1);
 }

 /* create a pipe for communication with the child process */

 if(pipe(pipe_auth) == -1)
 {
	perror("pipe ");
	exit(1);
 }

 /* install the signal SIGCHLD so that when a child exits it doesn't become a zombie */

 signal(SIGCHLD,kill_zombies);

 /* main loop for listening new connections */

 while(1)
 {
	/* accept the new connection at the socket server_sock */

	client_addr_size = sizeof(struct sockaddr_in);
	if((client_sock = accept(server_sock,(struct sockaddr *)&client_addr,&client_addr_size)) == -1)
	{
		perror("accept ");
		exit(1);
	}

	/* reopen the file */

	if((server_seq = fopen(CHAT_CONF_PATH,"r+")) == NULL)
	{
		perror("cannot open chat_sequence.conf ");
		exit(1);
	}

	/* initial random sequence number, the second step of the handshake (first step being the tcp handshake) */

	for(loop = 0;loop < SEQ_LEN;loop++)
		fscanf(server_seq,"%x",&server_seq_no[loop]);

	fclose(server_seq);

	if((server_seq = fopen(CHAT_CONF_PATH,"w+")) == NULL)
	{
		perror("cannot open chat_sequence.conf ");
		exit(1);
	}

	for(loop = 0;loop < SEQ_LEN;loop++)
	{
		server_seq_no[loop] += rand();
		fprintf(server_seq,"%08x\t",server_seq_no[loop]);
	}

	fclose(server_seq);

	/* create a new process for managing the chat session, if it isn't already created */

	if(!forked)
	{
		forked = 1;
		pid_manage = fork();
	}

	if(!pid_manage)
	{
		static	struct	sockaddr_in	client_addr_queue[ADDR_QUEUE],
						client_addr;
		int	client_sock_queue[ADDR_QUEUE],
			client_sock,
			activity;
		unsigned short int client_queue,
			max_fdval = 0;
		fd_set	client_array;

		/* uninstall the signal handler and close the server socket, write-end of the pipe */

		signal(SIGCHLD,SIG_IGN);
		close(server_sock);
		close(pipe_auth[1]);

		/* initialize the client socket queue to zero */

		for(loop = 0;loop < ADDR_QUEUE;loop++)
			client_sock_queue[loop] = 0;

		FD_ZERO(&client_array);
		FD_SET(pipe_auth[0],&client_array);

		if(pipe_auth[0] > max_fdval)
			max_fdval = pipe_auth[1];

		client_queue = 0;
		while(1)
		{
			activity = select(max_fdval + 1, &client_array, NULL, NULL, NULL);

			if(FD_ISSET(pipe_auth[0], &client_array))
			{
				if(read(pipe_auth[0],&client_sock,sizeof(int)) > 0)
					if(read(pipe_auth[0],&client_addr,sizeof(struct sockaddr)) > 0)
					{
						printf("login success from %s\n",inet_ntoa(client_addr.sin_addr));

						for(client_queue = 0;client_queue < ADDR_QUEUE;client_queue++)
						{
							if(client_sock_queue[client_queue] == 0)
							{
								client_sock_queue[client_queue] = client_sock;
								client_addr_queue[client_queue] = client_addr;

								FD_SET(client_sock_queue[client_queue], &client_array);
								if(client_sock_queue[client_queue] > max_fdval)
									max_fdval = client_sock_queue[client_queue];
								break;
							}
						}
						if(client_queue == ADDR_QUEUE)
							printf("client queue is full ...\n");
					}
			}

			else
				for(client_queue = 0;client_queue < ADDR_QUEUE;client_queue++)
				{
					if(client_sock_queue[client_queue])
						if(FD_ISSET(client_sock_queue[client_queue], &client_array))
					{
						addr_len = sizeof(struct sockaddr);
						if((recv_byte = recvfrom(client_sock_queue[client_queue],server_buf,BUF_LEN,0,
									(struct sockaddr *)&client_addr_queue[client_queue],&addr_len)) == 0)
						{
							printf("closing\n");
							close(client_sock_queue[client_queue]);

							client_sock_queue[client_queue] = 0;
							FD_CLR(client_sock_queue[client_queue], &client_array);
						}
					}
				}
		}
		exit(1);
	}

	pid_auth = fork();

	/* handover the newly accepted connection to the authentication process */

	if(!pid_auth)
	{
		char	file_data,file_username[USERNAME_LEN];
		unsigned short int user_found, str_equal;
		unsigned int	client_seq_no[SEQ_LEN];

		/* uninstall the signal handler and close the server socket, read-end of the pipe */

		signal(SIGCHLD,SIG_IGN);
		close(server_sock);
		close(pipe_auth[0]);

		if((user_db = fopen(USER_DB,"r+")) == NULL)
		{
			perror("cannot open user_db ");
			exit(1);
		}

		encrypt_seq = (unsigned int *)server_buf;

		/* send the prompt for username and the version */

		send_byte = CODE_LEN;
		server_buf[0] = USER_PROMPT;

		for(loop = 1;loop < BUF_LEN;loop++)
		{
			if(app_name[loop - 1] == '\0')
				break;

			server_buf[loop] = app_name[loop - 1];
			send_byte++;
		}

		if(loop < BUF_LEN)
		{
			server_buf[loop] = ' ';
			loop++;
			send_byte++;
		}

		if(loop < BUF_LEN)
		{
			for(in_loop = loop;in_loop < BUF_LEN;in_loop++)
			{
				if(version[in_loop - loop] == '\0')
					break;

				server_buf[in_loop] = version[in_loop - loop];
				send_byte++;
			}
		}

		if(sendto(client_sock,server_buf,send_byte,0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr)) == -1)
		{
			perror("auth-sendto ");
			exit(1);
		}

		addr_len = sizeof(struct sockaddr);
		if((recv_byte = recvfrom(client_sock,server_buf,BUF_LEN,0,(struct sockaddr *)&client_addr,&addr_len)) == -1)
		{
			perror("auth-sendto ");
			exit(1);
		}

		if(recv_byte < BUF_LEN)
			server_buf[recv_byte] = '\0';

		if(server_buf[0] != USER_LOGIN && server_buf[0] != USER_CREATE)
		{
			printf("unknown code(%d) from the client %s\nclosing connection ...\n",server_buf[0],inet_ntoa(client_addr.sin_addr));
			exit(1);
		}

		else if(server_buf[0] == USER_LOGIN)
		{
			for(loop = 1;loop < BUF_LEN;loop++)
			{
				if(loop < USERNAME_LEN)
				{
					if(server_buf[loop] == '\0')
					{
						username[loop - 1] = '\0';
						break;
					}

					username[loop - 1] = server_buf[loop];
				}
			}

			/* check if the userid exists (read the user_db file) */

			user_found = 0;

			while(1)
			{
				if((file_data = getc(user_db)) == EOF)
					break;

				if(file_data == '#')
				{
					while(1)
					{
						if(getc(user_db) == '\n');
						{
							file_data = getc(user_db);
							break;
						}
					}
				}

				else if(file_data == '\n')
					continue;

				loop = 0;
				while(1)
				{
					file_username[loop] = file_data;

					if((file_data = getc(user_db)) == EOF)
						break;

					loop++;
					if(loop == USERNAME_LEN || loop >= USERNAME_LEN)
					{
						printf("buf_overflow ...\nclosing connection to %s ...\n",inet_ntoa(client_addr.sin_addr));
						exit(1);
					}

					else if(file_data == '\t' || file_data == ' ')
						break;

					else if(file_data == '\n')
					{
						printf("invalid table found in the user_db file\nplease check the file ");
						printf(USER_DB);
						printf(" ...\nexiting ...\n");
						exit(1);
					}
				}

				if(loop == USERNAME_LEN - 1)
				{
					printf("username exceeded the limit ...\nclosing connection to %s ...\n",inet_ntoa(client_addr.sin_addr));
					exit(1);
				}

				else
					file_username[loop] = '\0';

				str_equal = 0;
				for(in_loop = 0;in_loop < USERNAME_LEN;in_loop++)
				{
					if(file_username[in_loop] == username[in_loop])
					{
						if(file_username[in_loop] == '\0' && username[in_loop] == '\0')
						{
							str_equal = 1;
							break;
						}
					}

					else
						break;
				}

				if(str_equal)
				{
					user_found = 1;
					for(loop = 0;loop < SEQ_LEN;loop++)
						fscanf(user_db,"%x",&pass_hash[loop]);

					break;
				}

				else
				{
					while(1)
					{
						if((file_data = getc(user_db)) == EOF)
							break;

						if(file_data == '\n')
							break;
					}
				}

				if(feof(user_db))
					break;
			}

			fclose(user_db);

			/* send USER_OK if the username exists */

			if(user_found == 1)
			{
				server_buf[0] = USER_OK;
				if(sendto(client_sock,server_buf,CODE_LEN,0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr)) == -1)
				{
					perror("auth-sendto ");
					exit(1);
				}
			}

			/* send USER_FAIL if the username doesnot exist and terminate the connection */

			if(user_found == 0)
			{
				server_buf[0] = USER_FAIL;
				if(sendto(client_sock,server_buf,CODE_LEN,0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr)) == -1)
				{
					perror("auth-sendto ");
					exit(1);
				}

				printf("failed login attempt from %s\n",inet_ntoa(client_addr.sin_addr));
				exit(1);
			}

			/* encrypt the password for that username with a random sequence number */

			encrypt_init(pass_hash,server_seq_no,encrypt_seq);

			if(sendto(client_sock,server_buf,BUF_LEN,0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr)) == -1)
			{
				perror("auth-sendto ");
				exit(1);
			}

			/* calculate the desired response from the client */

			rotate_seq_init(server_seq_no,rot_seq_no);

			/* third step of the handshake */

			if(recvfrom(client_sock,server_buf,BUF_LEN,0,(struct sockaddr *)&client_addr,&addr_len) == -1)
			{
				perror("auth-recvfrom ");
				exit(1);
			}

			decrypt_init(pass_hash,client_seq_no,encrypt_seq);

			/* fourth step of the handshake */

			for(loop = 0;loop < SEQ_LEN;loop++)
			{
				/* check for the authentication */

				if(rot_seq_no[loop] != client_seq_no[loop])
				{
					printf("failed login attempt from %s\n",inet_ntoa(client_addr.sin_addr));

					server_buf[0] = FAIL_LOGIN;
					if(sendto(client_sock,server_buf,CODE_LEN,0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr)) == -1)
					{
						perror("auth-sendto ");
						close(client_sock);
						exit(1);
					}

					close(client_sock);
					exit(1);
				}
			}

			/* login successful */

			server_buf[0] = SUCCESS_LOGIN;
			if(sendto(client_sock,server_buf,CODE_LEN,0,(struct sockaddr *)&client_addr,sizeof(struct sockaddr)) == -1)
			{
				perror("auth-sendto ");
				exit(1);
			}

			/* send client socket and the client address to the manage process */

			write(pipe_auth[1],&client_sock,sizeof(int));
			write(pipe_auth[1],&client_addr,sizeof(struct sockaddr));
			exit(1);

		}

		else if(server_buf[0] == USER_CREATE)
		{
			printf("creating user\n");
			exit(1);
		}
	}
 }
}
