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

int fd, errcode, s;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
char buffer[128] = "";
char ASIP[50] = "localhost";
char ASport[6] = "58032";
char FSIP[50] = "localhost";
char FSport[6]= "59032";
char command[5] = "";
char UID[6] = "";
char pass[9] = "";
char Fop[] = "";
char Fname[] = "";
char VC[] = "";
char filename = "";

void parseArgs(int argc, char *argv[]){
    if (argc < 1 || argc > 9) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-n"))
            strcpy(ASIP, argv[i + 1]);
        else if (!strcmp(argv[i], "-p"))
            strcpy(ASport, argv[i + 1]);
        else if (!strcmp(argv[i], "-m"))
            strcpy(FSIP, argv[i + 1]);
        else if (!strcmp(argv[i], "-q"))
            strcpy(FSport, argv[i + 1]);
    }
}

bool isNumeric(char *str) {
    for (int i = 0; i < str.length(); i++)
        if (isdigit(str[i]) == false)
            return false;
    return true;
}

bool isAlphanumeric(char *str) {
    for (int i = 0; i < str.length(); i++)
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
    if (sendto(sfd, &buf, buf.length(), 0, res->ai_addr, res->ai_addrlen) == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd); 
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) 
        exit(EXIT_FAILURE);
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    s = getaddrinfo(ASIP, ASport, &hints, &res);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Nao conectou");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        char str[50];
        cin.getline(str, 50);
        if (!strcmp(command, "exit")) {

        }
        else if (!strcmp(command, "login")) {
            sscanf(str, "%s %s %s", command, UID, pass);
            if (!checkUID(UID))
                exit(EXIT_FAILURE);
            if (!checkPass(pass))
                exit(EXIT_FAILURE);
            sendToServer(fd, UID);
            sendToServer(fd, pass);
        }
        else if (!strcmp(command, "req")) {
            
        }
        else if (!strcmp(command, "val")) {
            
        }
        else if (!strcmp(command, "list")) {
            
        }
        else if (!strcmp(command, "l-")) {
            
        }
        else if (!strcmp(command, "retrieve")) {
            
        }
        else if (!strcmp(command, "r")) {
            
        }
        else if (!strcmp(command, "upload")) {
            
        }
        else if (!strcmp(command, "u")) {
            
        }
        else if (!strcmp(command, "delete")) {
            
        }
        else if (!strcmp(command, "d")) {
            
        }
        else if (!strcmp(command, "remove")) {
            
        }
        else if (!strcmp(command, "x")) {
            
        }
        sscanf(str, "%s ", command);
        switch (command) {
            case 'req':
            case 'val':
            case 'list':
            case 'l-':
            case 'retrieve':
            case 'r':
            case 'upload':
            case 'u':
            case 'delete':
            case 'd':
            case 'remove':
            case 'x':
            case 'exit':
        }
        
        const char *args[4] = {UID, pass, PDIP, PDport}; 
        
        for (int i = 0; i < 4; i++) {
            cout << args[i] << endl;
            sendToServer(sfd, args[i]);
        }
    }

    /*
    n = write(fd, "Hello!\n", 7);
    if(n == -1)
        exit(1);

    n = read(fd, buffer, 128);
    if(n == -1) 
        exit(1);

    write(1, "echo: ", 6); 
    write(1, buffer, n);
    */

    //freeaddrinfo(res);
    close(fd);
}