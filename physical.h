#pragma once

struct readFileConfig {
	HANDLE hComm;
	HDC hdc;
	HWND hwnd;
	LPVOID lpBuffer;
	DWORD dwNumberOfBytesToRead;
	LPDWORD dwNumberOfBytesRead;
	OVERLAPPED lpOverlapped;
};

/*
Opens the specified port and returns the handle to it.
*/
HANDLE openSerialPort(LPCSTR lpszCommName);

/*
Configures the specified port with the COMMCONFIG and COMMTIMEOUT structures.
*/
bool configurePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize);

/*
Configures the COMMTIMEOUT structure to be set for the port.
*/
bool setPortTimeout(HANDLE hComm, LPCOMMTIMEOUTS portTimeoutConfig);


/*
Writes the character received from keyboard input to the serial port.
*/
void writeToPort(HANDLE hComm, TCHAR wParam, DWORD dwNumOfBytesToWrite, DWORD dwNumBytesWritten, OVERLAPPED overlapped);

/*
Creates the reading thread.
*/
HANDLE openReadingThread(readFileConfig* rfConfig, DWORD IDThread);

/*
Main function passed to the reading thread to process characters read from serial port.
*/
DWORD WINAPI readFromPort(LPVOID readFunctionParams);

/*
Closes the handle to the open port.
*/
//bool closePort(HANDLE hComm);