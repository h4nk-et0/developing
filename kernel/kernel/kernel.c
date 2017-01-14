#define WHITE_ON_BLACK 0x0f

void print_string(char *str)
{
	unsigned int loop,in_loop;
	char *video_mem = (char *)0xb8000;

	for(loop = 1; str[loop] != '\0'; loop++)
	{
		*video_mem = str[loop];
		video_mem++;
		*video_mem = WHITE_ON_BLACK;
		video_mem++;
	}
}

void clear()
{
	char *video_mem = (char *)0xb8000;
	unsigned int loop;

	for(loop = 0;loop < 7200;loop++)
	{
		*video_mem = 0;
		video_mem++;
	}
}

void main()
{
 char *msg = "this is a test ...";

// clear();
 print_string("test");
 print_string(msg + 2);
}
