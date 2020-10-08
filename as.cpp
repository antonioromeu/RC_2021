#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER 500
#define GN 32

using namespace std;

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int sfd, s;
    struct sockaddr_in addr;
    socklen_t addrlen;
    ssize_t n, nread;
    //struct addrinfo *result, *rp;
    char buf[BUFFER], ASport[BUFFER];
    bool verbose = false;

    if (argc < 1) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(ASport, "58032"); 
    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-p"))
            strcpy(ASport, argv[i + 1]);
        else if (!strcmp(argv[i], "-v"))
            verbose = true;
    }

    if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((s = getaddrinfo(NULL, ASport, &hints, &res) != 0))
        exit(1);

    n = bind(sfd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        cout << n << endl;
        exit(1);
    }

    while (1) {
        addrlen = sizeof(addr);
        nread = recvfrom(sfd, buf, BUFFER, 0, (struct sockaddr*) &addr, &addrlen);
        if (nread == -1)
            exit(1);
        cout << buf << endl;
        n = sendto(sfd, buf, nread, 0, (struct sockaddr*) &addr, addrlen);
        if (n == -1)
            exit(1);
        
        // n = recvfrom(sfd, buf, BUFFER, 0, (struct sockaddr*) &addr, &addrlen);
        // if (n == -1)
        //     exit(1);
            
        // write(1, "received: ", 10);
        // write(1, buf, n);addr*) &addr, &addrlen);
        // if (n == -1)
        //     exit(1);
            
        // write(1, "received: ", 10);
        // write(1, buf, n);
        //    cout << "fodeu" << endl;
        //    continue;
        //}
        //char host[NI_MAXHOST], service[NI_MAXSERV];

        //s = getnameinfo((struct sockaddr *) &addr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
        //if (s == 0)
        //    printf("Received %zd bytes from %s:%s\n", n, host, service);
        //else
        //    fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

        // if (sendto(sfd, buf, n, 0, (struct sockaddr *) &addr, addrlen) != n) {
        //     fprintf(stderr, "Error sending response\n");
        // }
        
    }
}