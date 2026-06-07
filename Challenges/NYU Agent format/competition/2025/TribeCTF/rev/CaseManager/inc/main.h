#ifndef MAIN_H
#define MAIN_H

#include "raylib.h"
#include "raygui.h"
#include "common.h"
#include "case.h"

typedef struct GuiPanelCaseListState {

	bool setValues;
    bool panelActive;
    Rectangle panelBounds;
    Vector2 panOffset;

    Vector2 containerScrollOffset;

    Rectangle caseRecs[CASE_COUNT]; 
	char **caseText;

} GuiPanelCaseListState;

GuiPanelCaseListState InitGuiPanelCaseList(Case *caseList);
void UpdateCaseListRecs(GuiPanelCaseListState *state);
void drawScrollPanel(GuiPanelCaseListState *state);

#endif
