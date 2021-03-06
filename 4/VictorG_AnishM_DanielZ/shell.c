#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
//more includes
void run(char * input);

void myexec(char *input){
  int i = 0;
  char *singlearg;
  char * args[10];
  while ((singlearg = strsep(&input, " ") )) {
    args[i] = singlearg;
    i++;
  }
  args[i] = 0;
  int f = fork();
  if ( !f ) {
    execvp(args[0], args);
    exit(0);
  }
  else {
    wait( &f );
      f = 0;
  }
}

void piping(char * args[]){
  int check1,check2,check3;
  int i = fork();
  if (!i){
    check1 = open("check.txt", O_CREAT | O_WRONLY | O_TRUNC, 0777);
    dup2(check1,STDOUT_FILENO);
    execlp(args[0],args[0],NULL);
  }
  else{
    check2 = wait(&check3);
    check1 = open("check.txt",O_RDONLY);
    dup2(check1,STDIN_FILENO);
    execlp(args[2],args[2],NULL);
  }
}

void semisep(char *s){
  int i=0;
  char * raw;
  if (strstr(s,"\n")){
    raw=strsep(&s,"\n");
  }
  else{
    raw=s;
  }
  char * singlecommand;
  while ((singlecommand=strsep(&raw,";") )) {
    int f=fork();
    if (!f)
      run(singlecommand);
    wait(NULL);
  }
}



char ** separate(char *s){
  char *pos;
  int i=0;
  if ((pos=strchr(s, '\n')) != NULL)
    *pos = '\0';
  char *s1=s;
  char **s2=(char**)(malloc(strlen(s)*strlen(s)*sizeof(char)+sizeof(char)));

  for (i=0;s[i];i++){
    s2[i] = (char *)(malloc(strlen(s)*sizeof(char)));
    s2[i] = strsep(&s1, " ");
  }

  s2[i]=NULL;
  return s2;
}
void redirout(char** args, char * dest) {
  int fd;
  fd = open(dest, O_WRONLY | O_TRUNC | O_CREAT, 0666);
  dup2(fd,STDOUT_FILENO);
  execvp(args[0], args);
  return;
}
void redirin(char** args, char * source){
  int fd;
  fd = open(source, O_RDONLY);
  dup2(fd,STDIN_FILENO);
  execvp(args[0], args);
  return;
}

void redirpipe(char** args, char ** args2) {
    int pfds[2];

    pipe(pfds);
    int i =0;
    /*
    while (args[i]){
      if (!strcmp(args[i]," ")){
        args[i]=args[i+1];
      }
      i++;
    }
    i =0;
    while (args2[i]){
      if (!strcmp(args[i]," ")){
        args2[i]=args[i+1];
      }
      i++;
    }*/



    if (!fork()) {
        //close(1);       /* close normal stdout */
        dup2(STDOUT_FILENO,pfds[1]);   /* make stdout same as pfds[1] */
        close(pfds[0]); /* we don't need this */
        //printf("%s\n",args[0]);
        execvp(args[0], args);
        //execlp("ls","ls", NULL);
    } else {
        wait(NULL);
        //close(0);       /* close normal stdin */
        int backup = dup(STDIN_FILENO);
        dup2(pfds[0],STDIN_FILENO);   /* make stdin same as pfds[0] */
        close(pfds[1]); /* we don't need this */
        dup2 (backup, STDIN_FILENO);

        execvp(args2[0], args2);
    }
}

void run(char *input){
  if (!strcmp(input,"\n")){//so you can hit enter with no commands
    return;
  }
  else if (strchr(input, ';')){
    semisep(input);
    return;
  }
  else if (strstr(input, "<")){
    char *input2=input;
    char ** args;
    args=separate(strsep(&input2,"<"));
    char * raw;
    if (strstr(input2,"\n")){
      raw=strsep(&input2,"\n");
    }
    else{
      raw=input2;
    }
    int f=fork();
    if (!f)
      redirin(args,raw);
    return;
  }
  else if (strstr(input, ">")){
    char *input2=input;
    char ** args;
    args=separate(strsep(&input2,">"));
    char * raw;
    if (strstr(input2,"\n")){
      raw=strsep(&input2,"\n");
    }
    else{
      raw=input2;
    }
    int f=fork();
    if (!f)
      redirout(args,raw);
    return;
  }
  else if (strstr(input, "|")){
    char * input2=input;
    char * raw;
    if (strstr(input2,"\n")){
      raw=strsep(&input2,"\n");
    }
    else{
      raw=input2;
    }
    char ** args=(char **) malloc(2048);
    char ** args2=(char **) malloc(2048);
    args=separate(strsep(&raw,"|"));
    //printf("%s\n%s\n",args[0],args[2]);
    args2=separate(raw);
    //int f=fork();
    //if(!f)
      redirpipe(args,args2);
    return;
  }
  char **inputA=malloc(strlen(input)*strlen(input)*sizeof(char)+sizeof(char));
  inputA=separate(input);
  if (!strcmp(inputA[0],"cd")){
    if(inputA[1]){
      if (inputA[1][0]=='~'){
	//printf("here\n");
      	char a;
      	char * b=(char *) malloc(sizeof(char)*strlen(inputA[1]));
      	strcpy(b,inputA[1]);
      	a=b[1];
      	//printf("This is b: %s\n",b);
      	int c=1;
      	inputA[1][0]=b[c];
	for (c=2;!inputA[1][c];c++){
      	  inputA[1][c]=b[c];
      	  //printf("%c\n",b[c]);
        }
      	//printf("%s\n",inputA[1]);
      	char *env;
      	strcpy(env,getenv("HOME"));
      	chdir(strcat(env,inputA[1]));
      	//free(env);
      	free(b);
      	free(inputA);
      	return;
      }
      //printf("hey\n");
      int i = chdir(inputA[1]);
      if (i!=0){
	printf("%s is not a directory.",inputA[0]);
	//free(inputA);
	return;
      }
    }
    else{
      printf("%s\n",getenv("HOME"));
      chdir(getenv("HOME"));
      free(inputA);
      return;
    }
  }
    if (!strcmp(inputA[0],"exit")){
      int z=fork();
      if (z==0){
	    printf("Thanks for using our shell!\n");}
      //free(inputA);
      exit(0);
      return;
    }
    //printf("heyhey\n");
    else{
      int f=fork();
      if (!f)
	execvp(inputA[0],inputA);
      free(inputA);
      return;
    }
}




int main(){
  char s[1024];
  int f=1;

  while(1){
    printf("==|==<(x_x)>===>: ");
    fgets(s,1024,stdin);
    //f=fork();
    run(s);
    //return 0;
    wait(NULL);
  }

  //char damn[256]="ls;echo hello\n";
  //semisep(damn);
  return 0;
}
