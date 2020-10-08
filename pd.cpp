#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#define BUFFER 500
#define PORT 58001
#define GN 32

using namespace std;

int main(int argc, char **argv) {
    struct addrinfo hints, *res;
    struct sockaddr_in addr, *result, *rp;
    socklen_t addrlen;
    int sfd, s, j;
    size_t len;
    ssize_t nread, n;
    char buf[BUFFER];
    char PDIP[BUFFER], PDport[BUFFER], ASIP[BUFFER], ASport[BUFFER];

    if (argc < 2 || argc > 8) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(PDIP, argv[1]);
    strcpy(PDport, "57032");
    strcpy(ASIP, "127.0.0.1");
    strcpy(ASport, "58032");

    for (int i = 2; i < argc; i += 2) {
        if (!strcmp(argv[i], "-d"))
            strcpy(PDport, argv[i + 1]);
        else if (!strcmp(argv[i], "-n"))
            strcpy(ASIP, argv[i + 1]);
        else if (!strcmp(argv[i], "-p"))
            strcpy(ASport, argv[i + 1]);
    }
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    cout << "antes do get" << endl;
    s = getaddrinfo(ASIP, ASport, &hints, &res);
    if (s != 0) {
        cout << "get" << endl;
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }


    /*
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        close(sfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }
    */
    //freeaddrinfo(result); /* No longer needed */

    cout << "antes do for" << endl;
    for (j = 3; j < argc; j += 2) {
        cout << "dentro do";
        len = strlen(argv[j]) + 1; /* +1 for terminating null byte */
        if (len + 1 > BUFFER) {
            fprintf(stderr, "Ignoring long message in argument %d\n", j);
            continue;
        }

        n = sendto(sfd, argv[j], len, 0, res->ai_addr, res->ai_addrlen);
        if (n == -1) {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }
    }

    //n = recvfrom(sfd, buf, BUFFER, 0, (struct sockaddr*) &addr, &addrlen);
    if (n == -1)
        exit(1);

    exit(EXIT_SUCCESS);
}