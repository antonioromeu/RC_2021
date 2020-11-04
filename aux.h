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

#define BUFFER 500
#define GN 32
#define max(A, B) ((A) >= (B) ? (A) : (B))

using namespace std;

// socklen_t addrlenClient, addrlenServer;
// struct addrinfo hintsClient, hintsServer, *resClient, *resServer;
// struct sockaddr_in addrClient, addrServer;
struct timeval timeout;
size_t len;
ssize_t nread, n;
fd_set inputs, testfds, rfds, wfds, pdfds, readfds, writefds;
int i, out_fds, sfd, s, errocode, j, newfd, counter;
char str[128] = "";
char senderBuf[BUFFER] = "";
char receiverBuf[BUFFER] = "";
char PDIP[50] = "";
char PDport[6]= "57032";
char ASIP[50] = "localhost";
char ASport[6] = "58032";
char command[128] = "";
char UID[6] = "";
char recvUID[6] = "";
char pass[9] = "";
char buffer[128] = "";
char FSIP[50] = "localhost";
char FSport[6]= "59032";
char Fop[50] = "";
char Fname[50] = "";
char RID[5] = "";
char VC[5] = "";
char TID[5] = "";
char filename[128] = "";

void append(char *s, char c) {
    int len = strlen(s);
    s[len] = c;
    s[len + 1] = '\0';
}

bool isNumeric(char *str) {
    for (int i = 0; i < (int) strlen(str); i++)
        if (isdigit(str[i]) == false)
            return false;
    return true;
}

bool isAlphanumeric(char *str) {
    for (int i = 0; i < (int) strlen(str); i++)
        if (isalnum(str[i]) == false)
            return false;
    return true;
}

bool checkUID(int sfd, char *str) {
    if (strlen(str) != 5 || !isNumeric(str)) {  
        perror("UID Error");
        close(sfd);
        return false;
    }
    return true;
}

bool checkPass(int sfd, char *str) {
    if (strlen(str) != 8 || !isAlphanumeric(str)) {
        perror ("Pass Error");
        close(sfd);
        return false;
    }
    return true;
}

bool checkFilename(char *str) {
    cout << str << endl;
    if (strlen(str) > 24)
        return false;
    for (int i = 0; i < (int) strlen(str); i++) {
        if (isalnum(str[i]) || str[i] == '.' || str[i] == '-' || str[i] == '_') {
            cout << "entrou " << i << str[i] << endl;
            continue;
        }
        else {
            cout << "nao entrou " << i << "---" << str[i] << "---" << endl;
            return false;
        }
    }
    return true;
}

char* createString(const char **args, int len) {
    strcpy(senderBuf, "\0");
    for (int i = 0; i < len; i++)
        strcat(senderBuf, args[i]);
    return senderBuf;
}