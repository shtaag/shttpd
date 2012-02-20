#include <stdio.h>
#include "filereader.h"

FILE *FileOpen(const char *, const char *);
void FileRead(FILE *, const char *, char *);
void OpenAndRead(const char *, const char *, char *);

FILE *FileOpen(const char *filename, const char *mode) {

    return fopen(filename, mode);
}

void FileRead(FILE *fp, const char *format, char *chararr) {

    if (fp == NULL) return;
    char c;
    int i = 0;
    while ((c = fgetc(fp)) != EOF) {
	printf("%c",c);
	*(chararr + i) = c;
	i++;
    }
    fclose(fp);
    char *p;
    fp = NULL;
    return;

}

void OpenAndRead(const char *filename, const char *format, char *chararr) {

    FILE *fp;
    fp = FileOpen(filename, "r");

    if (fp == NULL) {
	printf("no file.\n");
	return;
    }

    FileRead(fp, format, chararr);
    return;
}

int FileReaderMain(void) {

    FILE *fp;
    fp = FileOpen("hello.txt","r");
    if (fp == NULL) {
	printf("no file.\n");
	return -1;
    }

    char str[1024];
    char *result;
    char *p;
    
    FileRead(fp, "%s", str);
    if (str != NULL) printf("%s",str);

    printf("sizeof result pointer: %zd\n", sizeof(&str));
    printf("sizeof result array: %zd\n", sizeof(str));

    return 0;
}

