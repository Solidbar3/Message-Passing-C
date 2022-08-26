#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

#define NAME "OS"
#define NUM 1024
#define SIZE (NUM*sizeof(int))
int main(int argc, char *argv[])
{
        int n = 0;
        if(argc > 1) {
                n = atoi(argv[1]);
        }

        if(n <= 0) {
                printf("input is not positive integer\n");
                return 0;
        }
        pid_t pid;

        pid = fork();
        if (pid == 0) { //Child Process
                int fd = shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
                if (fd < 0) {
                        perror("shm_open()");
                        return EXIT_FAILURE;
                }

                ftruncate(fd, SIZE);

                int *data = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

                int i = 1; // Starts a 1 because sequence length is kept at 0.
                int count = 0;
                while(n != 0) {
                        data[i] = n;
                        if( n % 2 == 0) {
                                n = n / 2;
                        }
                        else if( n == 1) {
                                n = 0;
                        }
                        else {
                                n = (3*n) + 1;
                        }
                        i += 1;
                        count += 1;
                }
                data[0] = count;// Keeps track of sequence length in shared memory

                munmap(data, SIZE);
                
                close(fd);

                return EXIT_SUCCESS;
        }
        else { //Parent Process
                wait(NULL);

                int fd = shm_open(NAME, O_RDONLY, 0666);
                if(fd < 0) {
                        perror("shm_open()");
                        return EXIT_FAILURE;
                }
                int *data = (int *)mmap(0, SIZE, PROT_READ, MAP_SHARED, fd, 0);

                for(int i = 1; i < data[0]+1; ++i) { // Prints the length of the sequence
                        printf("%d ", data[i]);
                }
                printf("\n");

                munmap(data, SIZE);

                close(fd);

                shm_unlink(NAME);

                return EXIT_SUCCESS;
        }
        return 0;
}
