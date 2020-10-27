#include "aux.h"

int afd = 0, ASClientTCP, FSClientTCP;
struct addrinfo hintsASClient, hintsFSClient, *resASClient, *resFSClient;
struct sockaddr_in addrClient, addrServer;

void parseArgs(int argc, char *argv[]){
    if (argc < 1 || argc > 9) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-n"))
            strcpy(ASIP, argv[i + 1]);
        else if (!strcmp(argv[i], "-p"))
            strcpy(ASport, argv[i + 1]);
        else if (!strcmp(argv[i], "-m"))
            strcpy(FSIP, argv[i + 1]);
        else if (!strcmp(argv[i], "-q"))
            strcpy(FSport, argv[i + 1]);
    }
}

void sendToServer(int sfd, char *buf) {
    if (write(sfd, buf, strlen(buf)) == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd); 
        exit(EXIT_FAILURE);
    }
    cout << buf << endl;
    strcpy(buf, "\0");
}

void receiveFromServer(int sfd) {
    int n = read(sfd, receiverBuf, BUFFER);
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd); 
        exit(EXIT_FAILURE);
    }
    receiverBuf[n] = '\0';
    cout << receiverBuf << endl;
    strcpy(receiverBuf, "\0");
}

void processCommands() {
    fgets(str, 50, stdin);
    sscanf(str, "%s ", command);
    if (!strcmp(command, "exit")) {
        const char *args[5] = {"UNR ", UID, " ", pass, "\n"};
        sendToServer(ASClientTCP, createString(args, 5));
        close(ASClientTCP);
        exit(EXIT_SUCCESS);
    }
    else if (!strcmp(command, "login")) {
        sscanf(str, "%s %s %s", command, UID, pass);
        if (!checkUID(UID) || !checkPass(pass))
            exit(EXIT_FAILURE);
        const char *args[5] = {"LOG ", UID, " ", pass, "\n"};
        sendToServer(ASClientTCP, createString(args, 5));
    }
    else if (!strcmp(command, "req")) {
        sscanf(str, "%s %s", command, Fop);
        srand(time(0));
        sprintf(RID, "%d", rand() % 9000 + 1000);
        if (Fop[0] == 'R' || Fop[0] == 'U' || Fop[0] == 'D') {
            sscanf(Fop, "%s %s", Fop, Fname);
            const char *args[9] = {"REQ ", UID, " ", RID, " ", Fop, " ", Fname, "\n"};
            sendToServer(ASClientTCP, createString(args, 9));
        }
        else if (!strcmp(Fop, "L") || !strcmp(Fop, "X")) {
            const char *args[7] = {"REQ ", UID, " ", RID, " ", Fop, "\n"};
            sendToServer(ASClientTCP, createString(args, 7));
        }
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    
    /*----------ASclientTCP---------------------------------*/
    ASClientTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (ASClientTCP == -1)
        exit(EXIT_FAILURE);
    memset(&hintsASClient, 0, sizeof hintsASClient);
    hintsASClient.ai_family = AF_INET;
    hintsASClient.ai_socktype = SOCK_STREAM;
    s = getaddrinfo(ASIP, ASport, &hintsASClient, &resASClient);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    if (connect(ASClientTCP, resASClient->ai_addr, resASClient->ai_addrlen) == -1) {
        close(ASClientTCP);
        exit(EXIT_FAILURE);
    }
    
    /*----------FSserverTCP---------------------------------*/
    /*
    FSClientTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (FSClientTCP == -1)
        exit(1);
    memset(&hintsFSClient, 0, sizeof hintsFSClient);
    hintsFSClient.ai_family = AF_INET;
    hintsFSClient.ai_socktype = SOCK_STREAM;
    s = getaddrinfo(FSIP, FSport, &hintsFSClient, &resFSClient);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close(FSClientTCP);
        exit(EXIT_FAILURE);
    } 

    //addrlenServer = sizeof(addrServer);
    if (connect(FSClientTCP, resFSClient->ai_addr, resFSClient->ai_addrlen) == -1) {
        perror("Nao conectou2");
        close(FSClientTCP);
        exit(EXIT_FAILURE);
    }
    */

    while (1) {
        timeout.tv_sec = 120;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(afd, &readfds); // i.e reg 92427 ...
        FD_SET(ASClientTCP, &readfds); // i.e VLC 9999
        //FD_SET(FSClientTCP, &readfds); // i.e REG OK
        //maxfd = max(ASClientTCP, FSClientTCP);
        out_fds = select(ASClientTCP + 1, &readfds, (fd_set *) NULL, (fd_set *) NULL, &timeout);
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
                if (FD_ISSET(ASClientTCP, &readfds)) {
                    receiveFromServer(ASClientTCP);
                    break;
                }
                /*
                if (FD_ISSET(FSClientTCP, &readfds)) {
                    receiveFromServer(FSClientTCP);
                    break;
                }
                */
        }
    }
}