#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <ctype.h>
#include <arpa/inet.h>
#include <string>
#include <map>

#define BUFFER 500
#define GN 32

using namespace std;

socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
int sfd, s, j;
size_t len;
ssize_t nread, n;
char senderBuf[500] = "";
char receiverBuf[500] = "";
char PDIP[50] = "";
char PDport[6]= "57032";
char ASIP[50] = "localhost";
char ASport[6] = "58032";
char command[5] = "";
char UID[6] = "";
char pass[9] = "";
map<const char*, int> commands;

void parseArgs(int argc, char *argv[]) {
    if (argc < 2 || argc > 8) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    strcpy(PDIP, argv[1]);
    for (int i = 2; i < argc; i += 2) {
        if (!strcmp(argv[i], "-d"))
            strcpy(PDport, argv[i + 1]);
        else if (!strcmp(argv[i], "-n"))
            strcpy(ASIP, argv[i + 1]);
        else if (!strcmp(argv[i], "-p"))
            strcpy(ASport, argv[i + 1]);
    }
}

bool isNumeric(char *str) {
    for (int i = 0; i < strlen(str); i++)
        if (isdigit(str[i]) == false)
            return false;
    return true;
}

bool isAlphanumeric(char *str) {
    for (int i = 0; i < strlen(str); i++)
        if (isalnum(str[i]) == false)
            return false;
    return true;
}

bool checkUID(char *str) {
    if (strlen(str) != 5 || !isNumeric(str)) {  
        perror("UID Error");
        close(sfd);
        return false;
    }
    return true;
}

bool checkPass(char *str) {
    if (strlen(pass) != 8 || !isAlphanumeric(pass)) {
        perror ("Pass Error");
        close(sfd);
        return false;
    }
    return true;
}

void sendToServer(int sfd, char *buf) {
    if (sendto(sfd, buf, strlen(buf), 0, res->ai_addr, res->ai_addrlen) == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd); 
        exit(EXIT_FAILURE);
    }
}

char* createString(const char **args, int len) {
    for (int i = 0; i < len; i++) {
        strcat(senderBuf, args[i]);
    }
    return senderBuf;
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    s = getaddrinfo(ASIP, ASport, &hints, &res);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close(sfd); 
        exit(EXIT_FAILURE);
    }

    while (1) {
        char str[50];
        cin.getline(str, 50);
        commands = {{"exit", 0}, {"reg", 1}};
        sscanf(str, "%s ", command);
        cout << commands.find(command) << endl;
        int code = (int) commands.find(command);
        switch (code) {
            case 0:
                sendToServer(sfd, createString("UNR", UID, pass));
                close(sfd);
                break;
            case 1:
                sscanf(str, "%s %s %s", command, UID, pass);
                if (!checkUID(UID))
                    exit(EXIT_FAILURE);
                if (!checkPass(pass))
                    exit(EXIT_FAILURE);
                sendToServer(sfd, createString(["REG", " ", UID, " ", pass, " ", PDIP, " ", PDport, "\n"], 10));
                break;
            default:
                exit(1);
        }

        cout << senderBuf << endl;
        sendToServer(sfd, senderBuf);

        addrlen = sizeof(addr);
        if (recvfrom(sfd, receiverBuf, BUFFER, 0, (struct sockaddr*) &addr, &addrlen) == -1)
            exit(1);
        cout << receiverBuf;
    }
    close(sfd);
    exit(EXIT_SUCCESS);
}