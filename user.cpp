#include "aux.h"

int afd = 0, ASClientTCP, FSClientTCP, maxfd, nRead, s, out_fds, sfd;

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
char status[6];
char nrFiles[4];
char filesize[128];

struct addrinfo hintsASClient, hintsFSClient, *resASClient, *resFSClient;
fd_set readfds;

void parseArgs(int argc, char *argv[]) {
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
        fprintf(stderr, "Failed write to server\n");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    memset(buf, '\0', strlen(buf));
}

void closeFSConnection() {
    FD_CLR(FSClientTCP, &readfds);
    close(FSClientTCP);
}

void receiveFromServer(int sfd) {
    nRead = read(sfd, command, 4);
    if (nRead == -1) {
        fprintf(stderr, "Failed read from server\n");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    command[nRead] = '\0';
    if (!strcmp(command, "RLO ")) {
        nRead = read(sfd, status, 4);
        if (!strcmp(status, "OK\n"))
            cout << "Login: successful" << endl;
        else if (!strcmp(status, "NOK\n")) {
            cout << "Login: not successful" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else {
            cout << "Error" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
    }
    if (!strcmp(command, "RRQ ")) {
        nRead = read(sfd, status, 6);
        if (!strcmp(status, "OK\n"))
            cout << "Request: successful" << endl;
        else if (!strcmp(status, "NOK\n")) {
            cout << "Request: not successful" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "ELOG\n")) {
            cout << "Request: successful login not previously done" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "EPD\n")) {
            cout << "Request: message couldnt be sent by AS to PD" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "EUSER\n")) {
            cout << "Request: UID incorrect" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "EFOP\n")) {
            cout << "Request: Fop is invalid" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else {
            close(sfd);
            exit(EXIT_FAILURE);
        }
    }
    if (!strcmp(command, "RAU ")) {
        nRead = read(sfd, TID, 4);
        TID[nRead] = '\0';
        if (!strcmp(TID, "0")) {
            cout << "Authentication: failed" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        cout << "Authentication: successful" << endl;
    }
    if (!strcmp(command, "RLS ")) {
        char aux[2];
        memset(nrFiles, '\0', strlen(nrFiles));
        while (1) {
            nRead = read(sfd, aux, 1);
            if (aux[0] == ' ' || aux[0] == '\n')
                break;
            aux[1] = '\0';
            strcat(nrFiles, aux);
            strcpy(aux, "\0");
        }
        if (!strcmp(nrFiles, "EOF")) {
            cout << "List: no files available" << endl;
            close(sfd);
        }
        else if (!strcmp(nrFiles, "INV")) {
            cout << "List: AS validation error" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (!strcmp(nrFiles, "ERR")) {
            cout << "List: LST request not correctly formulated" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        for (int i = 1; i <= (int) atoi(nrFiles); i++) {
            int nSpaces = 2;    
            memset(filename, '\0', strlen(filename));
            while (nSpaces) {
                strcpy(aux, "\0");
                nRead = read(sfd, aux, 1);
                if (nRead <= 0)
                    // nSpaces = 0;
                    break;
                if (aux[0] == '\n')
                    nSpaces = 0;
                if (aux[0] == ' ') {
                    nSpaces--;
                    if (!nSpaces)
                        break;
                }
                if (isalnum(aux[0]) || aux[0] == '.' || aux[0] == '-' || aux[0] == '_' || aux[0] == ' ') {
                    aux[1] = '\0';
                    strcat(filename, aux);
                }
                else if (aux[0] == '\n')
                    break;
            }
            strcpy(aux, "\0");
            memset(aux, '\0', strlen(aux));
            strcat(filename, "\0");
            cout << i << " " << filename << endl;
        }
        closeFSConnection();
    }
    if (!strcmp(command, "RRT ")) {
        nRead = read(sfd, status, 3);
        status[nRead] = '\0';
        if (!strcmp(status, "OK ")) {
            /*------Reads filesize-----*/
            memset(filesize, '\0', strlen(filesize));
            char c;
            do {
                read(sfd, &c, 1);
                strncat(filesize, &c, 1);
            } while (isdigit(c));

            /*------Reads data------*/
            int reading = 1024;
            int intFilesize = atoi(filesize);;
            FILE *file = fopen(Fname, "wb");
            do {
                memset(buffer, '\0', strlen(buffer));
                nRead = read(sfd, buffer, reading);
                intFilesize -= nRead;
                fwrite(buffer, 1, nRead, file);
            } while (intFilesize > 0);
            fclose(file);
            cout << "Retrieve: successful" << endl;
        }
        else {
            char aux[2];
            nRead = read(sfd, aux, 1);
            if (!nRead) {
                cout << "Retrieve: cloud not read" << endl;
                close(sfd);
                exit(EXIT_FAILURE);
            }
            strcat(status, aux);
            if (!strcmp(status, "EOF\n"))
                cout << "Retrieve: file not available" << endl;
            else if (!strcmp(status, "NOK\n")) {
                cout << "Retrieve: no content available in the FS for respective user" << endl;
                close(sfd);
                exit(EXIT_FAILURE);
            }
            else if (!strcmp(status, "INV\n")) {
                cout << "Retrieve: AS validation error of the provided TID" << endl;
                close(sfd);
                exit(EXIT_FAILURE);
            }
            else if (!strcmp(status, "ERR\n")) {
                cout << "Retrieve: request is not correctly formulated" << endl;
                close(sfd);
                exit(EXIT_FAILURE);
            }
        }
        closeFSConnection();
    }
    if (!strcmp(command, "RUP ")) {
        nRead = read(sfd, status, 5);
        if (!strcmp(status, "OK\n"))
            cout << "Upload: successful" << endl;
        else if (!strcmp(status, "NOK\n")) {
            cout << "Upload: not successful" << endl;
            // close(sfd);
            // exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "DUP\n")) {
            cout << "Upload: file already existed" << endl;
            // close(sfd);
            // exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "FULL\n")) {
            cout << "Upload: 15 files were previously uploaded by this User" << endl;
            // close(sfd);
            // exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "INV\n")) {
            cout << "Upload: AS validation error of the provided TID" << endl;
            // close(sfd);
            // exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "ERR\n")) {
            cout << "Upload: UPL request is not correctly formulated" << endl;
            // close(sfd);
            // exit(EXIT_FAILURE);
        }
        else {
            close(sfd);
            exit(EXIT_FAILURE);
        }
        closeFSConnection();
    }
    if (!strcmp(command, "RDL ")) {
        nRead = read(sfd, status, 4);
        if (!strcmp(status, "OK\n"))
            cout << "Delete: successful" << endl;
        else if (!strcmp(status, "EOF\n")) {
            cout << "Delete: file not available" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "NOK\n")) {
            cout << "Delete: UID does not exist" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "INV\n")) {
            cout << "Delete: AS validation error of the provided TID" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "ERR\n")) {
            cout << "Delete: DEL request is not correctly formulated" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else {
            close(sfd);
            exit(EXIT_FAILURE);
        }
        closeFSConnection();
    }
    if (!strcmp(command, "RRM ")) {
        read(sfd, status, 4);
        if (!strcmp(status, "OK\n"))
            cout << "Remove: successful" << endl;
        else if (!strcmp(status, "NOK\n")) {
            cout << "Remove: UID does not exist" << endl;
            close(sfd);
            // exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "INV\n")) {
            cout << "Remove: AS validation error of the provided TID" << endl;
            close(sfd);
            // exit(EXIT_FAILURE);
        }
        else if (!strcmp(status, "ERR\n")) {
            cout << "Remove: REM request is not correctly formulated" << endl;
            close(sfd);
            // exit(EXIT_FAILURE);
        }
        else {
            close(sfd);
            exit(EXIT_FAILURE);
        }
        closeFSConnection();
    }
}

void openFSConnection() {
    FSClientTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (FSClientTCP == -1)
        exit(1);
    FD_SET(FSClientTCP, &readfds);
    memset(&hintsFSClient, 0, sizeof hintsFSClient);
    hintsFSClient.ai_family = AF_INET;
    hintsFSClient.ai_socktype = SOCK_STREAM;
    s = getaddrinfo(FSIP, FSport, &hintsFSClient, &resFSClient);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        FD_CLR(FSClientTCP, &readfds);
        close(FSClientTCP);
        exit(EXIT_FAILURE);
    }
    if (connect(FSClientTCP, resFSClient->ai_addr, resFSClient->ai_addrlen) == -1) {
        perror("Nao conseguiu conectar ao FS");
        FD_CLR(FSClientTCP, &readfds);
        close(FSClientTCP);
        exit(EXIT_FAILURE);
    }
}

