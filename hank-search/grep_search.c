/*

	title		-> hank-grep_search
	description	-> a grep (a linux utility) like searching program
	author		-> hank

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
void main(int argc,char *argv[])
{
 int s_string_length,f_string_length,f_loop,s_loop;
 char f_string[200];
 FILE *input_file;

 if((input_file=fopen(argv[1],"r"))==NULL|| argc<2)
 {
	printf("\r[\x1b[31m-\x1b[0m]\t\t\t\t\t\t\t\t\t      [\x1b[31mfail\x1b[0m]\n");
	perror("cannot open file");
	printf("usage : hank-search [file name].....\n");
	exit(1);
 }

 s_string_length=strlen(argv[2]);
 while(1)
 {
	fscanf(input_file,"%s",f_string);
	if(feof(input_file))
		break;
	f_string_length=strlen(f_string);
	for(f_loop=0;f_loop<f_string_length;f_loop++)
	{
		if(argv[2][0]==f_string[f_loop])
		{
			for(s_loop=0;s_loop<s_string_length;s_loop++)
			{
				if(argv[2][s_loop]!=f_string[f_loop++])
					break;
			}
			if(s_loop==s_string_length)
				printf("%s\n",f_string);
		}
	}
 }
 if(fclose(input_file))
	printf("\t\t\t\t\t\t      [\x1b[31mfail\x1b[0m]\n");
}
