#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>

void handle_sigint(int sig){
    printf("\nYou typed Control-C!\n");
    fflush(stdout);
}

int main() {
char command[1024];
char last_command[1024] = "";
char *token;
char *outfile;
int i, fd, amper, redirect, retid, status, append_redirect;
char *argv[10];
char prompt[1024] = "hello";

// Register the signal handler
signal(SIGINT, handle_sigint);

while (1)
{
    printf("%s: ", prompt);
    fflush(stdout);
    if (fgets(command, 1024, stdin) == NULL) {
        // Handle EOF or error
        break;
    }
    command[strlen(command) - 1] = '\0';
    // printf("Command=%s: ", command);
    // printf("last_command=%s: \n", last_command);

    /* Check if user wants to quit */
    if (!strcmp(command, "quit")) 
        break;

    /* Check if last command needs to be executed */
    if (!strcmp(command, "!!")) {
        if (strlen(last_command) == 0) {
            continue;
        }
        strcpy(command, last_command);
    } else {
        /* Save current command */
        strcpy(last_command, command);
    }

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
    if (argv[0] == NULL){
        // printf("\n");
        continue;
    }
        

    /* Does command line end with & */ 
    if (! strcmp(argv[i - 1], "&")) {
        amper = 1;
        argv[i - 1] = NULL;
        // printf("\n");
    }
    else 
        amper = 0; 

    /* Does command line contain ">" */ 
    if (! strcmp(argv[i - 2], ">")) {
        redirect = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
        // printf("\n");
        }

    /* Does command line contain ">>" */ 
    else if(! strcmp(argv[i - 2], ">>")){
        append_redirect = 1;
        argv[i-2] = NULL;
        outfile = argv[i-1];
        // printf("\n");
    }

    /* Does command line contain "2>" */ 
    else if(!strcmp(argv[i-2], "2>")){
        redirect = 1;
        argv[i-2] = NULL;
        outfile = argv[i-1];
        fd = creat(outfile, 0660);
        close(STDERR_FILENO);
        dup(fd);
        close(fd);
        // printf("\n");
    }
    else 
        redirect = append_redirect = 0; 

    /* Command to change the prompt*/
    if(i == 3 && !strcmp(argv[0], "prompt") && !strcmp(argv[1], "=")){
        strncpy(prompt, argv[2], 1024);
        continue;
    }

    /* Command to echo status */
    if (!strcmp(argv[0], "echo") && !strcmp(argv[1], "$?")) {
        printf("%d\n", status);
        continue;
    }

    /* Command to echo arguments*/
    if(!strcmp(argv[0], "echo")){
        for(int j=1; j<i; j++){
            printf("%s ", argv[j]);
        }
        printf("\n");
        continue;
    }

    

    /* Command to change directory */
    if (!strcmp(argv[0], "cd")) {
        if (i < 2) {
            fprintf(stderr, "cd: missing operand\n");
        }
        else if (chdir(argv[1]) != 0) {
            fprintf(stderr, "cd: %s: No such file or directory\n", argv[1]);
        }
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



/*
task 4


*/