#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_COUNT 10

char strings[10][30] = {{"\r\n1) First process"}, {"\r\n2) Second process"}, {"\r\n3) Third process"}, {"\r\n4) Fourth process"}, {"\r\n5) Fifth process"} , {"\r\n6) Sixth process"}, {"\r\n7) Seventh process"},{"\r\n8) Eighth process"}, {"\r\n9) Ninth process"}, {"\r\n10) Tenth process"}};

int printFlag = 0;
int flagEnd = 1;

struct sigaction printSignal, endSignal;

void canPrint(int signo)
{
  printFlag = 1;
}

void setEndFlag(int signo)
{
  flagEnd = 1;
}

int main(void)
{
  int currentNum = 0;
  int processNumber = 0;
  
  int flag = 0;
  
  initscr();
  clear();
  noecho();
  refresh();
  
  printSignal.sa_handler = canPrint;
  sigaction(SIGUSR1,&printSignal,NULL);
  
  endSignal.sa_handler = setEndFlag;
  sigaction(SIGUSR2,&endSignal,NULL);
  
  char c = 0;
  int i = 0;
  
  pid_t cpid[MAX_COUNT];
  
  
  while(c!='q')
  {
    c = getchar();
    
    switch(c)
    {
      case '+':	  
	if(processNumber < MAX_COUNT)
	{
	  cpid[processNumber] = fork();
	  processNumber++;
	  
	  switch(cpid[processNumber-1])
	  {
	    case 0: 
	      flagEnd = 0;
	      while(!flagEnd)
	      {
		usleep(100000);  
		if(printFlag)
		{
		  
		    if(flagEnd) 		
		      return 0;
		    for(i=0; i<strlen(strings[processNumber-1]); i++)
		  {
		    printf("%c",strings[processNumber-1][i]);
		    refresh();
		    usleep(50000);
		  }
		  refresh();
		  printFlag = 0;
		  kill(getppid(),SIGUSR2); 
		}
	      }
	      return 0;
	      break;
	      
	    case -1:
	      printf("Error!\n");
	      break;
	      
	    default: 
	      break;	    
	  }
	}
	break;
	
      case '-':
	if(processNumber > 0)
	{
	  kill(cpid[processNumber-1],SIGUSR2);
	  waitpid(cpid[processNumber-1],NULL,WNOHANG); 

	  processNumber--;  
	  
	  if (currentNum >= processNumber)  
	  {
	    currentNum = 0;	
	    flag = 1;           
	    
	    flagEnd = 1;	
	  }

	}	    
	    break;
    }
    
    if(flagEnd && processNumber>0)   
    {
      flagEnd = 0;
      if(currentNum >= processNumber - 1)  
	currentNum = 0;		    
      else if(!flag) currentNum++;   
				         
      flag = 0;
      
      kill(cpid[currentNum],SIGUSR1); 
    } 
    refresh();        
  }
  
  if(cpid[processNumber-1]!=0)
    for(;processNumber>=0;processNumber--)
    {
      kill(cpid[processNumber-1],SIGUSR2);
      waitpid(cpid[processNumber-1],NULL,WNOHANG);
    }
  
  clear();  
  endwin();
  
  return 0;
}

