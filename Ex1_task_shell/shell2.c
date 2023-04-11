#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <fcntl.h>

#define MAX_PROMPT_LENGTH 100
int main() {
char command[1024];
char *token;
char *outfile;
int i, fd, amper, redirect, retid, status;
char *argv[10];
char prompt[MAX_PROMPT_LENGTH] = "mypromp";

while (1)
{
    printf("hello: ");
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';

    /* parse command line */
    i = 0;
    token = strtok (command," ");
    while (token != NULL)
    {
        argv[i] = token;
        token = strtok (NULL, " ");
        i++;
    }
    argv[i] = NULL;

    /* Is command empty */
    if (argv[0] == NULL)
        continue;

    /* Does command line end with & */ 
    if (! strcmp(argv[i - 1], "&")) {
        amper = 1;
        argv[i - 1] = NULL;
    }
    else 
        amper = 0; 

    if (! strcmp(argv[i - 2], ">")) {
        redirect = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
        }

    /* 1) Does command line contain stderr redirection "2>" ? */
    if (i > 2 && ! strcmp(argv[i - 2], "2>") && argv[i - 1]) {
        redirect = 1;
        outfile = argv[i - 1];
        argv[i - 2] = NULL;
        argv[i - 1] = NULL;
    }
    /* Does command line contain ">>" ? */
    if (! strcmp(argv[i - 2], ">>")) {
        redirect = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
        fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0660);
        if(fd < 0){
            perror("open");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
        }

    if(!strcmp(argv[0], "echo")){
        for(int index=1; index<i; index++){
            printf("%s ", argv[index]);
        }
        printf("\n");
    }
    else 
        redirect = 0; 

    /* for commands not part of the shell command language */ 

    if (fork() == 0) { 
        /* redirection of IO ? */
        if (redirect) {
            fd = creat(outfile, 0660); 
            if (fd < 0) {
                    fprintf(stderr, "Failed to create output file '%s': %s\n",
                            outfile, strerror(errno));
                    exit(1);
                }
            close (STDOUT_FILENO) ; 
            dup(fd); 
            dup2(fd, STDERR_FILENO); 
            close(fd); 
            /* stdout is now redirected */
        } 
        execvp(argv[0], argv);
        fprintf(stderr, "Failed to execute command '%s': %s\n",
                    argv[0], strerror(errno));
            exit(1);
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);
}
}
