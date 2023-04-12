#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

int main() {
char command[1024];
char *token;
char *outfile;
int i, fd, amper, redirect, retid, status, append_redirect;
char *argv[10];
char prompt[1024] = "hello";

while (1)
{
    printf("%s: ", prompt);
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

    else if(! strcmp(argv[i - 2], ">>")){
        append_redirect = 1;
        argv[i-2] = NULL;
        outfile = argv[i-1];
        // fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0660);
        // close(STDOUT_FILENO);
        // dup(fd);
        // close(STDERR_FILENO);
        // dup(fd);
        // close(fd);
    }

    else if(!strcmp(argv[i-2], "2>")){
        redirect = 1;
        argv[i-2] = NULL;
        outfile = argv[i-1];
        fd = creat(outfile, 0660);
        close(STDERR_FILENO);
        dup(fd);
        close(fd);
    }
    else 
        redirect = append_redirect = 0; 

    /* Command to change the prompt*/
    if(i == 3 && !strcmp(argv[0], "prompt") && !strcmp(argv[1], "=")){
        strncpy(prompt, argv[2], 1024);
        continue;
    }

    /* for commands not part of the shell command language */ 

    if (fork() == 0) { 
        /* redirection of IO ? */
        if (redirect) {
            fd = creat(outfile, 0660); 
            close (STDOUT_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */
        } 
        else if(append_redirect){
        fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0660);
        close(STDERR_FILENO);
        dup(fd);
        close(fd);
        /* stdout is now redirected to file in append mode */
        }

        execvp(argv[0], argv);
        fprintf(stderr, "%s: command not found\n", argv[0]);
        exit(127);
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);

    // Redirect stdout back to the terminal
        fd = open("/dev/tty", O_WRONLY);
        close(STDOUT_FILENO);
        dup(fd);
        close(fd);
}
}