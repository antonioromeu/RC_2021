#include "aux.h"

fd_set readfds;
int out_fds, sfd, afd = 0, clientUDP, masterTCP, s, states[10], usersFD[10];
char PDIP[50];
char PDport[6]= "57032";
char ASIP[50] = "localhost";
char ASport[6] = "58032";
char command[128];
char UID[6];
char recvUID[6];
char pass[9];
char recvPass[9];
char buffer[1024];
char FSIP[50] = "localhost";
char FSport[6]= "59032";
char Fop[50];
char Fname[50];
char RID[5];
char VC[5];
char TID[5];
char filename[128];
char Fsize[10];
char newdir[128];
char status[5];
char base[128];
char auxbase[128];
char passFile[128];
char regFile[128];
char loginFile[128];
char tidFile[128];
char auxBuffer[128];
  
int master_socket, addrlen, new_socket, clientsFD[10], max_clients = 10, activity, i, valread, fd, maxFD;
struct sockaddr_in address;

socklen_t addrlenUDP, addrlenTCP;
struct addrinfo hintsUDP, hintsTCP, *resUDP, *resTCP;
struct sockaddr_in addrUDP, addrTCP;

void parseArgs(int argc, char *argv[]) {
    if (argc < 1 || argc > 8) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    strcpy(PDIP, argv[1]);
    for (int i = 2; i < argc; i += 2) {
        if (!strcmp(argv[i], "-q"))
            strcpy(FSport, argv[i + 1]);
        else if (!strcmp(argv[i], "-n"))
            strcpy(ASIP, argv[i + 1]);
        else if (!strcmp(argv[i], "-p"))
            strcpy(ASport, argv[i + 1]);
    }
}

void closeAllConnections() {
    close(clientUDP);
    close(masterTCP);
    for (int i = 0; i < max_clients; i++) {
        if (clientsFD[i])
            close(clientsFD[i]);
    }
    freeaddrinfo(resUDP);
    freeaddrinfo(resTCP);
}

void sendUDP(int socket, char *buf) {
    int n = 0;    
    n = sendto(socket, buf, strlen(buf), 0, resUDP->ai_addr, resUDP->ai_addrlen); 
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        closeAllConnections();
        exit(EXIT_FAILURE);
    }
    memset(buf, '\0', strlen(buf));
}

void sendTCP(int socket, char *buf) {
    int n = write(socket, buf, strlen(buf));
    memset(buf, '\0', strlen(buf));
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        closeAllConnections();
        exit(EXIT_FAILURE);
    }
}

void receiveUDP(int socket) {
    memset(buffer, '\0', strlen(buffer));
    memset(auxBuffer, '\0', strlen(auxBuffer));
    memset(command, '\0', strlen(command));
    memset(newdir, '\0', strlen(newdir));
    memset(filename, '\0', strlen(filename));
    int n = recvfrom(socket, buffer, BUFSIZE, 0, (struct sockaddr*) &addrUDP, &addrlenUDP);
    buffer[n] = '\0';
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "CNF")) {
        sscanf(buffer, "%s %s %s %s", command, UID, TID, Fop);
        const char *args[5] = {"./fsUSERS/", UID, "/", UID, "_fd.txt\0"};
        strcpy(filename, createString(args, 5));
        std::cout << filename << " filename" << std::endl;
        FILE *file;
        file = fopen(filename, "r");
        if (file == NULL) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
            return;
        }
        n = fread(auxBuffer, 1, 128, file);
        std::cout << auxBuffer << " aux" << std::endl;
        fclose(file);
        memset(buffer, '\0', strlen(buffer));
        sscanf(auxBuffer, "%s ", buffer);
        s = atoi(buffer);
        if (!strcmp(Fop, "E")) {
            const char *args1[1] = {"RLS INV\n"};
            sendTCP(s, createString(args1, 1));
        }
        else if (!strcmp(Fop, "L")) {
            const char *args1[2] = {"./fsUSERS/", UID};
            memset(auxBuffer, '\0', strlen(auxBuffer));
            strcpy(auxBuffer, createString(args1, 2));
            DIR *d = opendir(auxBuffer);
            struct dirent *dir;
            if (!d)
                perror("Could not open dir");
            char N[128];
            itoa(countFiles(UID), N, strlen(N));
            if (!strcmp(N, "0")) {
                const char *args5[1] = {"RLS EOF\n"};
                sendTCP(s, createString(args5, 1));
            }
            else {
                const char *args2[3] = {"RLS ", N, " "};
                strcpy(buffer, createString(args2, 3));
                sendTCP(s, buffer);
                while ((dir = readdir(d)) != NULL) {
                    memset(filename, '\0', strlen(filename));
                    memset(Fsize, '\0', strlen(Fsize));
                    strcpy(filename, dir->d_name);
                    file = fopen(filename, "r");
                    fseek(file, 0, SEEK_END);
                    int intFilesize = ftell(file);
                    rewind(file);
                    fclose(file);
                    itoa(intFilesize, Fsize, 10);
                    const char *args3[3] = {filename, " ", Fsize};
                    sendTCP(s, createString(args3, 3));
                }
                memset(auxBuffer, '\0', strlen(auxBuffer));
                strcpy(auxBuffer, "\n");
                sendTCP(s, auxBuffer);
            }
        }
        else {
            const char *args5[1] = {"RLS ERR\n"};
            sendTCP(s, createString(args5, 1));
        }
        memset(buffer, '\0', strlen(buffer));
        const char *args4[5] = {"./fsUSERS/", UID, "/", UID, "_fd.txt\0"};
        strcpy(filename, createString(args4, 5));
        std::cout << filename << " filename1" << std::endl;
        file = fopen(filename, "w");
        itoa(s, buffer, 128);
        fwrite(buffer, 1, 128, file);
        std::cout << buffer << " no buffer " << std::endl;
        fclose(file);
        memset(auxBuffer, '\0', strlen(auxBuffer));
    }
}

