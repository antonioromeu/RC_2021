#include "aux.h"

fd_set readfds;
int out_fds, sfd, afd = 0, PDServerUDP, clientUDP, masterTCP, FSServerUDP, s, newSocket;
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
char recvVC[5];
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
char fdFile[128];
char auxBuffer[128];
char states[10][128];
  
int master_socket, addrlen, new_socket, clientsFD[10], max_clients = 10, activity, i, valread, fd, maxFD;
struct sockaddr_in address;

socklen_t addrlenUDP, addrlenClientUDP, addrlenFSServerUDP, addrlenTCP;
struct addrinfo hintsUDP, hintsClientUDP, hintsFSServerUDP, hintsTCP, *resUDP, *resFSServerUDP, *resClientUDP, *resTCP;
struct sockaddr_in addrUDP, addrClientUDP, addrFSServerUDP, addrTCP;

void parseArgs(int argc, char *argv[]) {
    if (argc < 1 || argc > 9) {
        printf("Usage: %s host port msg...", argv[0]);
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-p"))
            strcpy(ASport, argv[i]);
    }
}

void closeAllConnections() {
    close(clientUDP);
    close(PDServerUDP);
    close(FSServerUDP);
    close(masterTCP);
    for (int i = 0; i < max_clients; i++) {
        if (clientsFD[i])
            close(clientsFD[i]);
    }
    freeaddrinfo(resClientUDP);
    freeaddrinfo(resFSServerUDP);
    freeaddrinfo(resUDP);
    freeaddrinfo(resTCP);
}

void sendUDP(int socket, char *buf) {
    printf("a mandar para udp: %s\n", buf);
    int n = 0;
    if (socket == PDServerUDP) {
        n = sendto(socket, buf, strlen(buf), 0, (struct sockaddr*) &addrUDP, addrlenUDP);
    }
    if (socket == FSServerUDP) {
        n = sendto(socket, buf, strlen(buf), 0, (struct sockaddr*) &addrFSServerUDP, addrlenFSServerUDP);
    }
    else if (socket == clientUDP) {
        n = sendto(socket, buf, strlen(buf), 0, resClientUDP->ai_addr, resClientUDP->ai_addrlen);
    }
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        closeAllConnections();
        exit(EXIT_FAILURE);
    }
    memset(buf, '\0', strlen(buf));
}

void sendTCP(int socket, char *buf) {
    int n = write(socket, buf, strlen(buf));
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        closeAllConnections();
        exit(EXIT_FAILURE);
    }
    memset(buf, '\0', strlen(buf));
}

void setupClientUDP(char *PDIP, char *PDport) {
    clientUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientUDP == -1)
        exit(1);
    memset(&hintsClientUDP, 0, sizeof hintsClientUDP);
    hintsClientUDP.ai_family = AF_INET;
    hintsClientUDP.ai_socktype = SOCK_DGRAM;
    hintsClientUDP.ai_flags = AI_PASSIVE;
    s = getaddrinfo(PDIP, PDport, &hintsClientUDP, &resClientUDP);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo AS: %s\n", gai_strerror(s));
        closeAllConnections();
        exit(EXIT_FAILURE);
    }
    FD_SET(clientUDP, &readfds);
    maxFD = max(maxFD, clientUDP);
}

