#ifndef FILE_H
#define FILE_H

#include "case.h"
#include <stdio.h>
#include "common.h"

FILE* open_output_file(char *filename);
int writeToFile(FILE *file, char **caseText);

#endif
