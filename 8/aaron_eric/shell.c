#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <fcntl.h>

/* to do 
   - hash table + custom dongerinos for directory
   - pipes
   - history
   - tabs
   -  ????
   - your mom
   - easter eggs
   - asciify
   - !!!!!!!!!
   -?!??
   - ¿¿¿¿¿?
*/

#define interino char *



char ** parse_string(char * s, char * parser) {
  char ** parsed = NULL;
  char * item = strtok(s,parser);
  int numitems = 0;
  while (item) {
    parsed = realloc(parsed,sizeof(char*)*++numitems);
    parsed[numitems-1] = item;
    item = strtok(NULL,parser);
  }
  strtok(parsed[numitems-1], "\n");
  parsed = realloc(parsed,sizeof(char*)*(numitems+1));
  parsed[numitems] = 0;
  return parsed;
}

char * get_nth_donger(int n) {
  FILE * dongerinos = fopen("dongers.txt", "rb");
  fseek(dongerinos, 0, SEEK_END);
  long dongersize = ftell(dongerinos);
  rewind(dongerinos);
  char * filerino = malloc(dongersize+1);
  fread(filerino, dongersize, 1, dongerinos);
  fclose(dongerinos);
  filerino[dongersize] = 0;

  char ** parsed_dongerinos = parse_string(filerino, "\n");

  //-----------------DONGERS NEED TO BE EDITED
  int i = 0;
  for (;i<128;++i)
    printf("i is %d, donger |%s|\n",i,parsed_dongerinos[i]);

  return parsed_dongerinos[n];
}

interino main() {
  //get_nth_donger(2);
  //exit(1);
  while ("( ‾ʖ̫‾)") {
   
    //printf("MY ASS\n");
    printf("ヽ༼ຈل͜ຈ༽ﾉ: ");
    char input[256];
    fgets(input,sizeof(input),stdin);
    char ** semicolon_parsed = parse_string(input,";");
    int i = 0;
    
    for (;semicolon_parsed[i];++i) {
      char ** command = parse_string(semicolon_parsed[i]," ");
      
      if (command[0][0] == 'c' && command[0][1] == 'd' && ((command[0][2] == ' ' || command[0][2] == '\n') || !command[0][2])) {//fuck strstr
	if (command[1])
	  chdir(command[1]);
	else {
	  char * home = getenv("HOME");
	  chdir(home);
	}
      }
      if (command[0][0] == 'e' && command[0][1] == 'x' && command[0][2] == 'i' && command[0][3] == 't' && ((command[0][4] == ' ' || command[0][4] == '\n') || !command[0][4])) {
	return "[̲̅$̲̅(̲̅ヽ̲̅༼̲̅ຈ̲̅ل͜ຈ̲̅༽̲̅ﾉ̲̅)̲̅$̲̅]"; 
      }
	
      
      else {
	int j = fork();
	if (j) {
	  wait();
	}
	else {
	  execvp(command[0],command);
	  return "( ͝° ͜ʖ͡°)つ";
	} 
      }
      free(command);
    }
    free(semicolon_parsed);
  }
}
