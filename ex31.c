/*
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define ERROR "Error in system call\n"
#define ERROR_SIZE  strlen(ERROR)
#define BUFF_SIZE 1


int main(int argc, char **argv) {
    if (argc != 3)
        return 0;
    int fd1, fd2;
    if ((fd1 = open(argv[1], O_RDONLY)) < 0 || (fd2 = open(argv[2], O_RDONLY)) < 0)
        write(2, ERROR, ERROR_SIZE);

   char buff1[BUFF_SIZE], buff2[BUFF_SIZE];
    int res = 1;
    int skip1 = 0, skip2 = 0;
    int numOfBytes1 = 0, numOfBytes2 = 0;
    while (1) {
        if (!skip1){
            bzero(buff1, BUFF_SIZE);
            numOfBytes1 = read(fd1, buff1, BUFF_SIZE);
        }
        if (!skip2){
            bzero(buff2, BUFF_SIZE);
            numOfBytes2 = read(fd2, buff2, BUFF_SIZE);
        }

        if (numOfBytes1 < 0 || numOfBytes2 < 0) {
            write(2, ERROR, ERROR_SIZE);
            break;
        }
        // check for eof.
        if (!numOfBytes1) {
            // if there is still data in the other file, it means they are totally different.
            if (numOfBytes2){
                res = 2;
            }
            break;
        }
        if (!numOfBytes2){
            res = 2;
            break;
        }
        if (skip1){
            if (*buff2 == '\n' || *buff2 == ' ') continue;

        } else if (skip2){
            if (*buff1 == '\n' || *buff1 == ' ') continue;
        }
        if (*buff1 != *buff2){
            if (*buff1 == '\n' || *buff1 == ' ') {
                res = 3;
                skip2 = 1;
                continue;
            }
            else if (*buff2 == '\n' || *buff2 == ' '){
                skip1 = 1;
                res = 3;
                continue;
            }
            else {
                int diff = *buff1 - *buff2;
                // check if the difference of the ascii values is 32, which means one of the letters
                // is the capital of the other one.
                if (diff == 32 || diff == -32) res = 3;
                    // the chars are totally different.
                else {
                    res = 2;
                    break;
                }
            }
        }
        skip1 = 0;
        skip2 = 0;
    }
    close(fd1);
    close(fd2);

    return res;
}
*/
