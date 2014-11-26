#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

int main() {
  
  char *command;
  char input[256];
  char *args_array[10];
  char *comm_array[10];
  int num_args = 0;
  int pid;
  int *status;
  siginfo_t *infop;
  char cwd[256];
  int num_commands;
  
  while(1) {
    printf("seashell:%s$ ", getcwd(cwd, sizeof(cwd)));
    fgets(input, sizeof(input), stdin);
    input[strlen(input)-1]='\0';
 
    char *count_commands = input;

    while(*count_commands) {
      if (*count_commands == ';'){
	num_commands++;
      }
      //printf("teehee%s\n",count_commands);
      count_commands++;
    }
    printf("%d\n",num_commands);

    comm_array[0] = strtok(input, ";");
    //printf("%s\n",comm_array[0]);
    int i = 1;
    while (i < num_commands) {
      comm_array[i] = strtok(NULL, ";");
      printf("%d: %s\n",i,comm_array[i]);
      if(i > num_commands)
	return;
      i++;
    }
    printf("num_commands: %d\n",num_commands);
    exit(0);
    for( i = 0; i < num_commands; i++)
    {
      
      command = comm_array[i];
      printf("command: %s\n",command);
      char *p = command;

      while (*p){
	if (*p == ' '){
	  num_args++;
	}
	p++;
      }

      char *comm = strtok(command, " ");
    
      if (!strcmp(comm,"exit")) {
	//printf(":%s:", comm);
	exit(0);
      }
    
      args_array[0] = comm;
    
      if (num_args== 0) {
	args_array[1]=NULL;
      }
    
      else {
	i = 1;
	while (i <= num_args) {
	  args_array[i] = strtok(NULL, " ");
	  i++;
	}
	args_array[i]=NULL;
      }

      if (!strcmp(comm,"cd")) {
	if (!args_array[1]) {
	  chdir(getenv("HOME"));
	}
	chdir(args_array[1]);
	//execvp(args_array[0], args_array);
      }
      else {
	pid = fork();
	if(!pid) {
	  execvp(args_array[0], args_array);	
	  return WEXITSTATUS(105);
	  exit(0);
	}
	waitid(P_PID, pid, infop, WEXITED);
      }
    }
  }
    
  return 0;
}

