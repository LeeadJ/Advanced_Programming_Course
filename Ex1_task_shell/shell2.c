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
    char if_command[1024];
    char then_command[1024];
    char next_command[1024];
    char cmd_cpy[1024];
    char last_command[1024] = "";
    char *cmds[NUM_STRINGS];
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status, append_redirect, numPipes, num_cmds;
    char *argv[NUM_STRINGS];
    char prompt[1024] = "hello";
    int if_status, fi_flag = 0;;

    // Register the signal handler
    signal(SIGINT, handle_sigint);

    while (1){
        if(fi_flag == 1){
            fi_flag = 0;
            if_command[0] = '\0';
            if(WEXITSTATUS(status) == 0){
                // if command is true:
                command = then_command;
                then_command[0] = '\0';
            }
            else{
                command = next_command;
                next_command[0] = '\0';
            }
            continue;
        }
        else{
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
        }
        
        command[strlen(command) - 1] = '\0';
        strcpy(cmd_cpy, command);
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
        // printf("Command: %s\n", command);

        //////////////////////////////////////////////////////////////
        if(strchr(command, '|') != NULL){
            // printf("Pipe Command\n");
            // Command is a pipe command:
            num_cmds = 0;
            for(i=0; i<NUM_STRINGS; i++){
                cmds[i] = NULL;
                cmds[i] = (char *)malloc(MAX_LENGTH * sizeof(char));
            }
            token = strtok(command, "|");
            while(token != NULL && num_cmds < NUM_STRINGS){
                while(*token == ' ') {
                    token++;
                }
                // printf("%s\n", token);
                strcpy(cmds[num_cmds], token);
                token = strtok(NULL, "|");
                num_cmds++;
            }
            numPipes = num_cmds -1;
            // printf("numPipes: %d\n", numPipes);
            // printf("num_cmds: %d\n", num_cmds);
        }///////////////////////////////////////////////////////////////

        else{
            // if command is a normal command:
            cmds[0] = (char *)malloc(MAX_LENGTH * sizeof(char));
            strcpy(cmds[0], command);
            num_cmds = 1;
            numPipes = 0;
        }

        int in=0;
        int pipe_fd[2*numPipes];
        pid_t pid;

        for(i = 0; i < (numPipes); i++){
            if(pipe(pipe_fd + i*2) < 0) {
                perror("couldn't pipe");
                exit(EXIT_FAILURE);
            }
        }

        int pipe_index=0;
        // Create pipes and fork processes
        for(int k=0; k<num_cmds; k++){
            char* cmd = cmds[k];

            // command is a normal command
            i = 0;
            token = strtok (cmd," ");
            for(i=0; i<NUM_STRINGS; i++){
                // argv[i] = NULL;
                argv[i] = malloc(MAX_LENGTH * sizeof(char));
                // printf("i=%d index=%s, \n", i,argv[i]);
            }

            // Copying the words of the command as arguments in argv[]:
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
        
        

            /* Is command empty */
            if (argv[0] == NULL){
                // printf("\n");
                continue;
            }

            /* Adding the "if" command */
            // printf("before: %s\n", cmd_cpy);
            if(cmd_cpy[0] == 'i' && cmd_cpy[1] == 'f'){
                
                
                memcpy(if_command, cmd_cpy + 3, strlen(cmd_cpy) -3);
                printf("if command:: %s\n", if_command);
                
                // check if next input is the word "then":
                char then[1024];
                fgets(then, 1024, stdin);
                then[strcspn(then, "\n")] = '\0';

                // printf("then: %s\n", then);
                if(strcmp(then, "then") == 0){
                    // printf("User entered then\n");

                    
                    fgets(then_command, 1024, stdin);
                    then_command[strcspn(then_command, "\n")] = '\0';
                    printf("then command: %s\n", then_command);

                    //save next command to check if its "else" of "fi".
                    
                    fgets(next_command, 1024, stdin);
                    next_command[strcspn(next_command, "\n")] = '\0';
                    printf("next command: %s\n", next_command);

                    if(strcmp(next_command, "fi") == 0){
                        printf("User entered fi\n");
                        // if the "if" statement is true, run the "then command":
                        memcpy(command, if_command, strlen(if_command));
                        fi_flag = 1;
                        continue;
                    }

                    else if(strcmp(next_command, "else")){
                        // if thhere is an else command:
                        printf("User entered else .\n");
                        char else_command[1024];
                        fgets(else_command, 1024, stdin);
                        else_command[strcspn(else_command, "\n")] = '\0';
                        char fi[1024];
                        fgets(fi, 1024, stdin);
                        fi[strcspn(fi, "\n")] = '\0';

                        if(strcmp(fi, "fi")){
                            printf("User entered fi\n");
                            // if the "if" statement is true, run the "then command":
                            printf("User entered fi\n");
                            // if the "if" statement is true, run the "then command":
                            memcpy(command, if_command, strlen(if_command));
                            fi_flag = 1;
                            continue;
                            }
                    }

                }
                
                else{
                    printf("--Error IN FLOW--\n");
                }
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
                //if not last command
                if(k + 1 != num_cmds){
                    if(dup2(pipe_fd[pipe_index + 1], 1) < 0){
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }

                //if not first command&& pipe_index!= 2*numPipes
                if(pipe_index != 0 ){
                    if(dup2(pipe_fd[pipe_index-2], 0) < 0){
                        perror(" dup2");
                        exit(EXIT_FAILURE);

                    }
                }
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

                for(int z = 0; z < 2*numPipes; z++){
                    close(pipe_fd[z]);
                }

                if (execvp(argv[0], argv) == -1) {
                    perror("Error executing command");
                    exit(EXIT_FAILURE);
                }
                
                
            }

            else if (pid < 0) {
                // Error
                perror("fork");
            } 

            pipe_index+=2;
        }

        for(i = 0; i < 2 * numPipes; i++){
            close(pipe_fd[i]);
        }

        for(i = 0; i < numPipes + 1; i++){
            wait(&status);
        }
         
    }
    free(command); 
}
