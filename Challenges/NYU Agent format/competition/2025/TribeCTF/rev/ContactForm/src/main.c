#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "menu.h"
#include "common.h"
#include "flag.h"


int main()
{
	time_t currTime = time(NULL);
	srand(currTime);
	printf("Welcome to the Thomas Jefferson & Associates Contact Form!\n");

	Representative reps[3];
	char phoneNumBuffer[16] = "(XXX) XXX-XXXX";
	char buffer[BUFFER_SIZE];
	for(int i = 0; i < 3; i++)
	{
		reps[i].id = i + 1;
		reps[i].approvalRating = (rand() % 50 + 50) / 10.0;

		sprintf(buffer, "%03d", rand() % 900 + 100);
		strncpy(phoneNumBuffer + 1, buffer, 3);

		sprintf(buffer, "%03d", rand()%1000);
		strncpy(phoneNumBuffer + 6, buffer, 3);

		sprintf(buffer, "%04d", rand()%10000);
		strncpy(phoneNumBuffer + 10, buffer, 4);
		strcpy(reps[i].phone_number, phoneNumBuffer);
	}

	strcpy(reps[0].name, "Thomas Jefferson");
	strcpy(reps[1].name, "James Madison");
	strcpy(reps[2].name, "William Small");
	strcpy(reps[0].email, "thomas.jefferson@wmlaw.com");
	strcpy(reps[1].email, "james.madison@wmlaw.com");
	strcpy(reps[2].email, "william.small@wmlaw.com");


	int input = 0;
	int inputId;
	while(input != 4)
	{
		input = inputMenu();
		switch(input)
		{
			case 1:
				printf("Listing:\n");
				printf("====================================\n");
				for(int i = 0; i < 3; i++)
				{
					printf("%03d\t%s\n", reps[i].id, reps[i].name);
				}

				break;
			case 2:
				printf("Enter the ID of the Desired Representative: ");
				if(fgets(buffer, BUFFER_SIZE, stdin) == NULL)
				{
					exit(1);
				}

				inputId = atoi(buffer);

				if(inputId <= 0 || inputId >= 4)
				{
					printf("Invalid ID.\n");
					break;
				}

				inputId--;
				printf("Information:\n");
				printf("==========================================================================\n");
				printf("ID: %03d\n", reps[inputId].id);
				printf("Name: %s\n", reps[inputId].name);
				printf("Phone Number: %s\n", reps[inputId].phone_number);
				printf("Email: %s\n", reps[inputId].email);
				printf("Rating: %.1f/10\n", reps[inputId].approvalRating);

				break;
			case 3:
				printf("Enter the ID of the Desired Representative: ");
				char buffer[BUFFER_SIZE];
				if(fgets(buffer, BUFFER_SIZE, stdin) == NULL)
				{
					exit(1);
				}
				int inputId = atoi(buffer);

				if(inputId <= 0 || inputId >= 4)
				{
					printf("Invalid ID.\n");
					break;
				}

				printf("\nThis Representative is Currently Unavailable.\n");
				break;
			case 4:
				printf("Exiting Program...\n");
				break;
			case 5:
				showFlag();
				break;
		}
	}
}

