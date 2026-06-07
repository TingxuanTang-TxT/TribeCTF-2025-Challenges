#include "file.h"
#include <string.h>

FILE* open_output_file(char *filename)
{
	FILE *outfile = fopen(filename, "w");
	if(outfile == NULL)
	{
		printf("File open failed.\n");
	}

	return outfile;
}

int writeToFile(FILE *file, char **caseText)
{
	for(int i = 0; i < CASE_COUNT; i++)
	{
		if(fprintf(file, "%s\n", caseText[i]) < 0)
		{
			printf("File Output Failed.\n");
			return -1;
		}
	}
	return 0;
}
