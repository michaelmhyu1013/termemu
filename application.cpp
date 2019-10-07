/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: application.cpp - Creates a Windows application that mimics a terminal emulator that transmits all
-- characters typed on the keyboard to the specified serial port that is open, as well as displays all characters received
-- via the serial port to the terminal. The application will provide a menu in which the user can utilize to perform various 
-- functions such as connecting and disconnecting from a port, exiting the application, and accessing help for the application.
-- This is the entry point to the application and where WinMain is defined.
--
-- PROGRAM: COMP3980Assignment1
--
-- FUNCTIONS:
-- int WINAPI WinMain(int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
-- void changeMenuItemState(HMENU menu)
-- void getHelpMessage(HWND hwnd)
-- void printToTerminal(readFileConfig* rfConfig, RECT windowCoordinates,
--		unsigned* x, unsigned* y, TEXTMETRIC textMetrics, char* readBuffer)
--
-- DATE: September 30, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- NOTES:
-- The application will emulate a dummy terminal with read and write capabilities. It utilizes RS232
-- serial ports to open a pair of COM ports. The program has two possible modes: CONNECT and COMMAND.
--
-- COMMAND MODE:
-- When the application is open, it will automatically enter COMMAND mode and keyboard input is directly outputted to the window.
-- From the menu bar, the user has the ability to open a COM port to perform writing to. Selection of one of the options in the
-- 'Connect' menu will open the port, and a dialog box wil be shown for the user to input the desired port configurations. The user
-- has the ability to configure the baud rate, bytes to send, parity bits, stop bits, and the flow control. Pressing
-- 'OK' will configure the port with the selected configurations and take the user into CONNECT mode if no errors occur.
--
-- CONNECT MODE:
-- Once in CONNECT mode, all keyboard input will be directly written to the serial port that was opened and configured by the user.
-- In addition, any keyboard input received from the serial port will be displayed in the application window. At any time, the user
-- may hit the 'ESC' key to immediately exit out of CONNECT mode. This will close the port and return the user to COMMAND mode.
--
-- Note that the serial port will be opened and continuously waiting for input until the user has exited the program.
----------------------------------------------------------------------------------------------------------------------*/

#define STRICT

#include <windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "application.h"
#include "physical.h"
#include "resource.h"
#include "session.h"

#pragma warning (disable: 4096)
using namespace std;

static const TCHAR	Name[] = TEXT("Comm Shell");

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: September 30, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--	LPSTR lspszCmdParam, int nCmdShow)
--
-- RETURNS: Exit value contained in the message's wParam when terminated through a WM_QUIT message.
-- Returns 0 if the function terminates before entering the message loop.
--
-- NOTES:
-- This function is the application entry point and is used to initalize the dummy terminal application and display its main window.
-- It enteres a message loop and continuously retrieves messages from the thread's message queue and dispatches them to the specified
-- procedure designated by the user.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	HWND			hwnd;
	MSG				Msg;
	WNDCLASSEX		Wcl;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	// This states that main window is processed by this specific precedure
	Wcl.lpfnWndProc = COMTermWndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = (LPCSTR)Name;

	Wcl.lpszMenuName = TEXT("MYMENU"); // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;
	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		600, 400, NULL, NULL, hInst, NULL);
	HMENU menu = GetMenu(hwnd);
	EnableMenuItem(menu, IDM_CONNECT, MF_ENABLED);
	EnableMenuItem(menu, IDM_DISCONNECT, MF_DISABLED);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getHelpMessage
--
-- DATE: September 30, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- void getHelpMessage(HWND hwnd)
--				HWND hwnd: handle to the window to display the help for
-- RETURNS: void
--
-- NOTES:
-- This function simply prints the desired text onto the MessageBox to help the user using the application.
----------------------------------------------------------------------------------------------------------------------*/
void getHelpMessage(HWND hwnd) {
	MessageBox(hwnd, "Communications Shell 1.0.0\nCreated by Michael Yu.\n\nThis application allows connection to a serial port for which the user can perform I/O operations.\n\n To initialize a session, Go into Settings > Port > Connect, and choose your desired port to open.\n\nOnce connected, the application will create a reading thread to receive input from the serial port. Keyboard input will be output to the serial port.", "Help", MB_OK);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: changeMenuItemState
--
-- DATE: September 30, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- void changeMenuItemState(HMENU menu)
--				HMENU menu: handle to the menu to change menu item states
-- RETURNS: void
--
-- NOTES:
-- If the application is in connect mode, this function will disable all menu item selections under "Connect", as well as
-- enable the 'Disconnect' menu item. If the application is in command mode, this function will enable all 
-- menu item selections under "Connect", and disable the 'Disconnect' menu item.
----------------------------------------------------------------------------------------------------------------------*/
void changeMenuItemState(HMENU menu) {
	if (isConnected) {
		EnableMenuItem(menu, IDM_DISCONNECT, MF_ENABLED);
		EnableMenuItem(menu, IDM_COM1, MF_DISABLED);
		EnableMenuItem(menu, IDM_COM2, MF_DISABLED);
		EnableMenuItem(menu, IDM_COM3, MF_DISABLED);
		EnableMenuItem(menu, IDM_COM4, MF_DISABLED);
	}
	else {
		EnableMenuItem(menu, IDM_DISCONNECT, MF_DISABLED);
		EnableMenuItem(menu, IDM_COM1, MF_ENABLED);
		EnableMenuItem(menu, IDM_COM2, MF_ENABLED);
		EnableMenuItem(menu, IDM_COM3, MF_ENABLED);
		EnableMenuItem(menu, IDM_COM4, MF_ENABLED);
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: void printToTerminal
--
-- DATE: September 30, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- void printToTerminal(readFileConfig* rfConfig, RECT windowCoordinates,
--		unsigned* x, unsigned* y, TEXTMETRIC textMetrics, char* readBuffer)
--				readFileConfig* rfConfig: pointer to the readFileConfig that stores the required Device Context and HWND
--				RECT windowCoordinates:	stores the current coordinates of the application window
--				unsigned* x:			stores the current x location in the application window
--				unsigned* y:			stores the current y location in the application window
--				TEXTMETRIC textMetrics: stores the calculated height and width of the character spacing
--				char* readBuffer:		buffer that contains the word that is to be printed
--
-- RETURNS: void
--
-- NOTES:
-- Receives the readBuffer containing the characters to be printed to the terminal. Prior to printing,
-- it calculates the necessary location of the current caret and sets the spacing of the location for the next
-- character to be printed.
----------------------------------------------------------------------------------------------------------------------*/
void printToTerminal(readFileConfig* rfConfig, RECT windowCoordinates,
	unsigned* x, unsigned* y, TEXTMETRIC textMetrics, char* readBuffer) {
	if ((unsigned long)windowCoordinates.right <= *x) {
		*y += textMetrics.tmAscent;
		*x = 0;
	}
	rfConfig->hdc = GetDC(rfConfig->hwnd);
	*x += textMetrics.tmMaxCharWidth;
	TextOut(rfConfig->hdc, *x, *y, (LPCSTR)readBuffer, strlen(readBuffer));
	ReleaseDC(rfConfig->hwnd, rfConfig->hdc);
}

