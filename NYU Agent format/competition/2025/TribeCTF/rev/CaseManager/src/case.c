#include "case.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "common.h"

void generateCases(Case *caseList)
{
	char buffer[BUFFER_SIZE];
	time_t randtime;
	for(int i = 0; i < CASE_COUNT; i++)
	{
		if(i != 2301)
		{
			 randtime = 1740000000 - (random() % 400000000 + 100000);
		}
		else
		{
			randtime = 1570283465;
		}
		strcpy(buffer, asctime(gmtime(&randtime)) + 4);
		strtok(buffer, "\n");
		strcpy(caseList[i].end_date, buffer);

		randtime -= 3000000 + (random() % 3000000);
		strcpy(buffer, asctime(gmtime(&randtime)) + 4);
		strtok(buffer, "\n");
		strcpy(caseList[i].start_date, buffer);

		int tmp = random() % 3;
		strcpy(caseList[i].handler_name, (tmp == 0)?"Thomas Jefferson":((tmp == 1)?"James Madison":"William Small"));

		caseList[i].id = i+1;
		caseList[i].satisfaction = (random() % 101) / 10.0;
	}

}

void fillCaseText(Case *caseList, char **caseText)
{
	for(int i = 0; i < CASE_COUNT; i++)
	{
		sprintf(caseText[i], "%03d  %-14s%-24s%-24s%2.1f/10%c", caseList[i].id, caseList[i].handler_name, caseList[i].start_date, caseList[i].end_date, caseList[i].satisfaction, '\0');
	}
}
