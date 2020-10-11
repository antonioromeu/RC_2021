#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <ctype.h>

#define BUFFER 500
#define GN 32

using namespace std;

int fd, errcode, s;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
char buffer[128];


char ASIP[50] = "localhost";
char ASport[6] = "58032";
char FSIP[50] = "localhost";
char FSport[6]= "59032";
char command[5] = "";
char UID[6] = "";
char pass[9] = "";

void parseArgs(int argc, char *argv[]){
    int n = argc, i = 1;
    if (argc < 1 || argc > 9) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    for (int i = 2; i < argc; i += 2) {
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

int main(int argc, char **argv) {

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) 
        exit(1);
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    s = getaddrinfo(ASIP, ASport, &hints, &res);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1)
        exit(1);

    cout << "conectou" << endl;
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

    freeaddrinfo(res);
    close(fd);
}