#include <iostream>
#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>

using namespace std;

enum ConsoleColor {
Black = 0,
Blue = 1,
Green = 2,
Cyan = 3,
Red = 4,
Magenta = 5,
Brown = 6,
LightGray = 7,
DarkGray = 8,
LightBlue = 9,
LightGreen = 10,
LightCyan = 11,
LightRed = 12,
LightMagenta = 13,
Yellow = 14,
White = 15
};

int main()
{
	Sleep(2000);

	HANDLE hConsole;
	HANDLE pchannel;
	HANDLE semaphore;
	char c;

	DWORD cbR;
	
	char str_client[200];

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	pchannel = CreateFile(L"\\\\.\\pipe\\new_pipe", GENERIC_ALL, 0, NULL, OPEN_EXISTING, 0, NULL);
	
	semaphore = CreateSemaphore(NULL, 1, 1, L"sem");

	if(pchannel == INVALID_HANDLE_VALUE)
	{

		SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | Cyan)); cout << endl << "Client: ";
		SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | White)); cout << "Error!\n Client is not connected!\nExit...";
		Sleep(3000);

		CloseHandle(pchannel);

		return 0;
	}

	if(!semaphore)
	{
		SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | LightGreen)); cout << endl << "Client: ";
		SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | White)); cout << "Error!\n Semaphore is't created!\nExit..." << endl; 
		Sleep(3000);

		CloseHandle(semaphore);
		CloseHandle(pchannel);
	
		return 0; 
	}

	Sleep(2000);
	SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | Cyan)); cout << endl << "Client: ";
	SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | White)); cout << "Client is connected!" << endl;
	
	Sleep(2000);

	while(1)
	{
	
	WaitForSingleObject(semaphore, INFINITE);
	
	if(!ReadFile(pchannel, str_client, 512, &cbR, NULL))
	{
		SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | Cyan)); cout << endl << "Client: ";
		SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | White)); cout << "Error!\nFile was not readed!\nExit...";

		Sleep(3000);
		return 0;
	}
	
	SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | Cyan)); cout << endl << "Client: ";
	SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | White)); cout << "Server string: "; 
	SetConsoleTextAttribute(hConsole, (WORD) ((Black << 4) | Yellow));

	puts(str_client);
	
	ReleaseSemaphore(semaphore, 1, NULL);
	}
	return 0;

}
