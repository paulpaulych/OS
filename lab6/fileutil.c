#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fileutil.h"

int fillStrlensTable(TextFile * file);
void fillOffsetsTable(TextFile * file);

int openTextFile(TextFile *file, char * filename){
    int openRes = open(filename, O_RDONLY);
    if (openRes == FAILURE_CODE){
        perror("open() failed");
        return FAILURE_CODE;
    }
    file->fd = openRes;
    int fillRes = fillStrlensTable(file);    
    if(FAILURE_CODE == fillRes){
        fprintf(stderr, "fillStrlensTable() failed\n");
        return FAILURE_CODE;
    }
    fillOffsetsTable(file);
    return SUCCESS_CODE;
}

int closeTextFile(TextFile *file){
    int closeRes = close(file->fd);  
    if (0 > closeRes){
        perror("close() failed");
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}

int getLineFromTextFile(char * line, TextFile * f, size_t lineNumber){
    off_t lseekRes = lseek(f->fd, f->offsets[lineNumber - 1], SEEK_SET);
    if(lseekRes == LSEEK_ERROR){
        perror("lseek() failed");
        return FAILURE_CODE;
    }
    ssize_t readRes = read(f->fd, line, f->strlens[lineNumber - 1]);
    if (0 > readRes){
        perror("read() failed");
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}

int fillStrlensTable(TextFile *file){
    char curChar;
    size_t curLineLen = 0, linesNum = 0;
    ssize_t readRes;
    do{
        readRes = read(file->fd, &curChar, 1);
        if(0 > readRes){
            perror("read() failed");
            return FAILURE_CODE;
        }
        curLineLen++;
        if ('\n' != curChar && readRes > 0){
            continue;
        }
        file->strlens[linesNum] = curLineLen;
        linesNum++;
        if(curChar == '\n'){
            curLineLen = 0;
        }
    }while(readRes > 0);
    if(curChar == '\n'){
        file->strlens[linesNum-1] = 0;
    }else{
        file->strlens[linesNum-1] = curLineLen-1;
    }
    file->linesNum = linesNum;
    return SUCCESS_CODE;
}

void fillOffsetsTable(TextFile * file){
    int i;
    off_t offset = 0;
    for(i = 0; i < file->linesNum; ++i){
        file->offsets[i] = offset;
        offset += file->strlens[i];
    }
}

int printFile(TextFile * file){
    int lineNum;
    for(lineNum = 0; lineNum < file->linesNum; ++lineNum){
        char lineToPrint[BUFSIZ] = {0};
        int getLineRes = getLineFromTextFile(lineToPrint, file, lineNum+1);
        if(getLineRes == FAILURE_CODE){
            fprintf(stderr, "getLine() failed\n");
            return FAILURE_CODE;
        }
        ssize_t writeRes = write(STDOUT_FILENO, lineToPrint, file->strlens[lineNum]);
        if (0 > writeRes){
            perror("write() failed");
            return FAILURE_CODE;
        }
    }
    return SUCCESS_CODE;
}

