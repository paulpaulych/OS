#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include "fileutil.h"

#define MAX_LINE_SIZE (5)
#define END_OF_INPUT (0)
#define TRUE (1)
#define FALSE (0)
#define BASE (10)
#define NO_ERROR (0)
#define STDOUT (0)
#define WRONG_INPUT (-2)

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
    if(lineTail == line) {
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

char* readLine(){
    char buf[BUFSIZ] = {0};
    int bufLen = 1;
    int curStrLen = 0;
    char *outStr = NULL;
    while('\n' != buf[bufLen-1]){   
        char* fgetsRes = fgets(buf, BUFSIZ, stdin);
        if(fgetsRes == NULL && ferror(stdin)){
            fprintf(stderr, "stdin read failed\n");
            free(outStr);
            return NULL;
        }
	if(fgetsRes == NULL && feof(stdin)){
	    fprintf(stderr, "fgets() failed: EOF encountered\n");
	    free(outStr);
	    return NULL;
	}
        bufLen = strlen(buf);
        curStrLen += bufLen;
        char* reallocRes = (char *)realloc(outStr, curStrLen);
        if(!reallocRes){
            perror("realloc() failed");
            free(outStr);
            return NULL;
        }
        if(!outStr){
            reallocRes[0] = '\0';
        }
        outStr = reallocRes;
        strcat(outStr, buf);
    }
    return outStr;
}

int getLineNum(){
    char * line = readLine();
    if(line == NULL) {
	fprintf(stderr, "readLine() failed\n");
        return FAILURE_CODE;
    }
    int lineNum;
    int parseRes = parseUnsigned(line, &lineNum);
    if(FAILURE_CODE == parseRes){
        return WRONG_INPUT;
    }
    return lineNum;
}

int readLinesFromFile(TextFile * file){
    int endOfInput = FALSE;
    while(!endOfInput){
	int lineNumber = getLineNum();
        if(FAILURE_CODE == lineNumber){
            fprintf(stderr, "getLineNum() failed\n");
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
    	    endOfInput = TRUE;
            continue;
	}
	char * lineToPrint = getLineFromTextFile(file, lineNumber);
        if(!lineToPrint){
            fprintf(stderr, "getLine() failed\n");
            return FAILURE_CODE;
        }
	fprintf(stdout, "Line %d:\n", lineNumber);
        ssize_t writeRes = write(STDOUT_FILENO, lineToPrint, file->strlens[lineNumber-1]);
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
        fprintf(stderr, "readLinesFromFile() failed\n");
	closeTextFile(&file);
        exit(EXIT_FAILURE);         
    }

    int closeRes = closeTextFile(&file);
    if(FAILURE_CODE == closeRes){
        fprintf(stderr, "closeTextFile() failed\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
