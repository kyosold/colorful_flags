#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv)
{

	struct tm when;
	time_t now_time;
	time(&now_time);
	when = *localtime(&now_time);
	
	printf("%04d-%02d-%02d %02d:%02d:%02d\n", when.tm_year + 1900, when.tm_mon + 1, when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec);

	return 0;
}

