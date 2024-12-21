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

void setup_shared_memory();
void write_output(char *command, char *output);
int shm_fd;
void *shm_ptr;

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
    if (shm_ptr = MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
}

void write_output(char *command, char *output){
    FILE *fp;
    fp = fopen("output.txt", "a");
    fprintf(fp, "The output of: %s : is\n", command);
    fprintf(fp, ">>>>>>>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<<<<\n", output);
    fclose(fp);
}



