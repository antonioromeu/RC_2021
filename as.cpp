#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>

#define BUFFER 500
#define GN 32

using namespace std;

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t addrlen;
    struct  sockaddr_in addr;
    ssize_t n;
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

    //sfd = socket(AF_INET, SOCK_DGRAM, 0);
    //if (sfd == -1)
    //    exit(1);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */

    s = getaddrinfo("tejo.tecnico.ulisboa.pt", ASport, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // for (rp = result; rp != NULL; rp = rp->ai_next) {
    //     sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    //     if (sfd == -1)
    //         continue;
        
    //     if (bind(sfd, rp->ai_addr, rp->ai_addrlen) != 0)
    //         break;
        
    //     close(sfd);
    // }

    sfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sfd==-1) /*error*/exit(1);   
    n= bind (sfd,res->ai_addr, res->ai_addrlen);
    if(n==-1) {
        cout << "acabou" << endl;
        /*error*/ exit(1);


    if (rp == NULL) { /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           /* No longer needed */

    /* Read datagrams and echo them back to sender */

    while (1) {
        addrlen = sizeof(addr);
        n = recvfrom(sfd, buf, BUFFER, 0, (struct sockaddr*) &addr, &addrlen);
        if (n == -1)
            exit(1);
            
        write(1, "received: ", 10);
        write(1, buf, n);
        //    cout << "fodeu" << endl;
        //    continue; /* Ignore failed request */
        //}
        //char host[NI_MAXHOST], service[NI_MAXSERV];

        //s = getnameinfo((struct sockaddr *) &addr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
        //if (s == 0)
        //    printf("Received %zd bytes from %s:%s\n", n, host, service);
        //else
        //    fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s)); /* escribing an error value for the getaddrinfo() and getnameinfo() functions*/

        if (sendto(sfd, buf, n, 0, (struct sockaddr *) &addr, addrlen) != n) {
            fprintf(stderr, "Error sending response\n");
        }
    }
}