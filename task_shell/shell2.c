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

void handler(int sig)
{
    printf("\nYou typed Control-C!\n");
    fflush(stdout);
}

int main() {
    signal(SIGINT, handler);
    char command[1024];
    char last_command[1024] = "";
    char *token;
    char *outfile, *error_file;
    int i, fd, amper, redirect, retid, status, arg_counter, append_redirect, isPiping;
    char *argv[10];
    char prompt[1024] = "hello";

    

    while (1)
    {
        printf("%s: ", prompt);
        fflush(stdout);
        if(fgets(command, 1024, stdin) == NULL){
            // Handle EOF or error
            break;
        }
        command[strlen(command) - 1] = '\0';

        /* Adding quit command */
        if (!strcmp(command, "quit")) 
            break;

        /* Adding "!!" command */
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


        /* Adding Prompt Command*/
        if(!strcmp(argv[0], "prompt")){
            if(arg_counter > 1 && !strcmp(argv[1], "=")){
                if(argv[2] == NULL){
                    perror("--PROMPT ERROR (MISSING 2ND ARGUMENT)--\n");
                }
                else{
                    strncpy(prompt, argv[2], 1024);
                }
            }
            continue;
        }

        /* Adding echo command*/
        if(!strcmp(argv[0], "echo")){
            if(argv[1] == NULL){
                perror("--ECHO ERROR (MISSING 2ND ARGUMENT)--\n");
                continue;
            }
            /* Addding $? command*/
            if(!strcmp(argv[1], "$?")){
                printf("%d\n", WEXITSTATUS(status));
                continue;
            }

            for(int j=1; j<i; j++){
                if(argv[j][0] == '$'){
                    char *var = argv[j] + 1;
                    char *str = getenv(var);
                    printf("%s ", str);
                }
                else
                    printf("%s ", argv[j]);
            }
            printf("\n");
            continue;
        }

        /* Addign variables to the shell  */
        if(i == 3 && argv[0][0] == '$' && !strcmp(argv[1], "=")){
            char *var = argv[0] + 1;
            setenv(var, argv[2], 1);
            continue;
        }



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

        /* Adding cd command*/
        if(!strcmp(argv[0], "cd")){
            if(argv[1] == NULL){
                perror("--CD ERROR (MISSING 2ND ARGUMENT)--\n");
                continue;
            }
            else if (chdir(argv[1]) != 0) {
                fprintf(stderr, "cd: %s: --ERROR (No such file or directory)--\n", argv[1]);
            }
            continue;
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
                    WEXITSTATUS(status);
                }

            }
        }
            
    }
}
