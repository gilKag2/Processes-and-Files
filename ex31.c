/*
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define ERROR "Error in system call\n"
#define ERROR_SIZE  strlen(ERROR)
#define BUFF_SIZE 150




int main(int argc, char **argv) {
    if (argc != 3)
        return 0;
    int fd1, fd2;
    if ((fd1 = open(argv[1], O_RDONLY)) < 0 || (fd2 = open(argv[2], O_RDONLY)) < 0)
        write(2, ERROR, ERROR_SIZE);

    char buff1[BUFF_SIZE];
    char buff2[BUFF_SIZE];

    int similarFlag = 0;
    int numOfBytes1, numOfBytes2 = 0;
    while (1) {
        bzero(buff1, BUFF_SIZE);
        bzero(buff2, BUFF_SIZE);
        numOfBytes1 = read(fd1, buff1, BUFF_SIZE - 1);
        numOfBytes2 = read(fd2, buff2, BUFF_SIZE - 1);

        if (numOfBytes1 < 0 || numOfBytes2 < 0) {
            write(2, ERROR,ERROR_SIZE);
            break;
        }
        // quick check to see if the files sizes are different
        if (numOfBytes1 != numOfBytes2) return 2;

        // eof check.
        if (numOfBytes1 == 0) break;

        int i;
        for ( i = 0; i < numOfBytes1; i++) {
            if (buff1[i] != buff2[i]) {
                int diff = buff1[i] - buff2[i];
                // check if the difference of the ascii values is 32, which means one of the letters
                // is the capital of the other one.
                if (diff == 32 || diff == -32) similarFlag = 1;
                    // the letters are totally different.
                else  return 2;

            }

        }
    }
    close(fd1);
    close(fd2);
    if (similarFlag) return 3;
    else return 1;
}
*/
