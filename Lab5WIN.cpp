
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <string>

using namespace std;

DWORD WINAPI WriterThread(PVOID pvParam);
DWORD WINAPI ReaderThread(PVOID argv);

#define read_finished_event 0
#define exit_event 1
#define write_finished_event 2

struct OperationInfo
{
    HANDLE hFile;            
	DWORD NumberOfBytes;	  
    CHAR  buf[100];			  
	DWORD  pos_in_file;		  
	DWORD  pos_out_file;	  
	OVERLAPPED Overlapped;
} info;

HINSTANCE library;
HANDLE events[3];


int main(int argc, char *argv[])
{
	if(argc!=3)
	{
		printf("arguments error");
		return 0;
	}

	HANDLE hEvent;        
	HANDLE hThreads[2];   

	hEvent = CreateEvent (NULL, FALSE, TRUE, TEXT("Event_lab5"));  

	events[write_finished_event] = CreateEvent (NULL, FALSE, TRUE, NULL);   
	events[read_finished_event] = CreateEvent (NULL, FALSE, FALSE, NULL);  
	events[exit_event] = CreateEvent (NULL, TRUE, FALSE, NULL);            
	 
	info.Overlapped.Offset = 0;
	info.Overlapped.OffsetHigh = 0;
	info.Overlapped.hEvent = hEvent;
	info.pos_out_file = 0;
	info.NumberOfBytes = sizeof(info.buf);
    
	library = LoadLibrary("library.dll");
  
	hThreads[0] = CreateThread(NULL, 0, WriterThread, (LPVOID)argv[2], 0, NULL); 
	hThreads[1] = CreateThread(NULL, 0, ReaderThread, (LPVOID)argv[1], 0, NULL); 

	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);

	CloseHandle(events[write_finished_event]);
	CloseHandle(events[read_finished_event]);
	CloseHandle(events[exit_event]);
	CloseHandle(hEvent);
	FreeLibrary(library);
	printf("\n\n");
	return 0;
}


DWORD WINAPI ReaderThread(PVOID folderPATH)
{
	string folder(((const char*)folderPATH));               
	folder.append("\\");
	string fileMask = folder + "*.txt";   
	char ReadFilePATH[MAX_PATH];

	WIN32_FIND_DATA FindFileData; 
	HANDLE find_Handle,  
           hReadFile;    

	BOOL readResult = false;

	BOOL (*Read)(OperationInfo*) = (BOOL (*)(OperationInfo*))GetProcAddress(library,"read"); 

	find_Handle = FindFirstFile(fileMask.c_str(), &FindFileData);

	if (find_Handle == INVALID_HANDLE_VALUE) 
    {
        printf (" Error: %d\n", GetLastError ());
		return 0;
    } 

	while(1)
	{   
		WaitForSingleObject(events[write_finished_event], INFINITE);
		if(readResult == false)
		{
			info.pos_in_file = 0;
			strcpy(ReadFilePATH, folder.c_str());
			strcat(ReadFilePATH, FindFileData.cFileName);  
			hReadFile = CreateFile(ReadFilePATH, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		}
		info.hFile = hReadFile;

		readResult = (Read)(&info);	

		if(readResult == false && GetLastError() == ERROR_HANDLE_EOF) 
		{
			if(FindNextFile(find_Handle, &FindFileData)) 
			{
				printf("\nRead from %s", ReadFilePATH);
				CloseHandle(hReadFile);
				SetEvent(events[write_finished_event]);
				continue;
			}
			else break;
		}
		
		SetEvent(events[read_finished_event]);
	}
	FindClose(find_Handle);
	CloseHandle(hReadFile);
	SetEvent(events[exit_event]);
	return 0;
}

DWORD WINAPI WriterThread(PVOID outFilePath)
{
	HANDLE hOutputFile = CreateFile((const char*)outFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	BOOL (*Write)(OperationInfo*) = (BOOL (*)(OperationInfo*))GetProcAddress(library,"write"); // Функция записи
	HANDLE events_[2] = {events[read_finished_event], events[exit_event]  };
	while(1)
	{   
		int event = WaitForMultipleObjects(2, events_, FALSE, INFINITE) - WAIT_OBJECT_0;
		if(event == exit_event)
			break;

		info.hFile = hOutputFile;
        (Write)(&info);
        SetEvent(events[write_finished_event]);
	}
	CloseHandle(hOutputFile);
	return 0;
}