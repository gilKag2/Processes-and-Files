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

#define OUTPUT_FILE "/output.txt"
#define COMPILE_NAME "student.out"
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
    char * name;
    char * description;
    char* grade;
} student;

int resFd;


int openFileForRead(char* path) {
    int fd;
    if ((fd = open(path, O_RDONLY)) < 0) {
        error();
        return -1;
    }
    return fd;

}

int compile(char* path) {
    pid_t pid;
    int status;

    char * args[] = {"gcc", path, "-o", COMPILE_NAME, NULL};
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

int cmpOutput(char* studOutput, char * correctOutput){
    pid_t  pid;
    int status;
    char  cwd[MAX_SIZE]= {0};
    if (getcwd(cwd, sizeof(cwd)) == NULL) return SYSCALL_ERROR;
    // set up path to the comp.out file in the cwd.
    strcat(cwd, "./comp.out");
    char * args[] = {cwd, studOutput, correctOutput, NULL};
    if ((pid = fork()) == 0) {
        if (execvp(args[0], args) == -1) return SYSCALL_ERROR;
    } else if (pid != -1){
        if (waitpid(pid, &status, 0) < 0) return SYSCALL_ERROR;
        int res = WEXITSTATUS(status);
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

int run(char* compiledFilepath, char* inputFilePath){

    pid_t pid;
    int status;
    int inFd = openFileForRead(inputFilePath);

    char * args[] = {compiledFilepath, inputFilePath, NULL};
    int outFd = open(OUTPUT_FILE, O_CREAT | O_RDWR | O_TRUNC, 0777);
    if (inFd == -1 || outFd < 0) return SYSCALL_ERROR;
    if ((pid = fork()) == 0){
        // redirection.
        if (dup2(inFd, 0) < 0) return SYSCALL_ERROR;
        if (dup2(outFd, 1) < 0) return SYSCALL_ERROR;
        if (execvp(args[0], args) == -1) return SYSCALL_ERROR;

    } else if (pid != -1){
       int runtime = 0;
       // count the runtime.
       while (!waitpid(pid, &status, WNOHANG) && runtime++ < 6)
           sleep(1);

       // if the program ran more then 5 seconds the return value will be TIMEOUT.
       if (runtime > 5) {
           // remove the outfile that has been created.
           unlink(OUTPUT_FILE);
           return TIMEOUT;
       }
        return 1;

    } else{
        return  SYSCALL_ERROR;
    }
    kill(pid, SIGSTOP);
}

int execute(char* cFilepath, char* inputFilePath, char* correctOutputFilePath) {
    //compile
    int compileRes = compile(cFilepath);
    // compile failed.
    if (compileRes == COMPILE_ERROR)
        return COMPILE_ERROR;
    else if (compileRes == SYSCALL_ERROR) return SYSCALL_ERROR;
    char  compiledFileLocation[MAX_SIZE] = {0};
    strcpy(compiledFileLocation, "./");
    // compiled file path
    strcat(compiledFileLocation, cFilepath);
    strcat(compiledFileLocation, "/");
    strcat(compiledFileLocation, COMPILE_NAME);
    // run the compiled file
    int runRes = run(compiledFileLocation, inputFilePath);
    if (runRes == TIMEOUT) return TIMEOUT;
    else if(runRes  == SYSCALL_ERROR) {
        unlink(compiledFileLocation);
        unlink(OUTPUT_FILE);
        return SYSCALL_ERROR;
    }

    int cmpRes = cmpOutput(OUTPUT_FILE, correctOutputFilePath);
    unlink(compiledFileLocation);
    unlink(OUTPUT_FILE);
    return cmpRes;
}

void setResults(int result, student* currStudent) {
    switch (result){
        case BAD:
            currStudent->grade = BAD_OUTPUT_GRADE;
            currStudent->description =  BAD_OUTPUT_DESCRIPTION;
        case COMPILE_ERROR:
            currStudent->grade = COMPILE_ERROR_GRADE;
            currStudent->description = COMPILE_ERROR_DESCRIPTION;
        case SIMILAR:
            currStudent->grade = SIMILAR_OUTPUT_GRADE;
            currStudent->description = SIMILAR_OUTPUT_DESCRIPTION;
        case GREAT:
            currStudent->grade = GREAT_JOB_GRADE;
            currStudent->description = GREAT_JOB_DESCRIPTION;
        case TIMEOUT:
            currStudent->grade = TIMEOUT_GRADE;
            currStudent->description = TIMEOUT_ERROR_DESCRIPTION;

    }
}

void writeToResults(student* currStudent) {
    char studentResult[MAX_SIZE] = {0};
    strcpy(studentResult, currStudent->name);
    strcat(studentResult, ",");
    strcat(studentResult, currStudent->grade);
    strcat(studentResult, ",");
    strcat(studentResult, currStudent->description);
    strcat(studentResult, "\n");
    int resultFd = open(RESULT_FILENAME, O_CREAT | O_RDWR | O_TRUNC, 0777);
    if (resultFd < 0) {
        error();
        return;
    }
    if (write(resultFd, studentResult, sizeof(studentResult)) < 0)
        error();
    close(resultFd);


}

int searchInDir(char* dirPath, char* inputPath, char* outputPath) {
    DIR* dir;
    struct dirent* pDirent;
    if ((dir = opendir(dirPath)) == NULL) {
        error();
        return 0;
    }
    char * pathToFile = {0};
    strcpy(pathToFile, dirPath);
    while ((pDirent = readdir(dir)) != NULL) {
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
        if ((userName = opendir(pDirent->d_name)) == NULL) {
            error();
            closedir(dir);
            return 0;
        }
        while ((userDirent = readdir(userName)) != NULL){
            strcat(pathToFile, "/");
            strcat(pathToFile, userDirent->d_name);
            if (strcmp(userDirent->d_name, ".") == 0 ||
                strcmp(userDirent->d_name, "..") == 0)
                continue;
            // c file founded.
            if (strstr(userDirent->d_name, ".c") != NULL) {

               int result = execute(pathToFile, inputPath, outputPath);
               if (result == SYSCALL_ERROR) {
                   closedir(userName);
                   closedir(dir);
                   error();
                   return 0;
               }
               setResults(result, &currStudent);
            }

        }
        currStudent.grade = NO_FILE_GRADE;
        strcpy(currStudent.description, NO_FILE_DESCRIPTION);
        writeToResults(&currStudent);
        closedir(userName);
    }
    closedir(dir);
    return 1;
}




int main(int argc, char** argv) {
    //confInfo info;
    if (argc != 2){
        perror("Not enough params!");
        return 0;
    }
    int fd = openFileForRead(argv[1]);
    char buffer[MAX_SIZE];
    bzero(buffer, BUFF_SIZE);
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
    while (lines[i] != NULL){
        lines[i++] = strtok(NULL, delims);
    }
    searchInDir(lines[0], lines[1], lines[2]);
    return 0;
}
