#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "directorystack.h"

#define TOK_INIT_SIZE 512
#define FILE_SIZE 256

#define FG_PROCESS 0
#define BG_PROCESS 1

typedef struct process {
    char *cmd;
    int pid;
    int index;
} process;

static const char *shell_name = "StD";

node * parse_input(char *, node *);
node * execute(char **, int, node *);
node * change_directory(char *, node *);

void cleanup_argv();
void setup_argv();
void add_null_argv();

void setup_shell();
void cleanup_shell();

static int global_stdin_backup = STDIN_FILENO;
static int global_stdout_backup = STDOUT_FILENO;
