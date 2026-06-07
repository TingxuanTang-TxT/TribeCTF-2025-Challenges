#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "menu.h"
#include "common.h"
#include "case.h"
#include "flag.h"
#include "main.h"
#include "file.h"

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "style_amber.h"

int main()
{
	time_t currTime = time(NULL);
	srandom(currTime);
	printf("Thomas Jefferson Case Manager\n");

	Case *caseList = NULL; 
	caseList = malloc(sizeof(Case) * CASE_COUNT);
	if(caseList == NULL)
	{
		exit(1);
	}
	generateCases(caseList);
	
	char buffer[BUFFER_SIZE];
	buffer[0] = '\0';
	char flagBuffer[BUFFER_SIZE] = {0};
	int inputId = 1;


	//Front End
	int screenWidth = 800;
	int screenHeight = 600;
	InitWindow(screenWidth, screenHeight, "Case Manager");
	SetTargetFPS(60); 

	//RayGUI Variables
	bool CaseListButtonPressed = false;
	bool CaseDetailsButtonPressed = false;
	bool PasswordMenuButtonPressed = false;
	bool LicensingButtonPressed = false;
	bool spinnerWindowActive = false;
	unsigned char activePanel = 0;
	bool secretView = true;
	bool spinnerTextEdit = false;

	bool showPasswordError = false;
	bool showFlagWindow = false;
	bool showFlagError = false;

	//Scroll Panel
	GuiPanelCaseListState state = InitGuiPanelCaseList(caseList);
	free(caseList);
	caseList = NULL;
	if(!state.setValues)
	{
		exit(1);
	}

	//Load Image
	//Image mackImage = LoadImage("res/mew.png");
	// Image mackImage = LoadImage("res/mackFormal.png");
	Image mackImage = LoadImage("res/jeffFormal.png");
	//Image mackImage = LoadImage("res/purpleMack.png");
	Texture2D mackTexture = LoadTextureFromImage(mackImage);
	UnloadImage(mackImage);


	char *licensingText = "You're hereby granted a\nnon-transferable license to\ngetting top-tier legal\nadvice from Law Office of\nThomas Jefferson\nwith all real-world\napplicability dependent\nentirely on cosmic\nalignment.";

	//Gui Styling
	GuiLoadStyleAmber();
	GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

	while (!WindowShouldClose())	// Detect window close button or ESC key
	{
		if(GetScreenWidth() != screenWidth || GetScreenHeight() != screenHeight)
		{
			SetWindowSize(screenWidth, screenHeight);
		}

		GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
		BeginDrawing();

			ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
			//draw mack image
			//DrawTexturePro(mackTexture, (Rectangle){0, 0, 584, 529}, (Rectangle){30, 20, 140, 140}, (Vector2){0,0}, 0.0f, WHITE);//mew
			DrawTexturePro(mackTexture, (Rectangle){0, 0, 509, 490}, (Rectangle){30, 20, 140, 140}, (Vector2){0,0}, 0.0f, WHITE);//formal
//			DrawTexturePro(mackTexture, (Rectangle){0, 0, 285, 361}, (Rectangle){50, 20, 100, 140}, (Vector2){0,0}, 0.0f, WHITE);//purple

			CaseListButtonPressed = GuiButton((Rectangle){ 25, 180, 150, 50 }, "List Case History"); 
			CaseDetailsButtonPressed = GuiButton((Rectangle){ 25, 240, 150, 50 }, "Get Case Details"); 
			PasswordMenuButtonPressed = GuiButton((Rectangle){ 25, 300, 150, 50 }, "Access Protected\nCases"); 
			GuiLine((Rectangle){10, 170, 180, 1}, NULL);
			GuiLine((Rectangle){10, 360, 180, 1}, NULL);

			if(CaseListButtonPressed)
			{
				activePanel = 0x03;
			}
			else if(CaseDetailsButtonPressed)
			{
				activePanel = 0x0C;
			}
			else if(PasswordMenuButtonPressed)
			{
				activePanel = 0x30;
			}
			else if(LicensingButtonPressed)
			{
				activePanel = 0xC0;
			}
			
			GuiSetStyle(DEFAULT, TEXT_SIZE, 23);
			switch(activePanel)
			{
				case 0x03: //Case Listing
					GuiPanel((Rectangle){200, 0, screenWidth - 200, screenHeight}, "Thomas Jefferson Case Manager - Case Listing");


					GuiLine((Rectangle){220, 90, 560, 1}, NULL);

					drawScrollPanel(&state);

					GuiSetIconScale(2);
					bool saveButtonPressed = GuiButton((Rectangle){735, 35, 45, 45}, "#2#");
					GuiSetIconScale(1);

					if(saveButtonPressed)
					{
						FILE *outfile = open_output_file("outputList.txt");
						if(outfile == NULL) break;
						
						writeToFile(outfile, state.caseText);

						printf("File made successfully.\n");
						fclose(outfile);
					}

					break;

				case 0x0C: //Case Details
					GuiPanel((Rectangle){200, 0, screenWidth - 200, screenHeight}, "Thomas Jefferson Case Manager - Case Details");

					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					GuiLabel((Rectangle){230, 25, 500, 40}, "Enter the ID of the Desired Case: ");

					GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
					if (GuiSpinner((Rectangle) {230, 70, 150, 30}, NULL, &inputId, 1, CASE_COUNT, spinnerTextEdit))
					{
						spinnerTextEdit = !spinnerTextEdit;
						spinnerWindowActive = false;
					}
					if(GuiButton((Rectangle){255, 110, 100, 30}, "Submit"))
					{
						spinnerWindowActive = true;
					}
					if(spinnerWindowActive)
					{
						if(inputId != 2302)
						{
							bool closeWindow = GuiWindowBox((Rectangle){400, 200, 300, 90}, "#220#Error Message");
							GuiLabel((Rectangle){410, 230, 280, 50}, "Unable to Fetch Case Details.");
							if(closeWindow) spinnerWindowActive = false;
						}
						else
						{
							bool closeWindow = GuiWindowBox((Rectangle){400, 200, 300, 90}, "#150#Success!");
							//Function to put fake flag into flagBuffer
							if(decryptFakeFlag(flagBuffer) == -1)
							{
								free(state.caseText[0]);
								state.caseText[0] = NULL;
								free(state.caseText);
								state.caseText = NULL;
							}
							char flagMessageBuffer[BUFFER_SIZE] = "Here's the flag:\n";
							strcat(flagMessageBuffer, flagBuffer);
							GuiSetStyle(DEFAULT, TEXT_SIZE, 17);
							GuiLabel((Rectangle){410, 230, 280, 50}, flagMessageBuffer);

							if(closeWindow) spinnerWindowActive = false;
						}
					}
					break;

				case 0x30: //Password Menu
					GuiPanel((Rectangle){200, 0, screenWidth - 200, screenHeight}, "Thomas Jefferson Case Manager - Protected Cases");

					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					Rectangle textInputRect = {230, 50, 300, 125};
					int submittedPassword = GuiTextInputBox(textInputRect, "#151#User Authentication", "Enter the Password: ", "Submit", buffer, 25, &secretView);
					if(submittedPassword == 1)
					{
						showFlag(buffer, flagBuffer);
						buffer[0] = '\0';
						if(flagBuffer[0] == '\0')
						{
							showPasswordError = true;
							showFlagWindow = false;
							showFlagError = false;
						}
						else if(flagBuffer[0] == '\x01')
						{
							showPasswordError = false;
							showFlagWindow = false;
							showFlagError = true;
						}
						else 
						{
							showFlagWindow = true;
							showPasswordError = false;
							showFlagError = false;
						}
					}

					if(showPasswordError)
					{
						bool closeWindow = GuiWindowBox((Rectangle){400, 200, 300, 90}, "#220#Error Message");
						GuiSetStyle(DEFAULT, TEXT_SIZE, 17);
						GuiLabel((Rectangle){410, 230, 280, 50}, "Incorrect Password Input.");
						if(closeWindow) showPasswordError = false;
					}
					else if(showFlagError)
					{
						bool closeWindow = GuiWindowBox((Rectangle){400, 200, 300, 90}, "#220#Error Message");
						GuiSetStyle(DEFAULT, TEXT_SIZE, 17);
						GuiLabel((Rectangle){410, 230, 280, 50}, "Failed to Retrieve Flag.");
						if(closeWindow) showFlagError = false;
					}
					else if(showFlagWindow)
					{
						bool closeWindow = GuiWindowBox((Rectangle){400, 200, 300, 120}, "#157#Success!");
						char flagMessageBuffer[BUFFER_SIZE] = "Here's the flag:\n";
						strcat(flagMessageBuffer, flagBuffer);
						GuiSetStyle(DEFAULT, TEXT_SIZE, 17);
						GuiLabel((Rectangle){410, 230, 280, 50}, flagMessageBuffer);
						if(closeWindow) showFlagWindow = false;
					}
					break;

				case 0xC0: //Licensing Secret
					GuiPanel((Rectangle){200, 0, screenWidth - 200, screenHeight}, "Thomas Jefferson Case Manager - Secret????");

					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					GuiWindowBox((Rectangle){250, 100, 300, 120}, "#150#Success!");
					GuiSetStyle(DEFAULT, TEXT_SIZE, 17);
					GuiLabel((Rectangle){260, 130, 280, 50}, "How'd you find this??\ndribectf{Y0u_f0uNd_7H3_s3Cr37}");

					break;

				default://Default Pane
					GuiPanel((Rectangle){200, 0, screenWidth - 200, screenHeight}, "Thomas Jefferson Case Manager");
					break;
			}

			GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
			Rectangle licRectangle = {5, 420, 190, 100};
			LicensingButtonPressed = GuiLabelButton(licRectangle, NULL);
			GuiLabel(licRectangle, licensingText);
		EndDrawing();

	}
	UnloadTexture(mackTexture);
	free(state.caseText[0]);
	state.caseText[0] = NULL;
	free(state.caseText);
	state.caseText = NULL;
	Font currFont = GuiGetFont();
	RAYGUI_FREE(currFont.recs);
	RAYGUI_FREE(currFont.glyphs);
	currFont.recs = NULL;
	currFont.glyphs = NULL;
	CloseWindow();
	return 0;
}

