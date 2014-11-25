#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "parse.h"

int parse(char **in, command_t *cmds[], int num_cmds)
{
  int count;
  for (count = 0; count < num_cmds; count++)
    {
      char *delim = (char *)malloc(1 * sizeof(char));

      cmds[count] = (command_t *)malloc(1 * sizeof(command_t));
      cmds[count]->execstr = strsep_save(in, DELIMS, delim);

      free(delim);
    }
}

char *strsep_save(char **stringp, char *delims, char *found)
{
  char *string = *stringp;
  int i_string;
  for (i_string = 0; string[i_string]; i_string++)
    {
      int i_del;
      for (i_del = 0; delims[i_del]; i_del++)
      	{
      	  if (string[i_string] == delims[i_del])
      	    {
      	      char *tok = string;
      	      *found = delims[i_del];
      	      string[i_string++] = 0;
      	      *stringp = &string[i_string];
      	      return tok;
      	    }
      	}
    }
  return string;
}

int count_delims(char *in, char *delims)
{
  int i_in, i_del, count = 0;
  for (i_in = 0; in[i_in]; i_in++)
    {
      for (i_del = 0; delims[i_del]; i_del++)
	{
	  if (in[i_in] == delims[i_del])
	    {
	      count++;
	      break;
	    }
	}
    }
  return count;
}