void receiveClientUDP(int socket) {
    memset(buffer, '\0', strlen(buffer));
    memset(command, '\0', strlen(command));
    memset(base, '\0', strlen(base));
    memset(newdir, '\0', strlen(base));
    int n = recvfrom(socket, buffer, BUFSIZE, 0, (struct sockaddr*) &addrClientUDP, &addrlenClientUDP);
    buffer[n] = '\0';
    std::cout << buffer << "buffer receiveUDP " << buffer << std::endl;
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "RVC")) {
        sscanf(buffer, "%s %s %s", command, UID, status);
        if (!strcmp(status, "OK")) {
            const char *args[3] = {"RRQ ", status, "\n"};
            strcpy(auxBuffer, createString(args, 3));
        }
        else if (!strcmp(status, "NOK")) {
            const char *args[3] = {"RRQ ", status, "\n"};
            strcpy(auxBuffer, createString(args, 3));
        }
        memset(buffer, '\0', strlen(buffer));
        const char *args[5] = {"USERS/", UID, "/", UID, "_fd.txt"};
        strcpy(filename, createString(args, 5));
        FILE *file = fopen(filename, "r");
        fread(buffer, 1, 128, file);
        fclose(file);
        s = atoi(buffer);
        sendTCP(s, auxBuffer);
        memset(auxBuffer, '\0', strlen(auxBuffer));
    }
    if (!strcmp(command, "VLD")) {
        sscanf(buffer, "%s %s %s", command, UID, TID);
        printf("TID: %s\n", TID);
        if (!strcmp(TID, "0")) {
            printf("o tid é 0\n");
            strcpy(Fop, "E");
            const char *args[7] = {"CNF ", UID, " ", TID, " ", Fop, "\n"};
            sendUDP(socket, createString(args, 7));
            return;
        }
        memset(buffer, '\0', strlen(buffer));
        memset(auxBuffer, '\0', strlen(auxBuffer));
        const char *args[5] = {"USERS/", UID, "/", UID, "_fd.txt"};
        strcpy(filename, createString(args, 5));
        FILE *file = fopen(filename, "r");
        fread(buffer, 1, 128, file);
        fclose(file);
        sscanf(buffer, "%s %s", auxBuffer, Fop);
        itoa(s, auxBuffer, strlen(auxBuffer));
        memset(auxBuffer, '\0', strlen(auxBuffer));
        if (Fop[0] == 'R' || Fop[0] == 'U' || Fop[0] == 'D') {
            sscanf(Fop, "%s %s", Fop, Fname);
            const char *args1[9] = {"CNF ", UID, " ", TID, " ", Fop, " ", Fname, "\n"};
            sendUDP(socket, createString(args1, 9));
        }
        else if (!strcmp(Fop, "X") || !strcmp(Fop, "L")) {
            const char *args1[8] = {"CNF ", UID, " ", TID, " ", Fop, " ", "\n"};
            sendUDP(socket, createString(args1, 8));
        }
        memset(buffer, '\0', strlen(buffer));
        memset(auxBuffer, '\0', strlen(auxBuffer));
        file = fopen(filename, "w");
        itoa(s, auxBuffer, strlen(auxBuffer));
        strcpy(buffer, auxBuffer);
        strcat(buffer, "\0");
        fwrite(buffer, 1, 128, file);
        fclose(file);
        memset(auxBuffer, '\0', strlen(auxBuffer));
    }
}

