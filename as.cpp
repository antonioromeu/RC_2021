/*
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT "58011"

int fd, newfd, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
char buffer[128];

int main(void) {
    fd = socket(AF_INET, SOCK_STREAM, 0); //TCP socket
    if (fd == -1) 
        exit(1); //Error

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP socket
    hints.ai_flags = AI_PASSIVE;

    errcode = getaddrinfo(NULL, PORT, &hints, &res);
    if ((errcode) != 0) //Error
        exit(1);

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) //Error 
        exit(1);

    if (listen(fd, 5) == -1) //Error
        exit(1);

    while (1) {
        addrlen = sizeof(addr);
        if ((newfd = accept(fd, (struct sockaddr*) &addr, &addrlen)) == -1) //Error 
            exit(1);
        
        n = read(newfd,buffer,128);
        if (n == -1) //Error
            exit(1);
        
        write(1,"received: ", 10);
        write(1, buffer, n);
        
        n = write(newfd, buffer, n);
        if(n == -1) //Error
            exit(1);
        
        close(newfd);
    }
    
    freeaddrinfo(res); 
    close(fd);
}
*/

#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    char in_str[128];
    fd_set inputs, testfds;
    struct timeval timeout;
    int i, out_fds, n;
    FD_ZERO(&inputs); // Clear inputs
    FD_SET(0, &inputs); // Set standard input channel on
    printf("Size of fd_set: %d\n", sizeof(fd_set));  
    printf("Value of FD_SETSIZE: %d\n", FD_SETSIZE);
    while(1) {
        testfds = inputs;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        printf("testfds byte: %d\n", ((char *)&testfds)[0]);
        out_fds = select(FD_SETSIZE,&testfds, (fd_set *) NULL, (fd_set *)NULL, &timeout);
        printf("Time = %d and %d\n", timeout.tv_sec, timeout.tv_usec);
        printf("testfds byte: %d\n", ((char *)&testfds)[0]);
        switch(out_fds) {
            case 0:
                printf("Timeout event\n");
                break;
            case -1:
                perror("select");
                exit(1);
            default:
                if(FD_ISSET(0, &testfds)) {
                    if ((n=read(0,in_str,127))!=0) {
                        if(n == -1)
                            exit(1);
                        in_str[n] = 0;
                        printf("From keyboard: %s\n",in_str);
                    }
                }
        }
    }
}