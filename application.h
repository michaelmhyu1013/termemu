#pragma once
#include "physical.h"

extern bool isConnected;

extern HANDLE hComm;

/*
Represents the main window application. The window will constantly read messages to pass to the specified window process.
*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow);

/*
Toggles the enabled state of the items in the menu.
*/
void changeMenuItemState(HMENU menu);

/*
Displays a MessageBox with help on how to use the application.
*/
void getHelpMessage(HWND hwnd);

/*
Displays the read character from the serial port onto the window terminal.
*/
void printToTerminal(readFileConfig* rfConfig, RECT windowCoordinates, 
	unsigned* x, unsigned* y, TEXTMETRIC textMetrics, char* readBuffer);
