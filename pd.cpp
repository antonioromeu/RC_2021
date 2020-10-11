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

#define BUFFER 500
#define GN 32

using namespace std;

socklen_t addrlen;
int sfd, s, j;
size_t len;
ssize_t nread, n;
char buffer[BUFFER] = "";
char PDIP[50] = "";
char PDport[6]= "57032";
char ASIP[50] = "localhost";
char ASport[6] = "58032";
char command[5] = "";
char UID[6] = "";
char pass[9] = "";

struct addrinfo hints, *res;

void parseArgs(int argc, char *argv[]){
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

bool isNumeric(string str) {
    for (int i = 0; i < str.length(); i++)
        if (isdigit(str[i]) == false)
            return false;
    return true;
}

bool isAlphanumeric(string str) {
    for (int i = 0; i < str.length(); i++)
        if (isalnum(str[i]) == false)
            return false;
    return true;
}

void sendToServer(int sfd, string buf) {
    if (sendto(sfd, &buf, buf.length(), 0, res->ai_addr, res->ai_addrlen) == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd); 
        exit(EXIT_FAILURE);
    }
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
        if (!strcmp(str, "exit")) { 
            close(sfd); 
            exit(EXIT_SUCCESS);
        }
        sscanf(str, "%s %s %s", command, UID, pass);

        if (strcmp(command, "reg")) {
            perror("Command is not reg");
            close(sfd);
            exit(EXIT_FAILURE);
        }
        
        if (strlen(UID) != 5 || !isNumeric(UID)) {  
            perror("UID Error");
            close(sfd);
            exit(EXIT_FAILURE);
        }

        if (strlen(pass) != 8 || !isAlphanumeric(pass)) {
            perror ("Pass Error");
            close(sfd); 
            exit(EXIT_FAILURE);
        }
        
        const char *args[4] = {UID, pass, PDIP, PDport}; 
        
        for (int i = 0; i < 4; i++) {
            cout << args[i] << endl;
            sendToServer(sfd, args[i]);
        }
    }
    //n = recvfrom(sfd, buf, BUFFER, 0, res->ai_addr, res->ai_addrlen);
    //if (n == -1)
    //    exit(1);
    //cout << buf << endl;
    close(sfd);
    exit(EXIT_SUCCESS);
}