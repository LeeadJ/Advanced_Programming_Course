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


#define NUM_STRINGS 10
#define MAX_LENGTH 100


void handle_sigint(int sig){
    printf("\nYou typed Control-C!\n");
    fflush(stdout);
}

int main() {

    char *command = malloc(1024);
    char last_command[1024] = "";
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status, append_redirect;
    char *argv[NUM_STRINGS];
    char prompt[1024] = "hello";

    // Register the signal handler
    signal(SIGINT, handle_sigint);

    while (1){

        printf("%s: ", prompt);
        fflush(stdout);
        if (fgets(command, 1024, stdin) == NULL) {
            if(command[strlen(command)-1] != '\n'){
                printf("Command buffer is overflowed\n");
                exit(1);
            }
            // Handle EOF or error
            break;
        }
        command[strlen(command) - 1] = '\0';

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
        printf("Command: %s\n", command);
        if(strchr(command, '|') != NULL){
            printf("Pipe Command\n");
            // Command is a pipe command:
            int num_cmds = 0;
            char *cmds[NUM_STRINGS];
            for(i=0; i<NUM_STRINGS; i++){
                cmds[i] = NULL;
                cmds[i] = malloc(MAX_LENGTH * sizeof(char));
            }
            char *buff[1024];
            token = strtok(command, "|");
            while(token != NULL && num_cmds < NUM_STRINGS){
                while(*token == ' ') {
                    token++;
                }
                printf("%s\n", token);
                strcpy(cmds[num_cmds], token);
                token = strtok(NULL, "|");
                num_cmds++;
            }
            int numPipes = num_cmds -1;
            printf("numPipes: %d\n", numPipes);
            printf("num_cmds: %d\n", num_cmds);

            int in=0;
            int pipe_fd[2];
            pid_t pid;

            // Create pipes and fork processes
            for(i=0; i<num_cmds; i++){
                //create a new pipe:
                pipe(pipe_fd);

                //fork a new proccess:
                pid = fork();
                if(pid < 0){
                    perror("fork error");
                    continue;
                }
                else if(pid == 0){
                    // entering child proccess:
                    char* cmd = cmds[i];

                    //closing the read end of the prev pipe:
                    close(in);

                    // If this is not the last command, redirect the write end of this pipe to the input of the next command:
                    if(i < num_cmds-1){
                        dup2(pipe_fd[1], STDOUT_FILENO);
                    }

                    // If this is not the first command, redirect the read end of the previous pipe to the input of this command
                    if (i > 0) {
                        dup2(pipe_fd[0], STDIN_FILENO);
                    }

                    // Close both ends of this pipe
                    close(pipe_fd[0]);
                    close(pipe_fd[1]);

                    // Execute the command
                    char *args[10];
                    i = 0;
                    token = strtok(cmd, " ");
                    while (token != NULL)
                    {
                        args[i] = token;
                        token = strtok(NULL, " ");
                        i++;
                    }
                    args[i] = NULL;
                    if (execvp(args[0], args) == -1) {
                    perror("Error executing command");
                    exit(EXIT_FAILURE);
                }

                    exit(EXIT_SUCCESS);;
                }
                else{
                    // entering parent proccess:
                    // Close the write end of the previous pipe
                    close(pipe_fd[1]);

                    // Redirect standard error to standard output
                    dup2(STDOUT_FILENO, STDERR_FILENO);

                    // If this is not the first command, save the read end of this pipe as the input for the next command
                    if (i > 0) {
                        in = pipe_fd[0];
                    }
                }
            }
            // Wait for all child processes to finish
            while (wait(NULL) > 0);

            for(i=0; i<NUM_STRINGS; i++){
                free(cmds[i]);
            }
            
        }

        else{
            // command is a normal command
            i = 0;
            token = strtok (command," ");
            for(i=0; i<NUM_STRINGS; i++){
                // argv[i] = NULL;
                argv[i] = malloc(MAX_LENGTH * sizeof(char));
                // printf("i=%d index=%s, \n", i,argv[i]);
            }
            i = 0;
            while (token != NULL)
            {
                if(i<10){
                    strcpy(argv[i], token);
                    i++;
                } else {
                    printf("To many arguments in argv.\n");
                    break;
                }
                
                token = strtok (NULL, " ");
            }
            argv[i] = NULL;
            redirect = 0;
            append_redirect = 0;
            outfile = NULL;
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
        } else {
            amper = 0; 
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
        pid_t pid = fork();
        if (pid == 0) {
            // Entering child proccess: 
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
            if (execvp(argv[0], argv) == -1) {
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
            } else if (pid < 0) {
                // Error
                perror("fork");
            } else {
                // In parent process
                if (! amper) {
                    // Wait for child process to finish
                    waitpid(pid, &status, 0);
                }
            }
        }
        free(command);
    }
        //     else{
        //         //redirect = 1 or 3
        //         fd == creat(outfile, 0660);
        //         close(STDOUT_FILENO);
        //         dup(fd);
        //         close(fd);
        //     }

        //     int exitstatus = execvp(argv[0], argv);
        //     fprintf(stderr, "%s: command not found\n", argv[0]);
        //     exit(exitstatus);
        // }
        // else{
        //     // Entering main proccess:
        //     /* parent continues here */
        // if (amper == 0)
        //     retid = wait(&status);

        // // Redirect stdout back to the terminal
        //     fd = open("/dev/tty", O_WRONLY);
        //     close(STDOUT_FILENO);
        //     dup(fd);
        //     close(fd);
        // }


        // /* parent continues here */
        // if (amper == 0)
        //     retid = wait(&status);

        // // Redirect stdout back to the terminal
        //     fd = open("/dev/tty", O_WRONLY);
        //     close(STDOUT_FILENO);
        //     dup(fd);
        //     close(fd);
    // }
// }