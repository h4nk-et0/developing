/*

	title		-> hank-search
	description	-> a simple searching algorithm for words in a large file
	author		-> hank

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void main(int argc,char *argv[])
{
 int line_count=1,column_count=1,s_string_length,f_string_length,f_loop,s_loop,match=0;
 char s_string[50],f_string[200],special_char;
 FILE *input_file;

 printf("[+] opening the file ......");
 if(argc<2)
 {
	printf("\r[\x1b[31m-\x1b[0m]\t\t\t\t\t\t\t\t\t      [\x1b[31mfail\x1b[0m]\n");
	printf("usage : hank-search [file name].....\n");
	exit(1);
 }
 else if((input_file=fopen(argv[1],"r"))==NULL)
 {
	printf("\r[\x1b[31m-\x1b[0m]\t\t\t\t\t\t\t\t\t      [\x1b[31mfail\x1b[0m]\n");
        perror("cannot open file");
	exit(1);
 }
 printf("\t\t\t\t\t\t        [Ok]\n");

 printf("Enter the string to search : ");
 scanf("%[^\n]",s_string);
 s_string_length=strlen(s_string);
 printf("[+] searching .....\n\n");

 while(1)
 {
//	fscanf(input_file,"%[^\n]%c",f_string,&special_char);
	fgets(f_string,200,input_file);
	if(feof(input_file))
		break;
	f_string_length=strlen(f_string);
	f_string[--f_string_length]='\0';
	printf("%s\r",f_string);

	for(f_loop=0;f_loop<f_string_length;f_loop++)
	{
		if(s_string[0]==f_string[f_loop])
		{
			column_count=f_loop+1;
			for(s_loop=0;s_loop<s_string_length;s_loop++)
			{
				if(s_string[s_loop]!=f_string[f_loop++])
					break;
			}
			if(s_loop==s_string_length)
			{
				match++,f_loop--;
				printf("%d th match at line %d column %d: %s",match,line_count,column_count,f_string);
				int i;
				for(i=0;i<f_string_length-column_count+1;i++)
					printf("\b");
				printf("\x1b[31m%s\x1b[0m\n",s_string);
			}
		}
	}
	line_count++,column_count=1;
 }

 printf("\r                                                                                                                                           \r");
 printf("\n[+] search complete ......\n");
 if(match==0)
	printf("[\x1b[31m-\x1b[0m] \"%s\" not found ......\n",s_string);

 printf("[+] closing the file ......");
 if(!fclose(input_file))
	printf("\t\t\t\t\t\t        [Ok]\n");
 else
	printf("\t\t\t\t\t\t      [\x1b[31mfail\x1b[0m]\n");
}