void processCommands() {
    fgets(buffer, 50, stdin);
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "exit")) {
        for (int fd = 0; fd < maxfd + 1; fd++)
            if (FD_ISSET(fd, &readfds))
                close(fd);
        exit(EXIT_SUCCESS);
    }
    else if (!strcmp(command, "login")) {
        sscanf(buffer, "%s %s %s", command, UID, pass);
        if (!checkUID(UID) || !checkPass(pass))
            exit(EXIT_FAILURE);
        const char *args[5] = {"LOG ", UID, " ", pass, "\n"};
        sendToServer(ASClientTCP, createString(args, 5));
    }
    else if (!strcmp(command, "req")) {
        sscanf(buffer, "%s %s", command, Fop);
        srand(time(0));
        sprintf(RID, "%d", rand() % 9000 + 1000);
        if (Fop[0] == 'R' || Fop[0] == 'U' || Fop[0] == 'D') {
            sscanf(buffer, "%s %s %s", command, Fop, Fname);
            const char *args[9] = {"REQ ", UID, " ", RID, " ", Fop, " ", Fname, "\n"};
            sendToServer(ASClientTCP, createString(args, 9));
        }
        else if (!strcmp(Fop, "L") || !strcmp(Fop, "X")) {
            const char *args[7] = {"REQ ", UID, " ", RID, " ", Fop, "\n"};
            sendToServer(ASClientTCP, createString(args, 7));
        }
    }
    else if (!strcmp(command, "val")) {
        sscanf(buffer, "%s %s", command, VC);
        const char *args[7] = {"AUT ", UID, " ", RID, " ", VC, "\n"};
        sendToServer(ASClientTCP, createString(args, 7));
    }
    else if (!strcmp(command, "list") || !strcmp(command, "l")) {
        openFSConnection();
        const char *args[5] = {"LST ", UID, " ", TID, "\n"};
        sendToServer(FSClientTCP, createString(args, 5));
    }
    else if (!strcmp(command, "retrieve") || !strcmp(command, "r")) {
        openFSConnection();
        sscanf(buffer, "%s %s", command, filename);
        strcpy(Fname, filename);
        const char *args[7] = {"RTV ", UID, " ", TID, " ", Fname, "\n"};
        sendToServer(FSClientTCP, createString(args, 7));
    }
    else if (!strcmp(command, "upload") || !strcmp(command, "u")) {
        openFSConnection();
        sscanf(buffer, "%s %s", command, filename);
        strcpy(Fname, filename);
        
        FILE *file = fopen(Fname, "rb");
        if (file == NULL) {
            cout << "Upload: file does not exist" << endl;
            closeFSConnection();
            exit(EXIT_FAILURE);
        }
        fseek(file, 0, SEEK_END);
        int intFilesize = ftell(file);
        rewind(file);
        itoa(intFilesize, Fsize, 10);

        const char *args[9] = {"UPL ", UID, " ", TID, " ", Fname, " ", Fsize, " "};
        sendToServer(FSClientTCP, createString(args, 9));
        
        int reading = 1024;
        do {
            memset(buffer, '\0', strlen(buffer));
            nRead = fread(buffer, 1, reading, file);
            intFilesize -= nRead;
            sendToServer(FSClientTCP, buffer);
        } while (intFilesize > 0);
        fclose(file);
        memset(buffer, '\0', strlen(buffer));
        strcpy(buffer, "\n");
        sendToServer(FSClientTCP, buffer);
    }
    else if (!strcmp(command, "delete") || !strcmp(command, "d")) {
        openFSConnection();
        sscanf(buffer, "%s %s", command, filename);
        strcpy(Fname, filename);
        const char *args[7] = {"DEL ", UID, " ", TID, " ", Fname, "\n"};
        sendToServer(FSClientTCP, createString(args, 7));
    }
    else if (!strcmp(command, "remove") || !strcmp(command, "x")) {
        openFSConnection();
        const char *args[5] = {"REM ", UID, " ", TID, "\n"};
        sendToServer(FSClientTCP, createString(args, 5));
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
        cout << "no connect" << endl;
        close(ASClientTCP);
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        // FD_ZERO(&readfds);
        FD_SET(afd, &readfds); // i.e reg 92427 ...
        FD_SET(ASClientTCP, &readfds); // i.e VLC 9999
        // FD_SET(FSClientTCP, &readfds); // i.e REG OK
        maxfd = max(ASClientTCP, FSClientTCP);
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
                if (FD_ISSET(ASClientTCP, &readfds)) {
                    receiveFromServer(ASClientTCP);
                    break;
                }
                if (FD_ISSET(FSClientTCP, &readfds)) {
                    receiveFromServer(FSClientTCP);
                    break;
                }
        }
    }
}