#include "parse.h"

int count_tokens(char *line, char delim) {
  char *p = line;
  int count = 0;
  while (*p) {
    if (*p == delim) {
      count++;
    }
    p++;
  }
  return count;
}
char **parse_args(char *command) {
  int num_args = count_tokens(command,' ');
    char **args_array = (char**)malloc(sizeof(char *)*num_args);
    int j = 0;
    while (j < 32) {
      args_array[j] = (char *)malloc(sizeof(char)*32);
      j++;
    }

    args_array[0] = strtok(command," ");
    int i = 1;
    while (i <= num_args) {
      args_array[i] = strtok(NULL," ");
      i++;
    }
    args_array[i] = NULL;
    return args_array;
 }

char** parse_commands(char input[256]) {
  char *commands = input;
  int num_commands = count_tokens(commands,';');
  char **comm_array = (char**)malloc(sizeof(char *)*num_commands);
  int j = 0;
  while (j < 32) {
    comm_array[j] = (char *)malloc(sizeof(char)*32);
    j++;
  }

  comm_array[0] = strtok(input, ";");
  int k = 1;
  while (k < num_commands){
    comm_array[k] = strtok(NULL, ";");
    if (k > num_commands)
      return 0;
    k++;
  }
  comm_array[k] = NULL;
  return comm_array;
}

