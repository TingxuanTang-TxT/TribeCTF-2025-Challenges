#include "menu.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>


inline int inputMenu()
{
	int retVal = 0;
	char buffer[BUFFER_SIZE];
	printMenu();

	while(TRUE)
	{
		printf("Enter Your Choice: ");
		if(fgets(buffer, BUFFER_SIZE, stdin) == NULL)
		{
			exit(1);
		}
		
		sscanf(buffer, "%d", &retVal);

		if(retVal >= 1 && retVal <= 4)
		{
			break;
		}
		else
		{
			printf("Invalid Menu Option.\n");
		}
	}

	return retVal;
}

inline void printMenu()
{
	printf("\nPossible Options:\n");
	printf("[1] List our Representatives\n");
	printf("[2] Get Representative's Details\n");
	printf("[3] Contact a Representative\n");
	printf("[4] Quit Program\n");
}
