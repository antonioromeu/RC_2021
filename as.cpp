#include "aux.h"

int afd = 0, UDP, TCP, s;
char newdir[12] = "USERS/";
char passFile[50];
socklen_t addrlenUDP, addrlenTCP;
struct addrinfo hintsUDP, hintsTCP, *resUDP, *resTCP;
struct sockaddr_in addrUDP, addrTCP;

void parseArgs(int argc, char *argv[]) {
    if (argc < 1 || argc > 9) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-p"))
            strcpy(ASport, argv[i]);
    }
}

void receiveUDP(int socket) {
    int nRead;
    memset(buffer, '\0', strlen(buffer));
    nRead = recvfrom(socket, buffer, BUFSIZE, 0, (struct sockaddr*) &addrUDP, &addrlenUDP);
    buffer[nRead] = '\0';
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "REG")) {
        sscanf(buffer, "%s %s %s %s %s", command, UID, pass, PDIP, PDport);
        if (!checkDir(UID)) {
            strcat(newdir, UID);
            strcat(newdir, "\0");
            if (!mkdir(newdir, 0777))
                cout << "USERS/" << UID << " directory created" << endl;
            else {
                cout << "Unable to create USERS/" << UID << " directory" << endl;
                close(socket);
                return;
            }
        }
        strcpy(passFile, newdir);
        strcat(passFile, "/");
        strcat(passFile, UID);
        strcat(passFile, "_pass.txt");
        FILE *UIDFile = fopen(passFile, "w");
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    int maxfd;
    int check = mkdir("USERS", 0777); 
    if (!check)
        cout << "USERS directory created" << endl;
    else {
        cout << "Unable to create USERS directory" << endl;
        exit(EXIT_FAILURE);
    }
    /*------------UDP Socket---------*/
    UDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (UDP == -1)
        exit(1);
    memset(&hintsUDP, 0, sizeof hintsUDP);
    hintsUDP.ai_family = AF_INET;
    hintsUDP.ai_socktype = SOCK_DGRAM;
    hintsUDP.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, ASport, &hintsUDP, &resUDP);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close(UDP);
        exit(EXIT_FAILURE);
    }
    addrlenUDP = sizeof(addrUDP);
    
    if (bind(UDP, resUDP->ai_addr, resUDP->ai_addrlen) < 0 ) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    /*------------TCP Socket---------*/
   /* TCP = socket(AF_INET, SOCK_STREAM, 0);
    if (TCP == -1)
        exit(1);
    memset(&hintsTCP, 0, sizeof hintsTCP);
    hintsTCP.ai_family = AF_INET;
    hintsTCP.ai_socktype = SOCK_STREAM;
    hintsTCP.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, ASport, &hintsTCP, &resTCP);
    if (s != 0) {
        cout << "a seguir" << endl;
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close(TCP);
        exit(EXIT_FAILURE);
    }
    addrlenTCP = sizeof(addrTCP);

    if (bind(TCP, resTCP->ai_addr, resTCP->ai_addrlen) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }*/

    while (1) {
        timeout.tv_sec = 120;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(UDP, &readfds);
        FD_SET(TCP, &readfds);
        maxfd = max(UDP, TCP);
        out_fds = select(maxfd + 1, &readfds, (fd_set *) NULL, (fd_set *) NULL, &timeout);
        switch (out_fds) {
            case 0:
                printf("Timeout\n");
                break;
            case -1:
                perror("Select\n");
                exit(EXIT_FAILURE);
            default:
                if (FD_ISSET(UDP, &readfds)) {
                    receiveUDP(UDP);
                    break;
                }
                if (FD_ISSET(TCP, &readfds)) {
                    // receiveFromServer(TCP);
                    break;
                }
        }
    }
}

/*
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
    int fd_tcp, fd_udp;
    struct addrinfo hints_tcp, hints_udp, *res_tcp, *res_udp;
    int sfd, s;
    struct sockaddr_in addr;
    socklen_t addrlen;
    ssize_t n, nread;
    char buf[BUFFER], ASport[BUFFER];
    variavel global
    bool verbose = false;
    int fd_udp;

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
    fd_tcp = create_TCP(hints_tcp, &res_tcp);
    fd_udp = create_UDP(hints_udp, &res_udp);
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
*/