void receiveTCP(int socket) {
    memset(buffer, '\0', strlen(buffer));
    memset(command, '\0', strlen(command));
    memset(auxBuffer, '\0', strlen(auxBuffer));
    memset(filename, '\0', strlen(filename));
    int n = read(socket, buffer, BUFSIZE);
    buffer[n] = '\0';
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "LST")) {
        sscanf(buffer, "%s %s %s", command, UID, TID);
        const char *args3[3] = {"./fsUSERS/", UID, "\0"};
        strcpy(filename, createString(args3, 3));
        if (!mkdir(filename, 0777))
            std::cout << "Created ./fsUSERS/" << UID << " directory" << std::endl;
        memset(filename, '\0', strlen(filename));
        const char *args[5] = {"./fsUSERS/", UID, "/", UID, "_fd.txt\0"};
        strcpy(filename, createString(args, 5));
        itoa(socket, auxBuffer, 128);
        std::cout << "socket quando escrevemos pela primeira vez: " << socket << std::endl;
        FILE *file;
        std::cout << filename << " filename" << std::endl;
        file = fopen(filename, "w");
        if (file == NULL) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
        }
        fwrite(auxBuffer, 1, strlen(auxBuffer), file);
        std::cout << "no LST " << auxBuffer << std::endl;
        fclose(file);
        const char *args1[5] = {"VLD ", UID, " ", TID, "\n"};
        sendUDP(clientUDP, createString(args1, 5));
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    int maxFD;
    char aux[10] = "./fsUSERS";
    if (!mkdir(aux, 0777))
        printf("Created ./fsUSERS directory\n");
    
    /*------------clientUDP Socket---------*/
    clientUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientUDP == -1)
        exit(1);
    memset(&hintsUDP, 0, sizeof hintsUDP);
    hintsUDP.ai_family = AF_INET;
    hintsUDP.ai_socktype = SOCK_DGRAM;
    s = getaddrinfo(ASIP, ASport, &hintsUDP, &resUDP);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close(clientUDP);
        exit(EXIT_FAILURE);
    }
    addrlenUDP = sizeof(addrUDP);

    /*------------masterTCP Socket---------*/
    for (i = 0; i < max_clients; i++)
        clientsFD[i] = 0;
    masterTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (masterTCP == -1)
        exit(1);
    memset(&hintsTCP, 0, sizeof hintsTCP);
    hintsTCP.ai_family = AF_INET;
    hintsTCP.ai_socktype = SOCK_STREAM;
    hintsTCP.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, FSport, &hintsTCP, &resTCP);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo masterTCP : %s\n", gai_strerror(s));
        close(masterTCP);
        exit(EXIT_FAILURE);
    }
    if (bind(masterTCP, resTCP->ai_addr, resTCP->ai_addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(masterTCP, max_clients) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }


    while (1) {
        FD_ZERO(&readfds);
        FD_SET(clientUDP, &readfds);
        FD_SET(masterTCP, &readfds);
        maxFD = masterTCP;
        for (int i = 0; i < max_clients; i++) {
            fd = clientsFD[i];
            if (fd > 0)
                FD_SET(fd, &readfds);
            if (fd > maxFD)
                maxFD = fd;
        }

        maxFD = max(clientUDP, maxFD);
        activity = select(maxFD + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
            printf("select error");

        switch (activity) {
            case 0:
                printf("Timeout\n");
                break;
            case -1:
                perror("Select\n");
                exit(EXIT_FAILURE);
            default:
                if (FD_ISSET(clientUDP, &readfds)) {
                    receiveUDP(clientUDP);
                    break;
                }
                if (FD_ISSET(masterTCP, &readfds)) {
                    addrlenTCP = sizeof(addrTCP);
                    int newfd = accept(masterTCP, (struct sockaddr*) &addrTCP, &addrlenTCP);
                    if (newfd < 0) {
                        printf("server acccept failed\n");
                        exit(EXIT_FAILURE);
                    }
                    for (int i = 0; i < max_clients; i++) {
                        if (clientsFD[i] == 0) {
                            clientsFD[i] = newfd;
                            break;
                        }
                    }
                }
                for (int i = 0; i < max_clients; i++) {
                    fd = clientsFD[i];
                    if (FD_ISSET(fd, &readfds)) {
                        receiveTCP(fd);
                    }
                }
        }
    }
    return 0;
}