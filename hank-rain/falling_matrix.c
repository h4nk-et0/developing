/*

	title		-> hank-rain
	description	-> matrix fall (rain of characters or numbers in a controlled speed)
	author		-> hank

*/

#include<stdio.h>
#include<stdlib.h>

#define length 170
#define breadth 46
#define digit_start 48
#define digit_end 57
#define char_start 32
#define char_end 127

void main(int argc,char *argv[])
{
 unsigned short int row[length],rain[length],count=0,set[length],speed[length],start_point[length];
 unsigned short int loop,start,end,choice,tail_in,tail[length],erase[length],erase_last_line[length],colour,speed_val,speed_lim;
 char fill_screen,trail,speed_conf;

 if(argc!=2||argv[1][0]!='-')
 {
	printf("usage : hank-rain [options]\n\n-c\t- configure\n-a\t- auto-configure\n");
	exit(0);
 }
 if(argv[1][1]=='a')
 {
	choice=1;
	fill_screen='y';
 }
 else if(argv[1][1]=='c')
 {
	system("clear");
	while(1)
	{
		printf("digit rain(1) or character rain(2) : ");
		scanf("%hu",&choice);
		if(choice==1 || choice==2)
			break;
		printf("invalid choice\n");
	}
	fill_screen='n';
 }
 else
 {
	printf("usage : hank-rain [options]\n\n-c\t- configure\n-a\t- auto-configure\n");
	exit(0);
 }

 if(fill_screen=='n')
 {
	while(1)
 	{
        	printf("\nenter the tail length : ");
	        scanf("%hu",&tail_in);
	        if(tail_in<40)
        	        break;
	        printf("tail length too high\n");
	}
	while(1)
	{
		printf("\ncolour :\n");
		printf("0->m\x1b[36mi\x1b[30mx\x1b[31me\x1b[34md\t\x1b[0m");
		for(loop=1;loop<38;loop++)
		{
			printf("%d->\x1b[%dmhi\x1b[0m\t",loop,loop);
			if(loop==4)
				loop=7;
			if(loop==9)
				loop=29;
		}
		printf("\n\nchoice : ");
		scanf(" %hu",&colour);
		if(colour<38)
			break;
		printf("invalid choice\n");
	}
	while(1)
	{
		printf("\nspeed :\n(s -> slow  m -> medium  h -> high) : ");
		scanf(" %c",&speed_conf);
		if(speed_conf=='s'||speed_conf=='m'||speed_conf=='h')
			break;
		printf("invalid choice\n");
	}
	if(speed_conf=='s')
	{
		speed_val=300;
		speed_lim=25;
	}
	else if(speed_conf=='m')
	{
		speed_val=150;
		speed_lim=15;
	}
	else
	{
		speed_val=10;
		speed_lim=5;
	}
 }

 if(choice==1)
 {
	start=digit_start;
	end=digit_end;
 }
 else
 {
	start=char_start;
	end=char_end;
 }

 for(loop=0;loop<length;loop++)
 {
	row[loop]=1;
	rain[loop]=start;
	set[loop]=0;
	if(fill_screen=='y')
	{
		speed[loop]=rand()%8;
		while(speed[loop]<5)
			speed[loop]=rand()%8;
		tail[loop]=rand()%30;
		while(tail[loop]<20)
			tail[loop]=rand()%30;
		erase[loop]=breadth-tail[loop];
		erase_last_line[loop]=0;
	}
	else
	{
		speed[loop]=rand()%speed_val;
		while(speed[loop]<speed_lim)
			speed[loop]+=speed_lim;
		erase[loop]=breadth-tail_in;
		erase_last_line[loop]=1;
	}
	start_point[loop]=rand()%10;
 }
 set[1]=1;

 system("clear");
 while(1)
 {
	for(loop=0;loop<length;loop++)
	{
		if(set[loop])
		{
			if(fill_screen=='n')
			{
				if(colour==0)
				{
					if(!(loop%3))
						printf("\x1b[31m");
					else if(!(loop%4))
						printf("\x1b[33m");
					else if(!(loop%5))
						printf("\x1b[35m");
					else if(!(loop%7))
						printf("\x1b[30m");
					else if(!(loop%11))
						printf("\x1b[1m");
					else if(!(loop%13))
						printf("\x1b[3m");
				}
				else
					printf("\x1b[%dm",colour);
			}
			else printf("\x1b[1m");

			printf("\033[%d;%dH%c",row[loop],loop+1,rain[loop]++);
			if(!(count%(speed[loop])))
			{
				if(count>20000)
					count=0;
				if(fill_screen=='y')
				{
					trail=rain[loop]-1;
					printf("\x1b[30m");
					if(row[loop]>tail[loop])
						erase_last_line[loop]=1;
				}
				else trail=' ';
				if(erase_last_line[loop]==1)
					printf("\033[%d;%dH%c",erase[loop],loop+1,trail);

				if(++erase[loop]==breadth)
					erase[loop]=1;
				if(++row[loop]==breadth)
					row[loop]=1;
			}
			if(rain[loop]==end)
				rain[loop]=start;
		}

		if(loop+1==1)
			continue;
		else if(row[1]>start_point[loop])
			set[loop]=1;
		printf("\x1b[0m");
	}
	count++;
 }
}
