#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "fileutil.h"

#define MAX_LINE_SIZE (256)
#define END_OF_INPUT (0)
#define NO_ERROR (0)
#define WRONG_INPUT (-2)
#define TIMEOUT_EXCEEDED (-3)

#define TIMEOUT (5)
#define ALARM_RESET (0)

void alarmSignalHandler(int sig){}

int parseLong(char * line, long * val) {
    int base = 10;
    char *lineTail = line;
    
    int errnoSave = errno;
    errno = NO_ERROR;

    long number = strtol(line, &lineTail, base);
    if (number == 0 && errno != NO_ERROR) {
        return FAILURE_CODE;
    }
    if (lineTail == line) {
        return FAILURE_CODE;
    }

    static const char spaceCharSet[] = " \t\n\v\f\r";
    unsigned long strLen = strlen(lineTail);
    size_t spaceSeqLen = strspn(lineTail, spaceCharSet);
    if ((size_t)strLen != spaceSeqLen) {
        return FAILURE_CODE;
    }
    *val = number;
    
    errno = errnoSave;
    return SUCCESS_CODE;
}

long readLineNum(int fd){ 
    char line[MAX_LINE_SIZE] = {0};
    ssize_t readRes = read(fd, line, MAX_LINE_SIZE);
    if(readRes < 0 && errno == EINTR){
	return TIMEOUT_EXCEEDED;
    }
    if(readRes < 0){
        fprintf(stderr, "read() failed\n");
        return FAILURE_CODE;
    }
    long lineNum;
    int parseRes = parseLong(line, &lineNum);
    if(FAILURE_CODE == parseRes){
        return WRONG_INPUT;
    }
    return lineNum;
}

int readLinesFromFile(TextFile * file, int terminal){
    while(1){ 
        alarm(TIMEOUT);
        int lineNumber = readLineNum(terminal); 
        alarm(ALARM_RESET);
	if(END_OF_INPUT == lineNumber) {
            break;
        }
        if(FAILURE_CODE == lineNumber){
            fprintf(stderr, "readLineNum() failed\n");
            return FAILURE_CODE;
        }
        if(WRONG_INPUT == lineNumber){
            fprintf(stderr,"Bad input. Try once more:\n");
            continue;    
        }
        if(TIMEOUT_EXCEEDED == lineNumber){
            int printRes = printFile(file);
            if(FAILURE_CODE == printRes){
                fprintf(stderr,"printFile() failed\n");
                return FAILURE_CODE;
            }
            return SUCCESS_CODE;
        }
        if(lineNumber > file->linesNum){
            fprintf(stderr,"Too big line number. Try once more:\n");
            continue;
        }
        char lineToPrint[BUFSIZ];
        int getLineRes = getLineFromTextFile(lineToPrint, file, lineNumber);
        if(getLineRes == FAILURE_CODE){
            fprintf(stderr, "getLine() failed\n");
            return FAILURE_CODE;
        }
        printf("Line %d :\n", lineNumber);
        ssize_t writeRes = write(STDOUT_FILENO, lineToPrint, file->strlens[lineNumber - 1]);
        if (0 > writeRes){
            perror("write() failed");
            return FAILURE_CODE;
        }
    }
    return SUCCESS_CODE;
}

int main(int argc, char *argv[]){
    if (argc != 2){
        fprintf(stderr, "Invalid arguments number\n");
        exit(EXIT_FAILURE);
    }
    TextFile file;    
    int openRes = openTextFile(&file, argv[1]);
    if(FAILURE_CODE == openRes){
        fprintf(stderr, "openTextFile() failed\n");
        exit(EXIT_FAILURE);
    }
    openRes = open("/dev/tty", O_RDONLY);
    if(FAILURE_CODE == openRes){
        fprintf(stderr, "open() failed\n");
        exit(EXIT_FAILURE);
    }
    int terminal = openRes;
      
    void (*prevSigHandler)(int) = sigset(SIGALRM, alarmSignalHandler);
    if (prevSigHandler == SIG_ERR) {
        perror("signal() failed");
        return EXIT_FAILURE;
    }

    int readLinesRes = readLinesFromFile(&file, terminal);
    if(FAILURE_CODE == readLinesRes){
        fprintf(stderr, "readLinesRes() failed\n");
        exit(EXIT_FAILURE);         
    }

    int closeRes = closeTextFile(&file);
    if(FAILURE_CODE == closeRes){
        fprintf(stderr, "closeTextFile() failed\n");
        exit(EXIT_FAILURE);
    }
    closeRes = close(terminal);
    if(FAILURE_CODE == closeRes){
        fprintf(stderr, "close() failed\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