GuiPanelCaseListState InitGuiPanelCaseList(Case *caseList)
{
    GuiPanelCaseListState state = { 0 };
	state.setValues = false;

    state.panelActive = true;
    state.panelBounds = (Rectangle){ 220, 100, 560, GetScreenHeight() - 120};
    state.panOffset = (Vector2){ 0, 0 };

	state.caseText = malloc(sizeof(char *) * CASE_COUNT);
	if(state.caseText == NULL)
	{
		printf("Malloc Failed\n");
		return state;
	}

	state.caseText[0] = malloc(CASE_COUNT * 80);
	if(state.caseText[0] == NULL)
	{
		printf("Malloc Failed\n");
		free(state.caseText);
		state.caseText = NULL;
		return state;
	}

	for(int i = 1; i < CASE_COUNT; i++)
	{
		state.caseText[i] = state.caseText[0] + 80 * i;
	}

	fillCaseText(caseList, state.caseText);
    UpdateCaseListRecs(&state);

	state.setValues = true;
    return state;
}

void UpdateCaseListRecs(GuiPanelCaseListState *state)
{
	for(int i = 0; i < CASE_COUNT; i++)
	{
		state->caseRecs[i] = (Rectangle){ state->panelBounds.x + 10, state->panelBounds.y + state->containerScrollOffset.y + (20 + 25 * i), state->panelBounds.width - 20, 25 };
	}
}

