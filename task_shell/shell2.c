#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>
#include <sys/wait.h>



int main() {
    char command[1024];
    char *token;
    char *outfile, *error_file;
    int i, fd, amper, redirect, retid, status, arg_counter, append_redirect, prev_status, isPiping;
    char *argv[10];
    char prompt[1024] = "hello";

    

    while (1)
    {
        printf("%s: ", prompt);
        fflush(stdout);
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
        arg_counter = i;

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

        /* Does command line contain ">" or ">>" */ 
        if(i > 1){
            if (! strcmp(argv[i - 2], ">") || (! strcmp(argv[i - 2], ">>"))) {
                redirect = 1;
                if((! strcmp(argv[i - 2], ">>"))){
                    append_redirect =1 ;
                }
                outfile = argv[i - 1];
                argv[i - 2] = NULL;
                argv[i-1] = NULL;
            }
            /* Does command line contain "2>" */ 
            else if(!strcmp(argv[i-2], "2>")){
                redirect = 3;
                argv[i-2] = NULL;
                outfile = argv[i-1];
                fd = creat(outfile, 0660);
                close(STDERR_FILENO);
                dup(fd);
                close(fd);
                // printf("\n");
            } else {
                redirect = append_redirect = 0; 
            }
        }

        /* for commands not part of the shell command language */ 
        pid_t pid = fork();
        if (pid == 0) { 
            /* redirection of IO ? */
            if(redirect){
                if(append_redirect){
                    fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND | O_RDONLY, 0644);
                } else{
                    fd = open(outfile, O_CREAT | O_TRUNC | O_WRONLY | O_RDONLY, 0644);
                }
                
                // close(STDOUT_FILENO);
                dup2(fd, STDOUT_FILENO);
                close(fd);

                
            }
            isPiping = 0;
            if(isPiping){
                printf("Piping");
            }
            else {
                if (execvp(argv[0], argv) == -1) {
                    perror("Error executing command");
                    exit(EXIT_FAILURE);
                }
            }
                
        } 
            
        else if (pid < 0) {
            // Error
            perror("--ERROR IN FORK ID (NEGATIVE ID)--");
        }
            
        else { // pid > 0
            // In parent process
            if (amper == 0){
                retid = wait(&status);
                if(retid < 0){
                    perror("--RETURN ID ERROR (NEGATICE ID)--");
                } else {
                    prev_status = WEXITSTATUS(status);
                }

            }
        }
            
    }
}
