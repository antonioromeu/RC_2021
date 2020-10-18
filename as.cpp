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
char ASport[BUFFER] = "58032" ;

int create_udp(struct addrinfo hints, struct addrinfor **res){
    int fd, n;
     memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((n = getaddrinfo(NULL, ASport, &hints, res) != 0))
        exit(1);

      if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        exit(1);

    n = bind(fd, (*res)->ai_addr, (*res)->ai_addrlen);
    if (n == -1)
        exit(1);

    return fd;

}

int create_TCP(struct addrinfo hints, struct addrinfo **res) {
    int n, fd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    n = getaddrinfo(NULL, ASport, &hints, res);
    if (n != 0) exit(1);;

    fd = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if (fd == -1) exit(1);;

    n = bind(fd, (*res)->ai_addr, (*res)->ai_addrlen);
    if (n == -1) exit(1);;

    if (listen(fd, 5) == -1) exit(1);;

    return fd;
}


int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    /*int fd_tcp, fd_udp;
    struct addrinfo hints_tcp, hints_udp, *res_tcp, *res_udp;
    */
    int sfd, s;
    struct sockaddr_in addr;
    socklen_t addrlen;
    ssize_t n, nread;
    char buf[BUFFER], ASport[BUFFER];
    /*variavel global*/
    bool verbose = false;
    /*int fd_udp;*/

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
/* fd_tcp = create_TCP(hints_tcp, &res_tcp);
    fd_udp = create_UDP(hints_udp, &res_udp);
*/
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((s = getaddrinfo(NULL, ASport, &hints, &res) != 0))
        exit(1);

    n = bind(sfd, res->ai_addr, res->ai_addrlen);
    if (n == -1)
        exit(1);

    while (1) {
        addrlen = sizeof(addr);
        nread = recvfrom(sfd, buf, BUFFER, 0, (struct sockaddr*) &addr, &addrlen);
        buf[nread] = '\0';
        if (nread == -1)
            exit(1);
        cout << buf << endl;
        //n = sendto(sfd, buf, nread, 0, (struct sockaddr*) &addr, addrlen);
        //if (n == -1)
        //    exit(1);
    }
}