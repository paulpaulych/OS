#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <unistd.h>

#define MAX_LINE_COUNT (100)
#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)
#define LSEEK_ERROR ((off_t)-1)

#define NOT_EOF (0)
#define NO_FERROR (0)

typedef struct textFile {
    int fd;
    size_t linesNum;
    size_t strlens[MAX_LINE_COUNT];
    off_t offsets[MAX_LINE_COUNT];
} TextFile;

int openTextFile(TextFile *file, char *filename);
int closeTextFile(TextFile *file);

int getLineFromTextFile(char * line, TextFile * f, size_t lineNumber);

#endif // FILEUTIL_H

