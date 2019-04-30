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
#include <wait.h>

#define ERROR "Error in system call\n"
#define ERROR_SIZE strlen(ERROR)
#define BUFF_SIZE 150
#define MAX_SIZE 450
#define MAX_TIME 5

#define OUTPUT_FILE "output.txt"
#define COMPILE_NAME "student.out"
#define COMP_FILE "./comp.out"
#define COMPILE_ERROR -1
#define TIMEOUT -2
#define BAD 2
#define SIMILAR 3
#define GREAT 4
#define SYSCALL_ERROR -3
#define NO_FILE_GRADE "0"
#define COMPILE_ERROR_GRADE "20"
#define TIMEOUT_GRADE "40"
#define BAD_OUTPUT_GRADE "60"
#define SIMILAR_OUTPUT_GRADE "80"
#define GREAT_JOB_GRADE "100"
#define NO_FILE_DESCRIPTION "NO_C_FILE"
#define COMPILE_ERROR_DESCRIPTION "COMPILATION_ERROR"
#define TIMEOUT_ERROR_DESCRIPTION "TIMEOUT"
#define BAD_OUTPUT_DESCRIPTION "BAD_OUTPUT"
#define SIMILAR_OUTPUT_DESCRIPTION "SIMILAR_OUTPUT"
#define GREAT_JOB_DESCRIPTION "GREAT_JOB"
#define RESULT_FILENAME "results.csv"


void error(){
    write(2, ERROR, ERROR_SIZE);
}

typedef struct Student{
    char name[BUFF_SIZE];
    char description[BUFF_SIZE];
    char grade[BUFF_SIZE];
} student;



int openFileForRead(char* path) {
    int fd;
    if ((fd = open(path, O_RDONLY)) < 0) {
        error();
        return -1;
    }
    return fd;

}
void setPath(char* path, char* file){
    strcat(path, "/");
    strcat(path, file);
}
int cmpOutput(char* dir, char * correctOutput){
    char studOutput[BUFF_SIZE];
    strcpy(studOutput, dir);
    setPath(studOutput, OUTPUT_FILE);
    pid_t  pid;
    int status;
    char  cwd[BUFF_SIZE];
    if (getcwd(cwd, sizeof(cwd)) == NULL){
        return SYSCALL_ERROR;
    }
    setPath(cwd, COMP_FILE);
    char * args[] = {cwd, studOutput, correctOutput, NULL};
    if ((pid = fork()) == 0) {
        if (execvp(args[0], args) == -1) return SYSCALL_ERROR;
    } else if (pid != -1){
        if (waitpid(pid, &status, 0) < 0) return SYSCALL_ERROR;
        int res = WEXITSTATUS(status);
        unlink(studOutput);
        switch (res){
            case 1:
                return GREAT;
            case 2:
                return BAD;
            case 3:
                return SIMILAR;

            default:
                return SYSCALL_ERROR;
        }
    } else return SYSCALL_ERROR;

}

int compile(char* path, char* cFileName) {
    pid_t pid;
    int status;
    char compiledFilePath[BUFF_SIZE];
    char filePath[BUFF_SIZE];
    strcpy(compiledFilePath, path);
    strcat(compiledFilePath, "/");
    strcpy(filePath, compiledFilePath);
    strcat(filePath, "/");
    strcat(filePath, cFileName);
    strcat(compiledFilePath, COMPILE_NAME);

    char * args[] = {"gcc", filePath, "-o", compiledFilePath, NULL};
    if ((pid = fork()) == 0) {
        if (execvp(args[0], args) == - 1) {
            return SYSCALL_ERROR;
        }
    } else if(pid != -1) {
        if (wait(&status) < 0) return 0;
        // error when compiling.
        if (WEXITSTATUS(status)) return COMPILE_ERROR;
        // if compiled seceded the return flag will be 1.
        return 1;
    } else return SYSCALL_ERROR;
    kill(pid, SIGKILL);
}



int run(char* dirPath, char* inputFilePath){

    pid_t pid;
    int status;
    int inFd = openFileForRead(inputFilePath);
    char compiledFilePath[BUFF_SIZE];
    strcpy(compiledFilePath, ".");
    setPath(compiledFilePath, dirPath);
    setPath(compiledFilePath, COMPILE_NAME);
    char * args[] = {compiledFilePath, inputFilePath, NULL};
    char outputFile[BUFF_SIZE];
    strcpy(outputFile, dirPath);
    setPath(outputFile, OUTPUT_FILE);
    int outFd = open(outputFile, O_CREAT | O_RDWR | O_TRUNC, 0777);
    if (inFd == -1 || outFd < 0) return SYSCALL_ERROR;

    if ((pid = fork()) == 0){
        // redirection.
        if ((dup2(inFd, 0) < 0) ||  (dup2(outFd, 1) < 0) || (execvp(args[0], args) == -1)){
            close(inFd);
            close(outFd);
            unlink(outputFile);
            return SYSCALL_ERROR;
        }


    } else if (pid != -1){
        close(inFd);
        close(outFd);
       int runtime = 0;

       while (!waitpid(pid, &status, WNOHANG) && runtime < MAX_TIME){
           runtime++;
           sleep(1);
       }
       unlink(compiledFilePath);
       if (runtime < MAX_TIME)
           return 1;
        unlink(outputFile);
        return TIMEOUT;



    } else{
        close(inFd);
        close(outFd);
        unlink(outputFile);
        return SYSCALL_ERROR;
    }
}

