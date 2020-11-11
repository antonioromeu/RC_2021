#include "aux.h"

int out_fds, sfd, afd = 0, clientUDP, serverUDP, s;
char PDIP[50];
char PDport[6]= "57032";
char ASIP[50] = "localhost";
char ASport[6] = "58032";
char command[128];
char UID[6];
char recvUID[6];
char pass[9];
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
char status[4];

socklen_t addrlenClient, addrlenServer;
struct addrinfo hintsClient, hintsServer, *resClient, *resServer;
struct sockaddr_in addrClient, addrServer;

fd_set readfds;

void parseArgs(int argc, char *argv[]) {
    if (argc < 2 || argc > 8) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    strcpy(PDIP, argv[1]);
    for (int i = 2; i < argc; i += 2) {
        if (!strcmp(argv[i], "-d"))
            strcpy(PDport, argv[i + 1]);
        else if (!strcmp(argv[i], "-n"))
            strcpy(ASIP, argv[i + 1]);
        else if (!strcmp(argv[i], "-p"))
            strcpy(ASport, argv[i + 1]);
    }
}

void closeAllConnections() {
    close(clientUDP);
    close(serverUDP);
    freeaddrinfo(resClient);
    freeaddrinfo(resServer);
}

void sendToServer(int socket, char *buf) {
    int n = 0;
    if (socket == clientUDP)
        n = sendto(socket, buf, strlen(buf), 0, resClient->ai_addr, resClient->ai_addrlen);
    else if (socket == serverUDP)
        n = sendto(socket, buf, strlen(buf), 0, (struct sockaddr*) &addrServer, addrlenServer);
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        closeAllConnections();
        exit(EXIT_FAILURE);
    }
    memset(buf, '\0', strlen(buf));
}

void processASAnswer(char *buf) {
    sscanf(buf, "%s ", command);
    if (!strcmp(command, "VLC")) {
        sscanf(buf, "%s %s %s %s", command, recvUID, VC, Fop);
        if (Fop[0] == 'R' || Fop[0] == 'U' || Fop[0] == 'D')
            sscanf(Fop, "%s %s", Fop, Fname);
        if (!strcmp(UID, recvUID)) {
            std::cout << "Validaton code: " << VC << std::endl;
            const char *args[3] = {"RVC ", UID, " OK\n"};
            sendToServer(serverUDP, createString(args, 3));
        }
        else if (strcmp(UID, recvUID)) {
            std::cout << "Validaton: invalid user ID" << std::endl;
            const char *args[3] = {"RVC ", UID, " NOK\n"};
            sendToServer(serverUDP, createString(args, 3));
        }
    }
}

char *receiveFromSocket(int socket) {
    int n = 0;
    memset(buffer, '\0', strlen(buffer));
    memset(command, '\0', strlen(command));
    memset(status, '\0', strlen(status));
    if (socket == clientUDP) {
        n = recvfrom(socket, buffer, BUFSIZE, 0, (struct sockaddr*) &addrClient, &addrlenClient);
        buffer[n] = '\0';
        std::cout << buffer << " no clientUDP" << std::endl;
        sscanf(buffer, "%s ", command);
        if (!strcmp(command, "RRG")) {
            sscanf(buffer, "%s %s", command, status);
            if (!strcmp(status, "OK"))
                std::cout << "Registration: successful" << std::endl;
            else if (!strcmp(status, "NOK"))
                std::cout << "Registration: not accepted" << std::endl;
        }
        if (!strcmp(command, "RVC")) {
            sscanf(buffer, "%s %s %s", command, UID, status);
            if (!strcmp(status, "OK"))
                std::cout << "Validation: valid user" << std::endl;
            else if (!strcmp(status, "NOK"))
                std::cout << "Validation: invalid user" << std::endl;
        }
        if (!strcmp(command, "RUN")) {
            sscanf(buffer, "%s %s", command, status);
            if (!strcmp(status, "OK")) {
                std::cout << "Unregister: successful" << std::endl;
                close(clientUDP);
                close(serverUDP);
                exit(EXIT_SUCCESS);
            }
            if (!strcmp(status, "NOK"))
                std::cout << "Unregister: not accepted" << std::endl;
        }
    }
    if (socket == serverUDP) {
        n = recvfrom(socket, buffer, BUFSIZE, 0, (struct sockaddr*) &addrServer, &addrlenServer);
        buffer[n] = '\0';
        std::cout << buffer << " no serverUDP" << std::endl;
        if (n != -1)
            processASAnswer(buffer);
    }
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        closeAllConnections();
        exit(EXIT_FAILURE);
    }
    return buffer;
}

void processCommands() {
    fgets(buffer, 50, stdin);
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "exit")) {
        const char *args[5] = {"UNR ", UID, " ", pass, "\n"};
        sendToServer(clientUDP, createString(args, 5));
    }
    else if (!strcmp(command, "reg")) {
        sscanf(buffer, "%s %s %s", command, UID, pass);
        if (!checkUID(UID))
            std::cout << "Register: invalid UID" << std::endl;
        else if (!checkPass(pass))
            std::cout << "Register: invalid password" << std::endl;
        else {
            const char *args[9] = {"REG ", UID, " ", pass, " ", PDIP, " ", PDport, "\n"};
            sendToServer(clientUDP, createString(args, 9));
        }
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    int maxfd;
    
    clientUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientUDP == -1)
        exit(1);
    memset(&hintsClient, 0, sizeof hintsClient);
    hintsClient.ai_family = AF_INET;
    hintsClient.ai_socktype = SOCK_DGRAM;
    s = getaddrinfo(ASIP, ASport, &hintsClient, &resClient);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        closeAllConnections();
        exit(EXIT_FAILURE);
    }
    addrlenClient = sizeof(addrClient);

    serverUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverUDP == -1)
        exit(1);
    memset(&hintsServer, 0, sizeof hintsServer);
    hintsServer.ai_family = AF_INET;
    hintsServer.ai_socktype = SOCK_DGRAM;
    hintsServer.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, PDport, &hintsServer, &resServer);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        closeAllConnections();
        exit(EXIT_FAILURE);
    }
    addrlenServer = sizeof(addrServer);
    
    if (bind(serverUDP, resServer->ai_addr, resServer->ai_addrlen) < 0 ) {
        perror("Bind failed");
        closeAllConnections();
        exit(EXIT_FAILURE);
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(afd, &readfds); // i.e reg 92427 ...
        FD_SET(clientUDP, &readfds); // i.e REG OK
        FD_SET(serverUDP, &readfds); // i.e VLC 9999
        maxfd = max(clientUDP, serverUDP);
        out_fds = select(maxfd + 1, &readfds, (fd_set *) NULL, (fd_set *) NULL, NULL);
        switch (out_fds) {
            case 0:
                printf("Timeout\n");
                break;
            case -1:
                perror("Select\n");
                exit(EXIT_FAILURE);
            default:
                if (FD_ISSET(afd, &readfds)) {
                    processCommands();
                    break;
                }
                if (FD_ISSET(clientUDP, &readfds)) {
                    receiveFromSocket(clientUDP);
                    break;
                }
                if (FD_ISSET(serverUDP, &readfds)) {
                    receiveFromSocket(serverUDP);
                    break;
                }
        }
    }
}