void drawScrollPanel(GuiPanelCaseListState *state)
{

	GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
	//Title
	char listHeader[BUFFER_SIZE];
	sprintf(listHeader, "%-5s%-14s%-24s%-24s%s%c", "ID:", "Handler:", "Start Date", "End Date", "Client Rating:", '\0');

	if (!GuiIsLocked())
	{
		UpdateCaseListRecs(state);       // Update all controls rectangles (in case screen changed)
	}

	Rectangle scissorRec = {0};
   	GuiScrollPanel(state->panelBounds, listHeader, (Rectangle){ state->panelBounds.x, state->panelBounds.y, state->panelBounds.width - 16, CASE_COUNT*25}, // WARNING: Hardcoded content height!
   	&state->containerScrollOffset, &scissorRec);

	// Limit drawing to scroll panel bounds
	// // WARNING: It requires a batch processing and restart
	BeginScissorMode(scissorRec.x, scissorRec.y, scissorRec.width, scissorRec.height);

		GuiLock();
		//Code Here
		for(int i = 0; i < CASE_COUNT; i++)
		{
			Rectangle tmpRect = state -> caseRecs[i];
			if(tmpRect.y >= state->panelBounds.y && tmpRect.y <= state->panelBounds.y + state->panelBounds.height)
			{
				GuiLabel(state->caseRecs[i], state->caseText[i]);
			}
		}
		GuiUnlock();

	EndScissorMode();
}
