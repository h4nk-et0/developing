/*

	title		-> hank-battery
	description	-> monitors the battery level
	author		-> hank

*/

#include<stdio.h>
#include<stdlib.h>

void main()
{
 char	power_status[11],
	next_line[40];
 unsigned short int	power_percentage,
			loop;
 FILE	*uevent;

 while(1)
 {
	if((uevent=fopen("/sys/class/power_supply/BAT1/uevent","r"))==NULL)
	{
		system("/media/darkden/my_code/projects/project_hank/hank-battery/file_open_error");
		sleep(30);
		continue;
	}

 fseek(uevent,43,SEEK_SET);
 fscanf(uevent,"%s",power_status);

 for(loop=0;loop<9;loop++)
	fscanf(uevent,"%s",next_line);

 fseek(uevent,23,SEEK_CUR);
 fscanf(uevent,"%hu",&power_percentage);
 fclose(uevent);

 if(!strcmp(power_status,"Discharging") && power_percentage < 94 && power_percentage > 15)
 {
	sleep(200);
	continue;
 }
 else if(!strcmp(power_status,"Discharging") && power_percentage < 10)
	system("/media/darkden/my_code/projects/project_hank/hank-battery/critical");

 else if(!strcmp(power_status,"Charging") && power_percentage > 97)
	system("/media/darkden/my_code/projects/project_hank/hank-battery/over_charge");

 else if(!strcmp(power_status,"Discharging") && power_percentage < 15)
	system("/media/darkden/my_code/projects/project_hank/hank-battery/low_battery");

 else if(!strcmp(power_status,"Charging") && power_percentage >= 96)
	system("/media/darkden/my_code/projects/project_hank/hank-battery/charged");

 else if(!strcmp(power_status,"Full") && power_percentage == 100)
	system("/media/darkden/my_code/projects/project_hank/hank-battery/full_battery");

 if(strcmp(power_status,"Discharging") && strcmp(power_status,"Charging") && strcmp(power_status,"Full"))
 {
	system("/media/darkden/my_code/projects/project_hank/hank-battery/error");
	exit(1);
 }

 sleep(200);
 }
}
