#include "aux.h"

int afd = 0, clientUDP, serverUDP;
socklen_t addrlenClient, addrlenServer;
struct addrinfo hintsClient, hintsServer, *resClient, *resServer;
struct sockaddr_in addrClient, addrServer;

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

void sendToServer(char *buf) {
    if (sendto(clientUDP, buf, strlen(buf), 0, resClient->ai_addr, resClient->ai_addrlen) == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(clientUDP); 
        exit(EXIT_FAILURE);
    }
    cout << buf << endl;
    strcpy(buf, "\0");
}

int receiveFromServer() {
    int n = recvfrom(serverUDP, receiverBuf, BUFFER, 0, (struct sockaddr*) &addrServer, &addrlenServer);
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(serverUDP); 
        exit(EXIT_FAILURE);
    }
    cout << receiverBuf << endl;
    strcpy(receiverBuf, "\0");
    return n;
}

void processCommands() {
    fgets(str, 50, stdin);
    sscanf(str, "%s ", command);
    if (!strcmp(command, "exit")) {
        const char *args[5] = {"UNR", " ", UID, " ", pass};
        sendToServer(createString(args, 5));
        close(clientUDP);
        close(serverUDP);
        exit(EXIT_SUCCESS);
    }
    else if (!strcmp(command, "reg")) {
        sscanf(str, "%s %s %s", command, UID, pass);
        if (!checkUID(UID) || !checkPass(pass)) {
            close(clientUDP);
            close(serverUDP);
            exit(EXIT_FAILURE);
        }
        const char *args[10] = {"REG", " ", UID, " ", pass, " ", PDIP, " ", PDport, "\n"};
        sendToServer(createString(args, 10));
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    
    clientUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientUDP == -1)
        exit(1);
    memset(&hintsClient, 0, sizeof hintsClient);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    s = getaddrinfo(ASIP, ASport, &hintsClient, &resClient);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close(clientUDP);
        exit(EXIT_FAILURE);
    }
    addrlenClient = sizeof(addrClient);

    serverUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverUDP == -1)
        exit(1);
    memset(&hintsServer, 0, sizeof hintsServer);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    s = getaddrinfo(NULL, ASport, &hintsServer, &resServer);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close(serverUDP);
        exit(EXIT_FAILURE);
    }
    addrlenServer = sizeof(addrServer);
    
    if (bind(serverUDP, (const sockaddr*) &hintsServer, addrlenServer) < 0 ) { 
        perror("Bind failed"); 
        exit(EXIT_FAILURE); 
    }

    while (1) {
        timeout.tv_sec = 120;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(afd, &readfds); // i.e reg 92427 ...
        FD_SET(clientUDP, &readfds); // i.e REG OK
        FD_SET(serverUDP, &readfds); // VLC 9999
        maxfd = max(afd, clientUDP);
        maxfd = max(maxfd, serverUDP);
        out_fds = select(maxfd + 1, &readfds, (fd_set *) NULL, (fd_set *) NULL, &timeout);
        switch (out_fds) {
            case 0:
                printf("Timeout\n");
                break;
            case -1:
                perror("Select\n");
                exit(EXIT_FAILURE);
            default:
                cout << out_fds << endl;
                for (int i = 0; i <= out_fds; i++) {
                    if (FD_ISSET(afd, &readfds)) {
                        FD_CLR(afd, &readfds);
                        processCommands();
                        cout << "afd" << endl;
                        break;
                    }
                    else if (FD_ISSET(clientUDP, &readfds)) {
                        FD_CLR(clientUDP, &readfds);
                        receiveFromServer();
                        cout << "client" << endl;
                        break;
                    }
                    /*if (FD_ISSET(serverUDP, &readfds)) {
                        receiveFromServer();
                        cout << "server" << endl;
                        break;
                    }*/
                }
        }
    }
}