#pragma once

/*
Establishes a connection to the desired COM port.
*/
int connect(LPCTSTR lpszCommName, HWND hwnd);

/*
Defines the window process that will handle all windows messages to this application.
*/
LRESULT CALLBACK COMTermWndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam);

/*
Disconnects the application from the port and enters COMMAND mode.
*/
int exitSession(HANDLE hComm, HMENU menu);

/*
Initializes the readFileConfig struct for reading.
*/
void initializeConfig(readFileConfig* rfConfig, HANDLE hComm, HWND hwnd);
