#ifndef CASE_H
#define CASE_H

#include "common.h"
#define CASE_COUNT 5000

typedef struct _case
{
    int id;
    char handler_name[50];
    char start_date[25];
    char end_date[25];
    float satisfaction;

}Case;

void generateCases(Case *caseList);
void fillCaseText(Case *caseList, char **caseText);

#endif
