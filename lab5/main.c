#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include "fileutil.h"

#define MAX_LINE_SIZE (256)
#define END_OF_INPUT (0)
#define TRUE (1)
#define FALSE (0)
#define NO_ERROR (0)
#define STDOUT (0)
#define WRONG_INPUT (-2)

int parseInt(char * line, unsigned * val) {
    int base = 10;
    char *lineTail = line;

    int errnoSave = errno;
    errno = NO_ERROR;

    unsigned number = (int)strtol(line, &lineTail, base);
    if (number == 0 && errno != NO_ERROR) {
        return FAILURE_CODE;
    }
    if (lineTail == line) {
        return FAILURE_CODE;
    }

    static const char spaceCharSet[] = " \t\n\v\f\r";
    size_t strLen = strlen(lineTail);
    size_t spaceSeqLen = strspn(lineTail, spaceCharSet);
    if ((size_t)strLen != spaceSeqLen) {
        return FAILURE_CODE;
    }
    *val = number;

    errno = errnoSave;
    return SUCCESS_CODE;
}

int readLineNum(){
    char line[MAX_LINE_SIZE] = {0};
    ssize_t readRes = read(STDIN_FILENO, line, MAX_LINE_SIZE);
    if(readRes < 0){
        fprintf(stderr, "read() failed\n");
        return FAILURE_CODE;
    }

    if(readRes < 0){
        fprintf(stderr, "read() failed\n");
        return FAILURE_CODE;
    }
    int lineNum;
    int parseRes = parseInt(line, &lineNum);
    if(FAILURE_CODE == parseRes){
        return WRONG_INPUT;
    }
    return lineNum;
}

int readLinesFromFile(TextFile * file){
    int lineNumber = FAILURE_CODE;
    while(END_OF_INPUT != lineNumber){
	lineNumber = readLineNum();
        if(FAILURE_CODE == lineNumber){
            fprintf(stderr, "readLineNum() failed\n");
            return FAILURE_CODE;
        }
	if(WRONG_INPUT == lineNumber){
	    printf("Wrong input, try again:\n");
	    continue;
	}
        if(lineNumber > file->linesNum){
            fprintf(stderr,"Too big line number. Try once more:\n");
            continue;
        }
	if(END_OF_INPUT == lineNumber){
	    continue;
	}
        
        char lineToPrint[BUFSIZ];
        int getLineRes = getLineFromTextFile(lineToPrint, file, lineNumber);
        if(getLineRes == FAILURE_CODE){
            fprintf(stderr, "getLine() failed\n");
            return FAILURE_CODE;
        }
        printf("Line %d:\n", lineNumber);
        ssize_t writeRes = write(STDOUT, lineToPrint, file->strlens[lineNumber - 1]);
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

    int readLinesRes = readLinesFromFile(&file);
    if(FAILURE_CODE == readLinesRes){
        fprintf(stderr, "readLinesRes() failed\n");
        exit(EXIT_FAILURE);         
    }

    int closeRes = closeTextFile(&file);
    if(FAILURE_CODE == closeRes){
        fprintf(stderr, "closeTextFile() failed\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
