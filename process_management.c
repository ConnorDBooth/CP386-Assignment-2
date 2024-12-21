/*
 -------------------------------------
 File:    process_management.c
 Project: Assignment_02
 file description
 -------------------------------------
 Author:  Connor Booth
 ID:      169038238
 Email:   boot8238@mylaurier.ca
 -------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#define SHM_NAME "process_management"
#define SHM_SIZE 4096

int shm_fd;
void *shm_ptr;

void write_output(char *command, char *output){
    FILE *fp;
    fp = fopen("output.txt", "a");
    fprintf(fp, "The output of: %s : is\n", command);
    fprintf(fp, ">>>>>>>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<<<<\n", output);
    fclose(fp);
}


void setup_shared_memory(){
    shm_fd = shm_open(SHM_NAME, O_CREAT| O_RDWR, 0666);
    if (shm_fd == -1){
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm_fd, SHM_SIZE) == -1){
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
}

void child_read_file(char *filename){

    FILE * file = fopen(filename, "r");
    if (file == NULL){
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[SHM_SIZE];
    int index = 0;
    while (fgets(buffer + index, sizeof(buffer) - index, file)){
        index += strlen(buffer + index);
    }

    strcpy((char*)shm_ptr, buffer);
    fclose(file);
    exit(0);
}

char **read_commands_from_mem(int *cmd_count){
    char *content = (char *)shm_ptr;
    char *line = strtok(content, "\n");
    char **commands = malloc(10 * sizeof(char*));

    *cmd_count = 0;

    while (line != NULL){
        commands[*cmd_count] = strdup(line);
        (*cmd_count)++;
        line = strtok(NULL, "\n");
    }

    return commands;
}


void execute_commands(char *command){
    char *args[10];
    int arg_count = 0;

    char *token = strtok(command, " ");
    while (token != NULL){                                                                                     
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    if(execvp(args[0], args) == -1){
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}

void execute_with_pipe(char *command){
    int pipe_fd[2];
    pipe(pipe_fd);

    if (fork() == 0){
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        char *args[10];
        int arg_count = 0;
        char *token = strtok(command, " ");
        while(token != NULL){
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        if (execvp(args[0], args) == - 1){
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        close(pipe_fd[1]);

        char output[1024];
        int nbytes = read(pipe_fd[0], output, sizeof(output));
        output[nbytes] = '\0';
        close(pipe_fd[0]);

        write_output(command, output);
    }
}

int main (int argc, char *argv[]){

    if (argc < 2){
        fprintf(stderr, "Usage: %s <command_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    setup_shared_memory();

    if (fork() == 0){
        child_read_file(argv[1]);
    } else {
        wait(NULL);

        int cmd_count = 0;
        char **commands = read_commands_from_mem(&cmd_count);

        for (int i = 0; i < cmd_count; i++){
            execute_with_pipe(commands[i]);
            free(commands[i]);
        }
        free(commands);

        munmap(shm_ptr, SHM_SIZE);
        shm_unlink(SHM_NAME);

        
    }

    return(EXIT_SUCCESS);
}
