#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "fileutil.h"

#define BASE (10)
#define MAX_LINE_SIZE (256)
#define END_OF_INPUT (0)
#define NO_ERROR (0)
#define WRONG_INPUT (-2)
#define TIMEOUT_EXCEEDED (-3)
#define TRUE (1)
#define FALSE (0)

#define TIMEOUT (5)
#define ALARM_RESET (0)

void alarmSignalHandler(int sig){}

int parseUnsigned(char * line, unsigned * val) {
    int errnoSaved = errno;
    errno = NO_ERROR;
    char *lineTail = line;
    long number = strtol(line, &lineTail, BASE);
    if ((number == 0 || number == ULONG_MAX) && errno != NO_ERROR) {
	errno = errnoSaved;
        return FAILURE_CODE;
    }
    if(number < 0){
	errno = errnoSaved;
	return FAILURE_CODE;
    }
    if (lineTail == line) {
	errno = errnoSaved;
        return FAILURE_CODE;
    }
    static const char spaceCharSet[] = " \t\n\v\f\r";
    size_t strLen = strlen(lineTail);
    size_t spaceSeqLen = strspn(lineTail, spaceCharSet);
    if ((size_t)strLen != spaceSeqLen) {
	errno = errnoSaved;
        return FAILURE_CODE;
    }
    *val = (unsigned)number;	
    errno = errnoSaved;
    return SUCCESS_CODE;
}

int readLine(char ** outStr_){
    char buf[BUFSIZ] = {0};
    size_t bufLen = 1;
    int curStrLen = 0;
    char *outStr = NULL;
    while('\n' != buf[bufLen-1]){
        char* fgetsRes = fgets(buf, BUFSIZ, stdin);
        if(fgetsRes == NULL && ferror(stdin) && errno == EINTR){
            free(outStr);
	    return TIMEOUT_EXCEEDED;
	}
	if(fgetsRes == NULL && ferror(stdin)){
            fprintf(stderr, "stdin read failed\n");
            free(outStr);
            return FAILURE_CODE;
        }
        if(fgetsRes == NULL && feof(stdin)){
            fprintf(stderr, "fgets() failed: EOF encountered\n");
            free(outStr);
            return FAILURE_CODE;
        }
        bufLen = strlen(buf);
        curStrLen += bufLen;
        char* reallocRes = (char *)realloc(outStr, curStrLen);
        if(!reallocRes){
            perror("realloc() failed");
            free(outStr);
            return FAILURE_CODE;
        }
        if(!outStr){
            reallocRes[0] = '\0';
        }
        outStr = reallocRes;
        strcat(outStr, buf);
    }
    *outStr_ = outStr;
    return SUCCESS_CODE;
}

int getLineNum(int fd){ 
    char * line;
    int readLineRes = readLine(&line);
    if(readLineRes == TIMEOUT_EXCEEDED){
	return TIMEOUT_EXCEEDED;
    }
    if(readLineRes == FAILURE_CODE){
	fprintf(stderr, "readLine() failed\n");
	return FAILURE_CODE;
    }
    unsigned lineNum;
    int parseRes = parseUnsigned(line, &lineNum);
        
    if(FAILURE_CODE == parseRes){
        return WRONG_INPUT;
    }
    return lineNum;
}

int readLinesFromFile(TextFile * file, int terminal){
    int endOfInput = FALSE;
    while(!endOfInput){ 
        alarm(TIMEOUT);
        int lineNumber = getLineNum(terminal); 
	alarm(ALARM_RESET);
	if(END_OF_INPUT == lineNumber) {
	    endOfInput = TRUE;
	    continue;        
	}
        if(FAILURE_CODE == lineNumber){
            fprintf(stderr, "getLineNum() failed\n");
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
            fprintf(stderr, "getLineFromTextFile() failed\n");
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
    int terminal = open("/dev/tty", O_RDONLY);
    if(FAILURE_CODE == terminal){
	closeTextFile(&file);
        fprintf(stderr, "open() failed\n");
        exit(EXIT_FAILURE);
    }
     
    void (*prevSigHandler)(int) = signal(SIGALRM, alarmSignalHandler);
    if (prevSigHandler == SIG_ERR) {
        perror("signal() failed");
        return EXIT_FAILURE;
    }

    int closeRes = closeTextFile(&file);
    if(FAILURE_CODE == closeRes){
        fprintf(stderr, "closeTextFile() failed\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