void receiveServerUDP(int socket) {
    memset(buffer, '\0', strlen(buffer));
    memset(command, '\0', strlen(command));
    memset(base, '\0', strlen(base));
    memset(newdir, '\0', strlen(base));
    int n = recvfrom(socket, buffer, BUFSIZE, 0, (struct sockaddr*) &addrUDP, &addrlenUDP);
    buffer[n] = '\0';
    printf("no receive server udp, recebe %s\n", buffer);
    std::cout << buffer << "buffer serverUDP " << buffer << std::endl;
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "REG")) {
        sscanf(buffer, "%s %s %s %s %s", command, UID, pass, PDIP, PDport);
        strcat(pass, "\0");
        strcat(PDIP, "\0");
        strcat(PDport, "\0");
        setupClientUDP(PDIP, PDport);
        if (!checkDir(UID)) {
            const char *args[3] = {"USERS/", UID, "\0"};
            strcpy(newdir, createString(args, 3));
            if (!opendir(newdir)) {
                mkdir(newdir, 0777);
                printf("./USERS/%s directory created\n", UID);
                strncpy(base, newdir, 13);
                const char *args2[4] = {base, "/", UID, "_pass.txt\0"};
                strcpy(passFile, createString(args2, 4));
                FILE *file;
                file = fopen(passFile, "w");
                fwrite(pass, 1, strlen(pass), file);
                fclose(file);
                
                const char *args3[4] = {base, "/", UID, "_reg.txt\0"};
                strcpy(regFile, createString(args3, 4));
                const char *args4[3] = {PDIP, " ", PDport};
                strcpy(buffer, createString(args4, 3));
                file = fopen(regFile, "w");
                fwrite(buffer, 1, strlen(buffer), file);
                fclose(file);

                strcat(status, "OK\n");
            }
            else {
                printf("Unable to create ./USERS/%s directory\n", UID);
                strcat(status, "NOK\n");
            }
        }
        strcat(auxBuffer, "RRG ");
        strcat(auxBuffer, status);
    }
    if (!strcmp(command, "UNR")) {
        memset(status, '\0', strlen(status));
        sscanf(buffer, "%s %s %s", command, UID, pass);
        if (checkDir(UID)) {
            const char *args[5] = {"./USERS/", UID, "/", UID, "_reg.txt\0"};
            strcpy(newdir, createString(args, 5));
            remove(newdir);
            strcat(status, "OK\n");
        }
        else {
            printf("Unable to remove USERS/%s/%s_reg.txt\n", UID, UID);
            strcat(status, "NOK\n");
        }
        strcat(auxBuffer, "RUN ");
        strcat(auxBuffer, status);
    }
    sendUDP(PDServerUDP, auxBuffer);
    memset(auxBuffer, '\0', strlen(auxBuffer));
}

