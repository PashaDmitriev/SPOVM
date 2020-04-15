#include <windows.h>
#include <conio.h>
#include <iostream>
#include <vector>

#define MAX_COUNT 10

using namespace std;

PROCESS_INFORMATION CreateNewProcess(char* path, char *commandline);
void InitialProcess(char *path); 
void printString(int ProccessNumber);

char strings[10][30] = {{"1) First process"}, {"2) Second process"}, {"3) Third process"}, {"4) Fourth process"}, {"5) Fifth process"},
{"6) Sixth process"}, {"7) Seventh process"}, {"8) Eighth process"}, {"9) Ninth process"}, {"10) Tenth process"}};

int getch_noblock() 
{
    if (_kbhit())
        return _getch();
    else
        return -1;
}


void main(int argc, char* argv[])
{
    if(argc == 2) { printString(atoi(argv[1])); } // Если дочерний процесс
    else InitialProcess(argv[0]);						// Если родительский процесс
    return;
}

void InitialProcess(char *path)
{
	char ch = 0;
	int currentNum = 0;
	bool flag = false;
																	
	vector<HANDLE> closeEvents;
	vector<HANDLE> CanPrintEvents;
	char EventID[30];

	PROCESS_INFORMATION procInf[MAX_COUNT];

	while(1)
	{
		Sleep(1);
		ch = getch_noblock();
		switch(ch)
		{			
		case '+':
			{
				if(closeEvents.size() < MAX_COUNT)
				{					
					sprintf(EventID," %d", closeEvents.size()+1);
					closeEvents.push_back(CreateEvent(NULL ,FALSE, FALSE, EventID )); // нач. состояние - несигнальное, автосброс

					sprintf(EventID," %dp", CanPrintEvents.size()+1);
					CanPrintEvents.push_back(CreateEvent(NULL ,TRUE, FALSE, EventID )); // Ручной сброс

					procInf[closeEvents.size()-1] = CreateNewProcess( path, EventID );
				}	
				break;
			}

		case '-':
			{
				if(closeEvents.size() > 0)
				{ 
					SetEvent(closeEvents.back()); 
					WaitForSingleObject(procInf[closeEvents.size()-1].hProcess,INFINITE); // Ожидаем завершения последнего процесса

					if(!CloseHandle(closeEvents.back()))
						printf( "Close Handle failed (%d)\n", GetLastError());
					if(!CloseHandle(CanPrintEvents.back()))
						printf( "Close Handle failed (%d)\n", GetLastError());

					closeEvents.pop_back();
					CanPrintEvents.pop_back();
					if (currentNum >= closeEvents.size()) 
					{
						currentNum = 0;
						flag = true;
					}
				}
				break;
			}
		case 'q':
			{
				if(closeEvents.size() > 0)
				{							
					while(closeEvents.size() > 0)
					{							
						SetEvent(closeEvents.back()); 
						WaitForSingleObject(procInf[closeEvents.size()-1].hProcess,INFINITE); // Ожидаем завершения последнего процесса

						if(!CloseHandle(closeEvents.back()) || !CloseHandle(CanPrintEvents.back()) || !CloseHandle(procInf[closeEvents.size()-1].hProcess)
							|| !CloseHandle(procInf[closeEvents.size()-1].hThread))
							printf( "Close Handle failed (%d)\n", GetLastError());

						closeEvents.pop_back();
						CanPrintEvents.pop_back();
					}	
					currentNum = 0;
				}
				printf("\n\n");
				system("pause");
				return;
			}

		}
		if(CanPrintEvents.size() > 0 && WaitForSingleObject(CanPrintEvents[currentNum],0) == WAIT_TIMEOUT ) // Пока событие печати последнего процесса 
																											// не будет сброшено
		{
			if(currentNum >= CanPrintEvents.size()-1 ) currentNum = 0;
			else if(!flag) currentNum++;
			flag = false;

			SetEvent(CanPrintEvents[currentNum]);
		}


	}	
}

PROCESS_INFORMATION CreateNewProcess(char* path, char *commandline)
{
	STARTUPINFO si;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	PROCESS_INFORMATION infProc;
	ZeroMemory( &infProc, sizeof(infProc) );
	if(!CreateProcess(path, commandline, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &infProc)) 
		printf( "Create Process failed (%d)\n", GetLastError());
	return infProc;
}

void printString(int processNumber)
{
	char EventID[30];
	sprintf(EventID," %d", processNumber);
	HANDLE closeEvent = OpenEvent(EVENT_ALL_ACCESS,FALSE,EventID);

	sprintf(EventID," %dp", processNumber);
	HANDLE CanPrintEvent = OpenEvent(EVENT_ALL_ACCESS,FALSE,EventID);

	while(1)
	{
		//Sleep(1);
		if(WaitForSingleObject(CanPrintEvent, 1) == WAIT_OBJECT_0) 
		{ 			
			for(int i=0; i<strlen(strings[processNumber-1]); i++)        // Печать строки
			{
				if(WaitForSingleObject(closeEvent, 0) == WAIT_OBJECT_0)  // Если установлено событие закрытия данного процесса	
				{
					CloseHandle(closeEvent); CloseHandle(CanPrintEvent); 
					return;
				}				
				printf("%c",strings[processNumber-1][i]);
				Sleep(100);
			}
			printf("\n");
			ResetEvent(CanPrintEvent);
		}

		if(WaitForSingleObject(closeEvent, 0) == WAIT_OBJECT_0)
		{
			CloseHandle(closeEvent); CloseHandle(CanPrintEvent); 
			return;
		}
	}
	return;
}