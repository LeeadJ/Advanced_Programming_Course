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
        if(strchr(command, '|') != NULL){
            //command is a pipe command
            i = 0;
            token = strtok(command, "| ");
            while(token != NULL){
                argv[i] = token;
                token = strtok(NULL, "| ");
                i++;
            }
            argv[i] = NULL;

            /* Does command line contain "|" */
            int pipe_present = 0;
            for (int j = 0; argv[j] != NULL; j++) {
                if (!strcmp(argv[j], "|")) {
                    pipe_present = 1;
                    argv[j] = NULL;
                    break;
                }
            }
            
            if (pipe_present) {
                int fd[2];
                pid_t pid1, pid2;

                if (pipe(fd) == -1) {
                    perror("pipe");
                    exit(1);
                }

                pid1 = fork();
                if (pid1 == -1) {
                    perror("fork");
                    exit(1);
                }
                if (pid1 == 0) {
                    /* child 1 */
                    close(fd[0]);
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[1]);

                    /* execute first command */
                    execvp(argv[0], argv);
                    perror("execvp");
                    exit(1);
                }

                pid2 = fork();
                if (pid2 == -1) {
                    perror("fork");
                    exit(1);
                }
                if (pid2 == 0) {
                    /* child 2 */
                    close(fd[1]);
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);

                    /* execute second command */
                    execvp(argv[i + 1], argv + i + 1);
                    perror("execvp");
                    exit(1);
                }

                /* parent */
                close(fd[0]);
                close(fd[1]);

                /* wait for child processes to finish */
                waitpid(pid1, &status, 0);
                waitpid(pid2, &status, 0);
                continue;
            }

        }
        else{
            // command is a normal command
            i = 0;
            token = strtok (command," ");
            while (token != NULL)
            {
                argv[i] = token;
                token = strtok (NULL, " ");
                i++;
            }
            argv[i] = NULL;
        }
        

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
            redirect = 2;
            argv[i-2] = NULL;
            outfile = argv[i-1];
            // printf("\n");
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
            printf("%d\n", WEXITSTATUS(status));
            continue;
        }

        /* Command to echo arguments*/
        if(!strcmp(argv[0], "echo")){
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

    
        if(i == 3 && argv[0][0] == '$' && !strcmp(argv[1], "=")){
            char *var = argv[0] + 1;
            setenv(var, argv[2], 1);
            continue;
        }

        if(!strcmp(argv[0], "read")){
            char* str = getenv(argv[1]);
            printf("%s \n", str);
            continue;
        }







        /* for commands not part of the shell command language */ 

        if (fork() == 0) { 
            /* redirection of IO ? */
            if(redirect==2){
                fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND | O_RDONLY, 0660);
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
            }
            else{
                //redirect = 1 or 3
                fd == creat(outfile, 0660);
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
            }

            int exitstatus = execvp(argv[0], argv);
            fprintf(stderr, "%s: command not found\n", argv[0]);
            exit(exitstatus);
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