void receiveTCP(int socket) {
    memset(buffer, '\0', strlen(buffer));
    memset(command, '\0', strlen(command));
    int n = read(socket, buffer, BUFSIZE);
    buffer[n] = '\0';
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "LOG")) {
        sscanf(buffer, "%s %s %s", command, recvUID, recvPass);
        memset(status, '\0', strlen(status));
        if (!strcmp(UID, recvUID) && !strcmp(pass, recvPass)) {
            const char *args[5] = {"USERS/", UID, "/", UID, "_login.txt\0"};
            strcpy(loginFile, createString(args, 5));
            FILE *file;
            file = fopen(loginFile, "w");
            fclose(file);
            strcpy(status, "OK");

            const char *args1[4] = {base, "/", UID, "_fd.txt\0"};
            strcpy(fdFile, createString(args1, 4));
            itoa(socket, buffer, 128);
            file = fopen(fdFile, "w");
            fwrite(buffer, 1, strlen(buffer), file);
            fclose(file);
        }
        else if (strcmp(UID, recvUID) || strcmp(pass, recvPass))
            strcpy(status, "NOK");
        else
            strcpy(status, "ERR");
        const char *args1[3] = {"RLO ", status, "\n"};
        sendTCP(socket, createString(args1, 3));
    }
    if (!strcmp(command, "REQ")) {
        setupClientUDP(PDIP, PDport);
        srand(time(0));
        sprintf(VC, "%d", rand() % 9000 + 1000);
        sscanf(buffer, "%s %s %s %s", command, UID, RID, Fop);
        const char *args1[4] = {base, "/", UID, "_fd.txt\0"};
        strcpy(fdFile, createString(args1, 4));
        FILE *file = fopen(fdFile, "a");
        memset(auxBuffer, '\0', strlen(auxBuffer));
        if (Fop[0] == 'R' || Fop[0] == 'U' || Fop[0] == 'D') {
            sscanf(buffer, "%s %s %s %s %s", command, UID, RID, Fop, Fname);
            const char *args[9] = {"VLC ", UID, " ", VC, " ", Fop, " ", Fname, "\n"};
            sendUDP(clientUDP, createString(args, 9));
            const char *args2[5] = {" ", Fop, " ", Fname, "\0"};
            strcpy(auxBuffer, createString(args2, 5));
            fwrite(auxBuffer, 1, strlen(auxBuffer), file);
        }
        else if (!strcmp(Fop, "L") || !strcmp(Fop, "X")) {
            const char *args[7] = {"VLC ", UID, " ", VC, " ", Fop, "\n"};
            sendUDP(clientUDP, createString(args, 7));
            const char *args2[3] = {" ", Fop, "\0"};
            strcpy(auxBuffer, createString(args2, 3));
            fwrite(auxBuffer, 1, strlen(auxBuffer), file);
        }
        memset(auxBuffer, '\0', strlen(auxBuffer));
        fclose(file);
    }
    if (!strcmp(command, "AUT")) {
        srand(time(0));
        sprintf(TID, "%d", rand() % 9000 + 1000);
        sscanf(buffer, "%s %s %s %s", command, UID, RID, recvVC);
        printf("VC: %s, recvVC: %s, TID: %s\n", VC, recvVC, TID);
        if (!strcmp(VC, recvVC)) {
            const char *args[3] = {"RAU ", TID, "\n"};
            sendTCP(socket, createString(args, 3));
        }
        else {
            const char *args[1] = {"RAU 0\n"};
            sendTCP(socket, createString(args, 1));
        }
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    int maxFD;
    char aux[6] = "USERS";
    if (!mkdir(aux, 0777))
        printf("Created ./USERS directory\n");
    
    /*------------PDServerUDP Socket---------*/
    PDServerUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (PDServerUDP == -1)
        exit(1);
    memset(&hintsUDP, 0, sizeof hintsUDP);
    hintsUDP.ai_family = AF_INET;
    hintsUDP.ai_socktype = SOCK_DGRAM;
    hintsUDP.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, ASport, &hintsUDP, &resUDP);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo AS: %s\n", gai_strerror(s));
        close(PDServerUDP);
        exit(EXIT_FAILURE);
    }
    addrlenUDP = sizeof(addrUDP);
    if (bind(PDServerUDP, resUDP->ai_addr, resUDP->ai_addrlen) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    /*------------FSServerUDP Socket---------*/
    FSServerUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (FSServerUDP == -1)
        exit(1);
    memset(&hintsFSServerUDP, 0, sizeof hintsFSServerUDP);
    hintsFSServerUDP.ai_family = AF_INET;
    hintsFSServerUDP.ai_socktype = SOCK_DGRAM;
    hintsFSServerUDP.ai_flags = AI_PASSIVE;
    s = getaddrinfo(FSIP, FSport, &hintsFSServerUDP, &resFSServerUDP);
    // s = getaddrinfo(NULL, FSport, &hintsFSServerUDP, &resFSServerUDP);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close(FSServerUDP);
        exit(EXIT_FAILURE);
    }
    addrlenFSServerUDP = sizeof(addrFSServerUDP);
    if (bind(FSServerUDP, resFSServerUDP->ai_addr, resFSServerUDP->ai_addrlen) < 0 ) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

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
    s = getaddrinfo(NULL, ASport, &hintsTCP, &resTCP);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo masterTCP : %s\n", gai_strerror(s));
        close(masterTCP);
        exit(EXIT_FAILURE);
    }
    // if (bind(masterTCP, (struct sockaddr *) &addrTCP, addrlenTCP) < 0) {
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
        FD_SET(PDServerUDP, &readfds);
        FD_SET(FSServerUDP, &readfds);
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

        maxFD = max(PDServerUDP, maxFD);
        maxFD = max(FSServerUDP, maxFD);
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
                if (FD_ISSET(FSServerUDP, &readfds)) {
                    printf("a receber fs\n");
                    receiveClientUDP(FSServerUDP);
                    break;
                }
                if (FD_ISSET(PDServerUDP, &readfds)) {
                    printf("no pd server\n");
                    receiveServerUDP(PDServerUDP);
                    printf("a sair da funcao receive Server udp\n");
                    break;
                }
                if (FD_ISSET(clientUDP, &readfds)) {
                    printf("no pd client\n");
                    receiveClientUDP(clientUDP);
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
                        printf("tcp\n");
                        receiveTCP(fd);
                    }
                }
        }
    }
    return 0;
}