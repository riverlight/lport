#include <stdlib.h>
#include <stdio.h>

#include "port.h"

int main(int argc, char *argv[])
{
	int time = 0;
	unsigned int runTime = 0, allTime = 0;

	int i=0, j=1000000000;

	while (i++<j);
	
	GetCPUTime(&runTime, &allTime);
	GetProcessRunTime(&time);

	printf("%d	%d	%d\r\n", runTime, allTime, time);
#ifdef WIN32
	system("pause");
#endif	
	return 0;
}

