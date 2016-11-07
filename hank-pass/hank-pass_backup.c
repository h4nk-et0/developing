#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
void main(int argc,char *argv[])
{
 int out_loop,in_loop,start,end,digit_count,count=0,f_var;
 char passwd[20],f_name[20]="pass0001.txt",f_count[5]="0001";
 FILE *fp;

 system("rm pass*.txt");
 if((fp=fopen(f_name,"w"))==NULL)
 {
	perror("open");
	exit (1);
 }
 
 printf("Enter the number of digits : ");
 scanf("%d",&digit_count);
// printf("Enter the first and last ascii value : ");
// scanf("%d%d",&start,&end);
 printf("\n[+] generating pass0001.txt .........");
 fflush(stdout);

 for(out_loop=0;out_loop<digit_count;out_loop++)
	passwd[out_loop]=start;
 while(1)
 {
	for(out_loop=start;out_loop<=end;out_loop++)
	{
		passwd[digit_count-1]=out_loop;
		for(in_loop=0;in_loop<digit_count;in_loop++)
		{
			putc(passwd[in_loop],fp);
		}
		putc('\n',fp);
	}
	passwd[digit_count-1]=end+1;
	for(out_loop=digit_count-1;out_loop>0;out_loop--)
	{
		if(passwd[out_loop]==end+1)
		{
			passwd[out_loop-1]=passwd[out_loop-1]+1;passwd[out_loop]=start;
		}
	}

	if(passwd[0]==end+1 && out_loop==0)
		break;
	count++;

	if((count % 1000000)==0)
	{
		count=0;
		if(!fclose(fp))
			printf("\t\t\t\t        [Ok]\n");
		else
		{
			printf("\r[\x1b[31m-\x1b[0m]\t\t\t\t\t\t\t\t      [\x1b[31mfail\x1b[0m]\n");
			perror("error ");
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
			exit(1);
		}
		printf("[+] generating %s ......... ",f_name);
		fflush(stdout);
	}
 }

 fclose(fp);
 printf("\t\t\t      [ok]\n");
 printf("[+] finished\n[+] all files are saved in following directory\n");
 system("pwd");
 printf("\n");
}
