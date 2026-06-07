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
	printf("[1] List Case History\n");
	printf("[2] Get Case Details\n");
	printf("[3] Access Protected Cases\n");
	printf("[4] Quit Program\n");
}
