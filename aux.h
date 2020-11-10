#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include <vector>
#include <cstring>

#define BUFSIZE 1024
#define FILENAMESIZE 28
#define DIRSIZE 12
#define GN 32
#define max(A, B) ((A) >= (B) ? (A) : (B))

using namespace std;

char newString[128];

bool isNumeric(char *str) {
    for (int i = 0; i < (int) strlen(str); i++)
        if (!isdigit(str[i]))
            return false;
    return true;
}

bool isAlphanumeric(char *str) {
    for (int i = 0; i < (int) strlen(str); i++)
        if (!isalnum(str[i]))
            return false;
    return true;
}

bool checkUID(char *str) {
    return (strlen(str) == 5 && isNumeric(str));
}

bool checkPass(char *str) {
    return (strlen(str) == 8 && isAlphanumeric(str));
}

bool checkFilename(char *str) {
    if (strlen(str) > 24)
        return false;
    for (int i = 0; i < (int) strlen(str); i++) {
        if (isalnum(str[i]) || str[i] == '.' || str[i] == '-' || str[i] == '_')
            continue;
        else
            return false;
    }
    return true;
}

char* createString(const char **args, int len) {
    strcpy(newString, "\0");
    for (int i = 0; i < len; i++)
        strcat(newString, args[i]);
    return newString;
}

void reverse(char *str, int length) { 
    int start = 0; 
    int end = length -1; 
    while (start < end) { 
        swap(*(str+start), *(str+end)); 
        start++; 
        end--; 
    } 
} 

char* itoa(int num, char *str, int base) {
    int i = 0;
    bool isNegative = false;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str; 
    }
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
    if (isNegative)
        str[i++] = '-';
    str[i] = '\0';
    reverse(str, i);

    return str; 
} 

bool checkDir(char *subdir) {
    DIR *d;
    struct dirent *dir;
    d = opendir("/USERS");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (!strcmp(dir->d_name, subdir)) {
                closedir(d);
                return true;
            }
        }
        closedir(d);
        return false;
    }
    return false;
}