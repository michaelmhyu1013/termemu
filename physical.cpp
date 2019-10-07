/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: physical.cpp
--
-- PROGRAM: COMP3980Assignment1
--
-- FUNCTIONS:
-- bool configurePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize)
-- bool setPortTimeout(HANDLE hComm, LPCOMMTIMEOUTS portTimeoutConfig)
-- DWORD WINAPI readFromPort(LPVOID readFunctionParams)
-- HANDLE openSerialPort(LPCTSTR lpszCommName)
-- void writeToPort(HANDLE hComm, TCHAR wParam, DWORD dwNumOfBytesToWrite, DWORD dwNumBytesWritten, OVERLAPPED overlapped)
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
-- This is responsible for opening the port and initializing it with the desired configurations specified by 
-- the user. 
--
-- In addition, it is responsible for writing characters received from keyboard input to the serial port and 
-- reading and displaying all characters from the serial port while it is connected.
----------------------------------------------------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>
#include "resource.h"

#include "application.h"
#include "physical.h"
#include <string>
#include <iostream>


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: openSerialPort
--
-- DATE: September 29, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- INTERFACE: HANDLE openSerialPort(LPCSTR lpszCommName)
--					LPCSTR lpszCommName: contains the name of the specified COM port to be opened
--
-- RETURNS: The HANDLE to the port if it is successfully opened, else INVALID_HANDLE_VALUE is returned.
--
-- NOTES:
-- Call this function to open the port with CreateFile, specifying the necessary read and write
-- settings for the port.
----------------------------------------------------------------------------------------------------------------------*/
HANDLE openSerialPort(LPCSTR lpszCommName) {
	if ((hComm = CreateFile(lpszCommName, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL))
		== INVALID_HANDLE_VALUE) {
		MessageBox(NULL, (LPCSTR)"Error opening COM port.", (LPCSTR)"", MB_OK);
		return INVALID_HANDLE_VALUE;
	}
	else {
		isConnected = !isConnected;
		MessageBox(NULL, (LPCSTR)"Port opened.", (LPCSTR)"", MB_OK);
		return hComm;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: configurePort
--
-- DATE: September 29, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- INTERFACE: bool configurePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize)
--
-- RETURNS: 1 if the port is successfully configured with the COMMCONFIG structure and the COMMTIMEOUT
-- structure.
--
-- NOTES:
-- Configures the port that is passed in with the desired COMMCONFIG structure. This function creates
-- its own COMMTIMEOUT structure through setPortTimeout() to configure the port.
----------------------------------------------------------------------------------------------------------------------*/
bool configurePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize) {
	COMMTIMEOUTS portTimeoutConfig;
	if (!SetCommConfig(hComm, &cc, dwSize)) {
		MessageBox(NULL, (LPCSTR)"Setting COMMCONFIG structure failed.", (LPCSTR)"Error", MB_OK);
		return 0;
	}
	if (!setPortTimeout(hComm, &portTimeoutConfig)) {
		MessageBox(NULL, (LPCSTR)"Setting port timeout failed.", (LPCSTR)"Error", MB_OK);
		return 0;
	}
	MessageBox(NULL, (LPCSTR)"Port successfully configred. Entering CONNECT mode.", (LPCSTR)"Success", MB_OK);
	return 1;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setPortTimeout
--
-- DATE: September 29, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- INTERFACE: bool setPortTimeout(HANDLE hComm, LPCOMMTIMEOUTS portTimeoutConfig)
--				HANDLE hComm: HANDLE to the port to configure the timeout
--				LPCOMMTIMEOUTS: pointer to COMMTIMEOUT to be set to the HANDLE hComm
--
-- RETURNS: 1 if the timeout of the port is successfully configured for COMMTIMEOUT, else 0.
--
-- NOTES:
-- Calling this function will automatically setup the COMMTIMEOUT of the specified
-- HANDLE that is passed in with pre-defined values.
----------------------------------------------------------------------------------------------------------------------*/
bool setPortTimeout(HANDLE hComm, LPCOMMTIMEOUTS portTimeoutConfig)
{
	portTimeoutConfig->ReadIntervalTimeout = 50;
	portTimeoutConfig->ReadTotalTimeoutConstant = 50;
	portTimeoutConfig->ReadTotalTimeoutMultiplier = 10;
	portTimeoutConfig->WriteTotalTimeoutMultiplier = 10;
	portTimeoutConfig->WriteTotalTimeoutConstant = 50;
	return SetCommTimeouts(hComm, portTimeoutConfig);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:		writeToPort
--
-- DATE:			September 29, 2019
--
-- REVISIONS:		(Date and Description)
--
-- DESIGNER:		Michael Yu
--
-- PROGRAMMER:		Michael Yu
--
-- INTERFACE:		void writeToPort(HANDLE hComm, TCHAR wParam, DWORD dwNumOfBytesToWrite, DWORD dwNumBytesWritten, OVERLAPPED overlapped)
--						HANDLE hComm: handle of the file to perform writing to
--						TCHAR wParam: string that is to be written to the file
--						DWORD dwNumOfBytesToWrite: the number of bytes to be written to the file or device
--						DWORD dwNumBytesWritten: pointer to the variable that receives the number of bytes written to
--						OVERLAPPED overlapped: OVERLAPPED structure that dictates asynchronous I/O
--
-- RETURNS:			void.
--
-- NOTES:			This function calls WriteFile() to perform asynchronous I/O writing to the port designed in
--					hComm.
----------------------------------------------------------------------------------------------------------------------*/
void writeToPort(HANDLE hComm, TCHAR wParam, DWORD dwNumOfBytesToWrite, DWORD dwNumBytesWritten, OVERLAPPED overlapped) {
	if (!WriteFile(hComm, &wParam, 1, NULL, &overlapped)) {
		MessageBox(NULL, (LPCSTR)"WriteFile failed.", (LPCSTR)"Error", MB_OK);
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readFromPort
--
-- DATE: September 29, 2019
--
-- REVISIONS: None
--
-- DESIGNER: Michael Yu
--
-- PROGRAMMER: Michael Yu
--
-- INTERFACE: DWORD WINAPI readFromPort(LPVOID readFunctionParams)
--				LPVOID readFunctionParams: pointer the struct readFileConfig to perform reading, members below:
--													HANDLE hComm;
--													HDC hdc;
--													HWND hwnd;
--													LPVOID lpBuffer;
--													DWORD dwNumberOfBytesToRead;
--													LPDWORD dwNumberOfBytesRead;
--													OVERLAPPED lpOverlapped;
--
-- RETURNS: void.
--
-- NOTES:
-- This function is the LP_THREAD_START_ROUTINE for the reading thread. It performs asynchronous reading while
-- the application is connected to a valid host. If a character is successfully read from the serial port and stored
-- in the designated buffer, the character will be passed to the application to display on the window terminal.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI readFromPort(LPVOID readFunctionParams) {
	OVERLAPPED	overlapped = { 0 };
	char		readBuffer[10] = { 0 };
	DWORD		threadEvent{ 0 };
	RECT		windowCoordinates{ 0 };
	TEXTMETRIC	textMetrics{ 0 };
	static unsigned y = 0;
	static unsigned x = 0;

	readFileConfig* rfConfig = (readFileConfig*)readFunctionParams;
	GetClientRect(rfConfig->hwnd, &windowCoordinates);

	if (!SetCommMask(hComm, EV_RXCHAR)) {
		MessageBox(NULL, (LPCSTR)"SetCommMask failed.", (LPCSTR)"Error", MB_OK);
	}
	while (isConnected) {
		if (WaitCommEvent(rfConfig->hComm, &threadEvent, 0)) {
			if (ReadFile(rfConfig->hComm, readBuffer, 1, NULL, &overlapped)) {
				GetTextMetrics(rfConfig->hdc, &textMetrics);
				printToTerminal(rfConfig, windowCoordinates, &x, &y, textMetrics, readBuffer);
			}
		}
	}
	return 1;
}

