/*

	title		-> hank-pass
	description	-> used to generate the password list of every combination specified (used in brute force)
	author		-> hank

*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<signal.h>

int	digit_count,
	start,end,
	skip_start = 0,
	skip_end = 0;
char	passwd[20];
FILE	*conf;

void intrpt(int sig)
{
	int loop;

	signal(sig,SIG_IGN);

	printf("\n\ngot Ctrl-C from user ...\nwrapping up the configuration ...\n");
	printf("writing to the configuration file ...\n\n");
	passwd[digit_count - 1]--;
	rewind(conf);
	fprintf(conf,"digit_count = %d\n",digit_count);
	fprintf(conf,"start = %d\nend = %d\nskip_start = %d\nskip_end = %d\npasswd = ",start,end,skip_start,skip_end);
	printf("digit_count = %d\n",digit_count);
	printf("start = %d\nend = %d\nskip_start = %d\nskip_end = %d\npasswd = ",start,end,skip_start,skip_end);
	for(loop = 0;loop < digit_count;loop++)
	{
		printf("%c",passwd[loop]);
		fprintf(conf,"%c",passwd[loop]);
	}
	printf("\n");
	fprintf(conf,"\n");

	fclose(conf);
	exit(0);
}
void main(int argc,char *argv[])
{
 int	out_loop,
	in_loop,
	count=0,
	f_var,
	char_type_len,
	conf_file = 1;
 char	load_conf,
	f_name[20]="pass0001.txt";
 char	f_count[5]="0001",
	char_type[10],
	conf_str[20];
 FILE	*fp;

 system("rm pass*.txt");
 if((conf = fopen("/etc/hank_pass.conf","r+")) == NULL)
 {
	if(errno == 2)
	{
		if((conf = fopen("/etc/hank_pass.conf","w")) == NULL)
		{
			perror("conf ");
			exit(0);
		}
		conf_file = 0;
	}
	else
	{
		perror("conf ");
		exit(0);
	}
 }
 system("clear");

 printf("\n\n\x1b[32m\t -----------------------------------------------\x1b[0m\n");
 printf("\t\x1b[32m|\x1b[1m     \x1b[32mIf thEy d0nt want pe0plE insidE ...\x1b[0m\t\x1b[32m|\n");
 printf("\t\x1b[32m|\x1b[1m     \t\x1b[31mThEy 0uGht t0 build it bEttEr ...\x1b[0m\t\x1b[32m|\n");
 printf("\x1b[32m\t -----------------------------------------------\x1b[0m\n\n\n");

 if(conf_file == 1)
 {
	while(1)
	{
		printf("load from where you left off ? [yY|nN] : ");
		scanf("%c",&load_conf);
		if(load_conf == 'y' || load_conf == 'Y' || load_conf == 'n' || load_conf == 'N')
			break;
		printf("invalid choice ...\n");
	}
 }

 else
	load_conf = 'n';

 if(load_conf == 'y' || load_conf == 'Y')
 {
	printf("loading from the configuration file ...\n\n");
	for(out_loop = 0;out_loop < 6;out_loop++)
	{
		fscanf(conf,"%[^=]",conf_str);
		fseek(conf,1,SEEK_CUR);
		if(!strcmp("digit_count ",conf_str))
		{
			fscanf(conf,"%d",&digit_count);
			printf("%s= %d\n",conf_str,digit_count);
		}
		else if(!strcmp("start ",conf_str))
		{
			fscanf(conf,"%d",&start);
			printf("%s= %d\n",conf_str,start);
		}
		else if(!strcmp("end ",conf_str))
		{
			fscanf(conf,"%d",&end);
			printf("%s= %d\n",conf_str,end);
		}
		else if(!strcmp("skip_start ",conf_str))
		{
			fscanf(conf,"%d",&skip_start);
			printf("%s= %d\n",conf_str,skip_start);
		}
		else if(!strcmp("skip_end ",conf_str))
		{
			fscanf(conf,"%d",&skip_end);
			printf("%s= %d\n",conf_str,skip_end);
		}
		else if(!strcmp("passwd ",conf_str))
		{
			printf("%s= ",conf_str);
			fscanf(conf,"%s",conf_str);
			for(in_loop = 0;in_loop < digit_count;in_loop++)
			{
				passwd[in_loop] = conf_str[in_loop];
				printf("%c",passwd[in_loop]);
			}
			printf("\n");
		}
		fscanf(conf,"%[\n]",&char_type[0]);	//remaining '\n'
	}
 }

 else if(load_conf == 'n' || load_conf == 'N')
 {
	while(1)
	{
		printf("Enter the number of digits : ");
		scanf("%d",&digit_count);
		if(20 >= digit_count > 0)
			break;
		else if(digit_count > 20)
			printf("too large\n");
		else
			printf("invalid digits\n");
	}
	while(1)
	{
		scanf("%c",&passwd[0]);//for the leftout \n from the last scanf
		printf("\n\t\t\tenter the type of characters :");
		printf("\n(digits -> d, a-z -> a, A-Z -> A, d and a-z -> da, d and A-Z -> dA, a-z and A-Z -> aA, d and a-z and A-Z -> daA)");
		printf("\npress enter for default(all) : ");
		fgets(char_type,10,stdin);
		char_type_len=strlen(char_type);
		if(char_type[0]=='\n')
		{
			start=32;
			end=126;
			break;
		}
		else if(char_type[1]=='\n')
		{
			if(char_type[0]=='d')
			{
				start=48;
				end=57;
				break;
			}
			else if(char_type[0]=='a')
			{
				start=97;
				end=122;
				break;
			}
			else if(char_type[0]=='A')
			{
				start=65;
				end=90;
				break;
			}
		}
		else if(char_type[2]=='\n')
		{
			if(char_type[0]=='a' && char_type[1]=='A')
			{
				start=65;
				end=122;
				skip_start=91;
				skip_end=96;
				break;
			}
			else if(char_type[0]=='d' && char_type[1]=='a')
			{
				start=48;
				end=122;
				skip_start=58;
				skip_end=96;
				break;
			}
			else if(char_type[0]=='d' && char_type[1]=='A')
			{
				start=48;
				end=90;
				skip_start=58;
				skip_end=64;
				break;
			}
		}
		else if(char_type[3]=='\n')
		{
			if(char_type[0]=='d' && char_type[1]=='a' && char_type[2]=='A')
			{
				start=48;
				end=122;
				break;
			}
		}

		printf("invalid choice\n");
	}
 }

 if((fp=fopen(f_name,"w"))==NULL)
 {
	perror("open");
	exit (1);
 }


 printf("\n[+] generating pass0001.txt .........");
 fflush(stdout);
 signal(SIGINT,intrpt);

 if(load_conf == 'n' || load_conf == 'N')
 {
	for(out_loop=0;out_loop<digit_count;out_loop++)
		passwd[out_loop]=start;
 }

 while(1)
 {
	for(out_loop=start;out_loop<=end;out_loop++)
	{
		if(out_loop==skip_start && skip_start)
			out_loop=skip_end+1;
		passwd[digit_count-1]=out_loop;
		for(in_loop=0;in_loop<digit_count;in_loop++)
			putc(passwd[in_loop],fp);
		putc('\n',fp);
		count++;
	}
	passwd[digit_count-1]=end+1;
	for(out_loop=digit_count-1;out_loop>0;out_loop--)
	{
		if(passwd[out_loop]==end+1)
		{
			passwd[out_loop-1]=passwd[out_loop-1]+1;
			passwd[out_loop]=start;
			if(passwd[out_loop-1]==skip_start && skip_start)
				passwd[out_loop-1]=skip_end+1;
		}
	}

	if(passwd[0]==end+1 && out_loop==0)
		break;

	if(((digit_count+1)*count) >= 1073741824)
	{
		count=0;
		if(!fclose(fp))
			printf("\t\t\t\t        [Ok]\n");
		else
		{
			printf("\r[\x1b[31m-\x1b[0m]\t\t\t\t\t\t\t\t      [\x1b[31mfail\x1b[0m]\n");
			perror("error ");
			intrpt(SIGINT);
			exit(1);
		}

		f_count[3]=f_count[3]+1;
		if(f_count[3]==58)
		{
			f_count[2]=f_count[2]+1;
			f_count[3]=48;
			if(f_count[2]==58)
			{
				f_count[1]=f_count[1]+1;
				f_count[2]=48;
				if(f_count[1]==58)
				{
					f_count[0]=f_count[0]+1;
					f_count[1]=48;
				}
			}
		}
		for(f_var=4;f_var<8;f_var++)
			f_name[f_var]=f_count[f_var-4];
		if((fp=fopen(f_name,"w"))==NULL)
		{
			perror("open");
			intrpt(SIGINT);
			exit(1);
		}
		printf("[+] generating %s ......... ",f_name);
		fflush(stdout);
	}
 }

 fclose(fp);
 fclose(conf);
 printf("\t\t\t      [ok]\n");
 printf("[+] finished\n[+] all files are saved in following directory\n");
 system("pwd");
 printf("\n");
}
