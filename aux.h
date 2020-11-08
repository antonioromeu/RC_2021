#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <fstream>

#define GN 32
#define max(A, B) ((A) >= (B) ? (A) : (B))

using namespace std;

struct timeval timeout;
fd_set readfds;
int out_fds, sfd;
char PDIP[50];
char PDport[6]= "57032";
char ASIP[50] = "localhost";
char ASport[6] = "58032";
char command[128];
char UID[6];
char recvUID[6];
char pass[9];
char buffer[1024];
char FSIP[50] = "localhost";
char FSport[6]= "59032";
char Fop[50];
char Fname[50];
char RID[5];
char VC[5];
char TID[5];
char filename[128];
char Fsize[10];

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
    strcpy(buffer, "\0");
    for (int i = 0; i < len; i++)
        strcat(buffer, args[i]);
    return buffer;
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