int execute(char* dirPath, char* inputFilePath, char* correctOutputFilePath, char* cFileName) {
    //compile
    int compileRes = compile(dirPath, cFileName);
    // compile failed.
    if (compileRes == COMPILE_ERROR)
        return COMPILE_ERROR;
    else if (compileRes == SYSCALL_ERROR) return SYSCALL_ERROR;
    // run the compiled file
    int runRes = run(dirPath, inputFilePath);
    if (runRes == TIMEOUT) return TIMEOUT;
    else if(runRes  == SYSCALL_ERROR) {
        return SYSCALL_ERROR;
    }
    int cmpRes = cmpOutput(dirPath, correctOutputFilePath);

    return cmpRes;
}

void setResults(int result, student* currStudent) {
    switch (result){
        case BAD:
            strcpy(currStudent->grade, BAD_OUTPUT_GRADE) ;
            strcpy(currStudent->description, BAD_OUTPUT_DESCRIPTION);
            break;
        case COMPILE_ERROR:
            strcpy(currStudent->grade, COMPILE_ERROR_GRADE);
            strcpy(currStudent->description, COMPILE_ERROR_DESCRIPTION) ;
            break;
        case SIMILAR:
            strcpy(currStudent->grade, SIMILAR_OUTPUT_GRADE);
            strcpy(currStudent->description , SIMILAR_OUTPUT_DESCRIPTION);
            break;
        case GREAT:
            strcpy(currStudent->grade, GREAT_JOB_GRADE);
            strcpy(currStudent->description, GREAT_JOB_DESCRIPTION);
            break;
        case TIMEOUT:
            strcpy(currStudent->grade, TIMEOUT_GRADE);
            strcpy(currStudent->description, TIMEOUT_ERROR_DESCRIPTION) ;

    }
}

void writeToResults(student* currStudent) {
    char studentResult[BUFF_SIZE] = {0};
    strcpy(studentResult, currStudent->name);
    strcat(studentResult, ",");
    strcat(studentResult, currStudent->grade);
    strcat(studentResult, ",");
    strcat(studentResult, currStudent->description);
    strcat(studentResult, "\n");
    int resultFd = open(RESULT_FILENAME, O_APPEND | O_CREAT | O_WRONLY, S_IRUSR | S_IWGRP | S_IRGRP | S_IWUSR);
    if (resultFd < 0) {
        error();
        return;
    }
    if (write(resultFd, studentResult, sizeof(studentResult)) < 0)
        error();
    close(resultFd);
}

int isCFile(char* file){
    return (file[strlen(file) - 2] == '.' && file[strlen(file) -1] == 'c');
}

int searchInDir(char* dirPath, char* inputPath, char* outputPath) {
    DIR* dir;
    struct dirent* pDirent;
    if ((dir = opendir(dirPath)) == NULL) {
        error();
        return 0;
    }

    while ((pDirent = readdir(dir)) != NULL) {
        char  pathToFile[BUFF_SIZE];
        strcpy(pathToFile, dirPath);
        strcat(pathToFile, "/");
        strcat(pathToFile, pDirent->d_name);
        student currStudent;
        if (strcmp(pDirent->d_name, ".") == 0 ||
            strcmp(pDirent->d_name, "..") == 0) {
            continue;
        }
        if (pDirent->d_type != DT_DIR) continue;
        // student name is the folder name.
        strcpy(currStudent.name, pDirent->d_name);
        struct dirent* userDirent;
        DIR * userName;
        if ((userName = opendir(pathToFile)) == NULL) {
            error();
            closedir(dir);
            return 0;
        }
        int found = 0;
        while ((userDirent = readdir(userName)) != NULL){
            if (strcmp(userDirent->d_name, ".") == 0 ||
                strcmp(userDirent->d_name, "..") == 0)
                continue;

            // c file founded.
            if (isCFile(userDirent->d_name)) {
                found = 1;
               int result = execute(pathToFile, inputPath, outputPath, userDirent->d_name);
               if (result == SYSCALL_ERROR) {
                   closedir(userName);
                   closedir(dir);
                   error();
                   return 0;
               }
               setResults(result, &currStudent);
                break;
            }

        }
        if (!found) {
            strcpy(currStudent.grade, NO_FILE_GRADE);
            strcpy(currStudent.description, NO_FILE_DESCRIPTION);
        }

        writeToResults(&currStudent);
        closedir(userName);
    }
    closedir(dir);
    return 1;
}




int main(int argc, char** argv) {

    if (argc != 2){
        perror("Not enough params!");
        return 0;
    }
    int fd = openFileForRead(argv[1]);
    char buffer[MAX_SIZE];
    bzero(buffer, MAX_SIZE);
    if (read(fd, buffer, MAX_SIZE) < 0) {
        error();
        close(fd);
        exit(2);
    }
    close(fd);
    const char delims[2] = {'\n', '\r'};
    char * lines[3];
    int i = 0;
    lines[i] = strtok(buffer, delims);
    while (lines[i++] != NULL){
        lines[i] = strtok(NULL, delims);
    }
    searchInDir(lines[0], lines[1], lines[2]);

    return 0;
}
