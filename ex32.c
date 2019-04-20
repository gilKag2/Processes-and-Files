//
// Gil Kagan
// 315233221
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>

#define ERROR "Error in system call\n"
#define ERROR_SIZE strlen(ERROR)
#define BUFF_SIZE 150
#define MAX_SIZE 450


char** readFile(char * path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        write(2, ERROR, ERROR_SIZE);
    char buffer[MAX_SIZE];
    bzero(buffer, BUFF_SIZE);
    if (read(fd, buffer, MAX_SIZE) < 0)
        write(2, ERROR, ERROR_SIZE);
    char ** lines;
    lines = (char**)malloc(3 * sizeof(char*));
    int j;
    for (j = 0; j < 3; j++)
        lines[j] = (char*) malloc(BUFF_SIZE * sizeof(char));
    int i = 0;
    lines[i] = strtok(buffer, "\n");
    while (lines[i] != NULL) {
        i++;
        lines[i] = strtok(NULL, "\n");
    }
    close(fd);
    return lines;
}

void searchInDir(char* path) {
    DIR* dir;
    struct dirent* pDirent;
    if ((dir = opendir(path)) == NULL)
        write(2, ERROR, ERROR_SIZE);

    while ((pDirent = readdir(dir)) != NULL) {
        if (strcmp(pDirent->d_name, ".") == 0 ||
            strcmp(pDirent->d_name, "..") == 0) {
            continue;
        }
        if (pDirent->d_type != DT_DIR) continue;
        struct dirent* userDirent;
        DIR * userName;
        if ((userName = opendir(pDirent->d_name)) == NULL)
            write(2, ERROR, ERROR_SIZE);
        while ((userDirent = readdir(userName)) != NULL){
            if (strcmp(userDirent->d_name, ".") == 0 ||
                strcmp(userDirent->d_name, "..") == 0)
                continue;

            if (strstr(userDirent->d_name, ".c") != NULL) {
                // compile this file.
            }

        }
        closedir(userName);
    }
    closedir(dir);
}

int getFd(char* path) {
    int fd;
    if ((fd = open(path, O_RDONLY)) < 0)
        write(2, ERROR, ERROR_SIZE);
    return fd;

}


int main(int argc, char** argv) {

    if (argc != 2)
        return 0;
    char path[BUFF_SIZE];
    strcpy(path, argv[1]);
    char ** lines = readFile(path);
    searchInDir(lines[0]);
    int inFd = getFd(lines[1]);
    int outfd = getFd(lines[2]);


    close(inFd);
    close(outfd);
    int i;
    for (i = 0; i < 3; i++)
        free(lines[i]);
    free(lines);
}
