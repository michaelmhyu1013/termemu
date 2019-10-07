/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: session.cpp -
--
-- PROGRAM: COMP3980Assignment1
--
-- FUNCTIONS:
-- LRESULT CALLBACK COMTermWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
-- int connect(HANDLE hComm, LPCTSTR lpszCommName, HWND hwnd)
-- int exitSession(HANDLE hComm, HMENU menu) 
-- void initializeConfig(readFileConfig* rfConfig, HANDLE hComm, HWND hwnd)
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
-- The session is responsible for allowing the user to establish a session with the desired port that they choose.
-- It allows the user to connect to a port through the physical layer, as well as the ability to terminate
-- the session.
-- 
-- It contains the main WndProc to process all messages received by the window application. All reading, writing, and menu
-- item related messages are passed to the session to process prior to handling.
--
-- This creates the thread for reading from the serial port once a valid handle to the port is established. All characters
-- received from the serial port is processed by readFromPort.
----------------------------------------------------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>

#include "application.h"
#include "resource.h"
#include "session.h"
#include "physical.h"

bool	isConnected = false;
HANDLE	hComm = { 0 };

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: COMTermWndProc
--
-- DATE: September 29, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- LRESULT CALLBACK COMTermWndProc(HWND hwnd, UINT Message,
-- WPARAM wParam, LPARAM lParam);
--				HWND hwnd:		handle of the window application
--				UINT Message:	Message received by window
--				WPARAM wParam:	Additional message information
--				LPARAM lParam:	Additional message information
-- 
-- RETURNS: Returns the result of the message that was processed and depends on the message that was sent.
--
-- NOTES:
-- This function defines the desired processing method for messages that are sent to the current application window. It
-- dictates the ability to open and close ports. In addition, it processes keyboard input from the user, for both COMMAND
-- mode and CONNECT mode.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK COMTermWndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	char				writeBuffer[80]{ 0 };
	char				readBuffer[80]{ 0 };
	DWORD				readThreadId{};
	DWORD				dwNumBytesWritten = NULL;
	HANDLE				hReadThread{ 0 };
	HDC					hdc;
	HMENU menu = GetMenu(hwnd);
	LPCSTR				lpszCommName;
	OVERLAPPED			overlapped{ 0 };
	PAINTSTRUCT			paintstruct;
	static unsigned k = 0;

	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_COM1:
			lpszCommName = TEXT("COM1");
			hComm = openSerialPort(lpszCommName);
			if (connect(lpszCommName, hwnd)) {
				changeMenuItemState(menu);
				readFileConfig* rfConfig = new readFileConfig();
				initializeConfig(rfConfig, hComm, hwnd);
				hReadThread = CreateThread(NULL, 0, readFromPort, rfConfig, 0, &readThreadId);
			}
			break;
		case IDM_COM2:
			lpszCommName = TEXT("COM2");
			hComm = openSerialPort(lpszCommName);
			if (connect(lpszCommName, hwnd)) {
				changeMenuItemState(menu);
				readFileConfig* rfConfig = new readFileConfig();
				initializeConfig(rfConfig, hComm, hwnd);
				hReadThread = CreateThread(NULL, 0, readFromPort, rfConfig, 0, &readThreadId);
			}
			break;
		case IDM_COM3:
			lpszCommName = TEXT("COM3");
			hComm = openSerialPort(lpszCommName);
			if (connect(lpszCommName, hwnd)) {
				changeMenuItemState(menu);
				readFileConfig* rfConfig = new readFileConfig();
				initializeConfig(rfConfig, hComm, hwnd);
				hReadThread = CreateThread(NULL, 0, readFromPort, rfConfig, 0, &readThreadId);
			}
			break;
		case IDM_COM4:
			lpszCommName = TEXT("COM4");
			hComm = openSerialPort(lpszCommName);
			if (connect(lpszCommName, hwnd)) {
				changeMenuItemState(menu);
				readFileConfig* rfConfig = new readFileConfig();
				initializeConfig(rfConfig, hComm, hwnd);
				hReadThread = CreateThread(NULL, 0, readFromPort, rfConfig, 0, &readThreadId);
			}
			break;
		case IDM_DISCONNECT:
			if (exitSession(hComm, menu)) {
				changeMenuItemState(menu);
				MessageBox(hwnd, (LPCSTR)TEXT("Returning to COMMAND mode."), (LPCSTR)TEXT("Info"), MB_OK);
			}
			break;
		case IDM_EXIT:
			PostQuitMessage(0);
			break;
		case IDM_HELP:
			getHelpMessage(hwnd);
			break;
		}
		break;
	case WM_CHAR:
		hdc = GetDC(hwnd);
		if (isConnected) {
			if (wParam == VK_ESCAPE) {
				if (exitSession(hComm, menu)) {
					changeMenuItemState(menu);
					MessageBox(hwnd, (LPCSTR)TEXT("Returning to COMMAND mode."), (LPCSTR)TEXT("Info"), MB_OK);
				}
			}
			//else if (wParam == VK_BACK) {
				//processBackSpace();
			//}
			else {
				WriteFile(hComm, &wParam, 1, NULL, &overlapped);
				//writeToPort(hComm, wParam, strlen(writeBuffer), dwNumBytesWritten, overlapped);
				ReleaseDC(hwnd, hdc);
			}
		}
		break;
	case WM_PAINT:	
		hdc = BeginPaint(hwnd, &paintstruct); 
		TextOut(hdc, 0, 0, (LPCSTR)readBuffer, strlen(readBuffer));
		EndPaint(hwnd, &paintstruct);
		break;
	case WM_DESTROY:	
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connect
--
-- DATE: September 29, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- INTERFACE: int connect(HANDLE hComm, LPCTSTR lpszCommName, HWND hwnd)
--							
-- RETURNS: 1 if the port is successfully configured with the parameters received from
-- the user, else 0.
--
-- NOTES:
-- Call this function to configure the opened port and enter CONNECT mode.
----------------------------------------------------------------------------------------------------------------------*/
int connect(LPCTSTR lpszCommName, HWND hwnd) {
	COMMCONFIG			cc;
	cc.dwSize = sizeof(COMMCONFIG);
	cc.wVersion = 0x100;
	cc.dcb.DCBlength = sizeof(DCB);
	if (hComm == INVALID_HANDLE_VALUE) {
		hComm = NULL;
		return 0;
	}
	GetCommConfig(hComm, &cc, &cc.dwSize);
	if (!CommConfigDialog(lpszCommName, hwnd, &cc)) {
		hComm = NULL;
		MessageBox(NULL, (LPCSTR)"Failed to retrieve port configurations.", (LPCSTR)"Error", MB_OK);
		return 0;
	}
	return configurePort(hComm, cc, cc.dwSize);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: exitSession
--
-- DATE: September 29, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- INTERFACE: int exitSession(HANDLE hComm, HMENU menu)
--					HANDLE hComm: handle to the port to be closed
--					HMENU menu: Handle to the windows menu items that will be enabled or disabled
--
-- RETURNS: 1 if the handle to the port is successfully closed, else false
--
-- NOTES:
-- Call this function to close the handle to the open port. Upon successfully closing the port, the application
-- will enter COMMAND mode by setting the global isConnected to false.
----------------------------------------------------------------------------------------------------------------------*/
int exitSession(HANDLE hComm, HMENU menu) {
	if (!CloseHandle(hComm)) {
		return 0;
	}
	isConnected = !isConnected;
	return 1;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initializeConfig
--
-- DATE: September 29, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- void initializeConfig(readFileConfig* rfConfig, HANDLE hComm, HWND hwnd)
--					readFileConfig* rfConfig: pointer to the readFileConfigStruct 
--						that will store the current applications port handle and window handle
--					HANDLE hComm: handle to the port to be stored in rfConfig
--					HWND hwnd: handle to the window to be stored in rfConfig
--
-- RETURNS: void
--
-- NOTES:
-- This function sets the data members of the readFileConfig struct to the necessary handles in order
-- to pass these parameters into the readFromPort function in the reading thread.
----------------------------------------------------------------------------------------------------------------------*/
void initializeConfig(readFileConfig* rfConfig, HANDLE hComm, HWND hwnd) {
	rfConfig->hComm = hComm;
	rfConfig->hwnd = hwnd;
	rfConfig->hdc = GetDC(hwnd);
}