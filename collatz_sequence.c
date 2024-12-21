/*
 -------------------------------------
 File:    collatz_squence.c
 Project: Assignment_02
 file description
 -------------------------------------
 Author:  Connor Booth
 ID:      169038238
 Email:   boot8238@mylaurier.ca
 -------------------------------------
*/

#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h> 

#define SHM_NAME "Assignment2"
#define SHM_SIZE 4096

int main(int argc, char *argv[]){

    int shm_fd;
    void *shm_ptr; 
    int numberArray[3];
    int i = 0;
    // Open start_numbers and add contents to array
    FILE *start_numbers = fopen(argv[1],"r");
    if (start_numbers == NULL){
        perror("Error opening start_numbers.txt");
        exit(EXIT_FAILURE);
    }

    while(fscanf(start_numbers, "%d", &numberArray[i]) == 1){
        i++;        
    }

    fclose(start_numbers);

    for (int j = 0; j < 3; j++){
        int pid = fork();

        if (pid < 0){
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0){
            //Child Function
            shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
            if (shm_fd == -1){
                perror("shm_open (child)");
                exit(EXIT_FAILURE);
            }

            shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            if (shm_ptr == MAP_FAILED){
                perror("mmap (child)");
                exit(EXIT_FAILURE);
            }

            printf("Child Process: The generated collatz squence is:  %s", (char *)shm_ptr);

            if (j == i - 1){
                shm_unlink(SHM_NAME);
                munmap(shm_ptr, SHM_SIZE);
                close(shm_fd);
        
            }
        
            exit(EXIT_SUCCESS);
        } else {
        // Parent Function
            char buffer[256];
            int index = 0;
            int num = numberArray[j];
            printf("Parent Process: The positive integer read from file is %d\n", num);

            while (num != 1){
                index += sprintf(&buffer[index], "%d, ",num);
                if (num % 2 == 0){
                    num = num/2;
                } else {
                    num = 3 * num + 1; 
                }
            }

            index += sprintf(&buffer[index], "1\n");
        
            // Establish Shared-Memory Object 
            shm_fd =  shm_open( SHM_NAME, O_CREAT | O_RDWR, 0666);
            if (shm_fd == -1){
                perror("shm_open");
                exit(EXIT_FAILURE);
            }

            if (ftruncate(shm_fd, SHM_SIZE) == - 1){
                perror("ftruncate");
                exit(EXIT_FAILURE);
            }

            shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            if (shm_ptr == MAP_FAILED){
                perror("mmap");
                exit(EXIT_FAILURE);
            }
            // Copy start_number contents to shared memory
            strcpy((char *)shm_ptr, buffer);
            // wait for children
            wait(NULL);
            // Unmap and close shared memory 
        }
    }
    return 0;
}
