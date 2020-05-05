/*	Создаются два процесса: клиентский и серверный. Серверный процесс
ждет ввода пользователем текстовой строки и по нажатию клавиши Enter про-
изводит следующие действия:
	•клиентский процесс уведомляется о том, что серверный процесс го-
тов начать передачу данных (синхронизация);
	•серверный процесс передает полученную от пользователя строку
клиентскому процессу, используя либо каналы, либо сегменты разделяемой па-
мяти / файловые проекции;
	•клиентский процесс выводит полученную строку на экран и уве-
домляет серверный процесс об успешном получении строки;
	•серверный процесс ожидает ввода следующей строки и т.д.
	В данной работе продолжается освоение синхронизации процессов. Уве-
домление процессов должно производиться посредством семафоров. Реализа-
ция механизма непосредственной передачи данных остается на выбор студента,
однако в теории освоены должны быть все варианты.*/

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sstream>

#define KEY 1147

using namespace std;

union semun {
int val;
struct semid_ds *buf;
unsigned short *array;
struct seminfo *__buf;
};

int main()
{

	initscr();
    clear();
    refresh();

    union semun arg;

    int sem_id;

    struct  sembuf sem_wait = {0, -1, 0};
    struct  sembuf sem_cont = {0, 0, 0};

 	sem_id = semget(KEY, 1, 0666 | IPC_CREAT);

 	if(sem_id < 0)
 	{
 		printf("Error! Semaphore is not created\r\n");
 		exit(-1);
 	}


    arg.val = 0;
  	semctl(sem_id, 0, SETVAL, arg);

    char w_buf[50] = "";
    char r_buf[50] = "";

    pid_t process;
   	int read_write[2];

   	start_color();
   	clear();
   	refresh();

   	init_pair(1, COLOR_WHITE, COLOR_RED);
    init_pair(2, COLOR_WHITE, COLOR_GREEN);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);

    attron(COLOR_PAIR(1));
	printw(">>Server:"); 
	refresh();
	
	printf(" Connect to client...\r\n\n");																
   	refresh();
   	
   	sleep(1.5);
    
    if(pipe(read_write) == -1)
   	{
   		printf("\n\r Error!\n\rChannel is not created!\n\rExit...");
   		return 0;
   	}

   	attron(COLOR_PAIR(1));
	printw(">>Server:"); 
	refresh();
 
	printf(" Connected done!\r\n\n");
   	refresh();

   	sleep(1.5);

    process = fork();

    
   	switch(process)
   	{
   		case -1:

   			printf("\n\r Error to create process!\n");
   			return 0;

   		case 0:

   			attron(COLOR_PAIR(2));
			printw(">>Client:"); 
			refresh();
   			
   			printf(" Client is connected too!\r\n\n");
 			refresh();
 			
   			while(1)
   			{
   			
   			semop(sem_id, &sem_wait, 1);

   			close(read_write[1]);

 			read(read_write[0], r_buf, sizeof(r_buf));
   			
   			attron(COLOR_PAIR(2));
			printw(">>Client:"); 	
			refresh();
   			
   			attron(COLOR_PAIR(4));
   			printw(" Server string: ");
   			refresh();
   			
   			attron(COLOR_PAIR(3));
   			addstr(r_buf);

   			memset(r_buf, 0, 50);
   			
   			refresh();

   			getch();
			clear();
			refresh();
   			
   			arg.val = 0;
			semctl(sem_id, 0, SETVAL, arg);  			
   			
   			}
   	}
    
    sleep(1.5);
   
   	while(1)
   	{

   		semop(sem_id, &sem_cont, 1);

   		close(read_write[0]);
   	
   		attron(COLOR_PAIR(1));
		printw(">>Server:");
		refresh(); 

		attron(COLOR_PAIR(4));
   		printw(" Input string: ");
   		refresh();
   	
   		attron(COLOR_PAIR(3));
   		refresh();
   		getstr(w_buf);

   	
   		printf("\r\n");

   		write(read_write[1], w_buf, strlen(w_buf));

   		memset(w_buf, 0, 50);

   		sem_wait = {0, 2, 0};
   		
   		semop(sem_id,&sem_wait, 1);
  }

   	refresh();
   	getch();
    endwin();

	return 0;
}

