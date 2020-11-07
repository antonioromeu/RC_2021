#include "aux.h"

int afd = 0, ASClientTCP, FSClientTCP, maxfd;
struct addrinfo hintsASClient, hintsFSClient, *resASClient, *resFSClient;
char status[6];
char nrFiles[4];
char filesize[128];
int nRead;

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
    // cout << buf << endl;
    strcpy(buf, "\0");
}

void closeFSConnection() {
    FD_CLR(FSClientTCP, &readfds);
    close(FSClientTCP);
    // FSClientTCP = -1;
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
        nRead = read(sfd, TID, 5);
        if (!strcmp(TID, "0")) {
            cout << "Authentication: failed" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        TID[nRead - 1] = '\0';
        cout << "Authentication: successful" << endl;
    }
    if (!strcmp(command, "RLS ")) {
        char aux[2];
        strcpy(nrFiles, "\0");
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
            // exit(EXIT_FAILURE);
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
            strcat(filename, "\0");
            cout << i << " " << filename << endl;
            strcpy(filename, "\0");
        }
        closeFSConnection();
    }
    if (!strcmp(command, "RRT ")) {
        nRead = read(sfd, status, 3);
        status[nRead] = '\0';
        if (!strcmp(status, "OK ")) {
            /*------Reads filesize-----*/
            char c[2];
            strcpy(filesize, "\0");
            read(sfd, c, 1);
            c[1] = '\0';
            while (strcmp(c, " ")) {
                strcat(filesize, c);
                strcpy(c, "\0");
                read(sfd, c, 1);
                c[1] = '\0';
            }

            /*------Reads data------*/
            FILE *file;
            int reading = 1024;
            int auxFilesize = atoi(filesize);
            file = fopen(Fname, "wb");
            //fp = fopen("xxxxxx.jpg","ab");
            if (atoi(filesize) < reading)
                reading = atoi(filesize);
            nRead = read(sfd, buffer, reading);
            if (nRead < reading)
                nRead -= 1;
            buffer[nRead] = '\0';
            auxFilesize -= nRead;
            fwrite(buffer, sizeof(char), reading, file);
            strcpy(buffer, "\0");
            while (auxFilesize) {
                fwrite(buffer, sizeof(char), reading, file);
                strcpy(buffer, "\0");
                nRead = read(sfd, buffer, reading);
                buffer[nRead] = '\0';
                if (nRead < reading) 
                    nRead -= 1;
                auxFilesize -= nRead;
                if (auxFilesize < reading)
                    reading = auxFilesize;
            }
            strcpy(buffer, "\n");
            fwrite(buffer, sizeof(char), 1, file);
            cout << "Retrieve: successful" << endl;
            fclose(file);
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
    fgets(str, 50, stdin);
    sscanf(str, "%s ", command);
    if (!strcmp(command, "exit")) {
        for (int fd = 0; fd < maxfd + 1; fd++)
            if (FD_ISSET(fd, &readfds))
                close(fd);
        exit(EXIT_SUCCESS);
    }
    else if (!strcmp(command, "login")) {
        sscanf(str, "%s %s %s", command, UID, pass);
        if (!checkUID(ASClientTCP, UID) || !checkPass(ASClientTCP, pass))
            exit(EXIT_FAILURE);
        const char *args[5] = {"LOG ", UID, " ", pass, "\n"};
        sendToServer(ASClientTCP, createString(args, 5));
    }
    else if (!strcmp(command, "req")) {
        sscanf(str, "%s %s", command, Fop);
        srand(time(0));
        sprintf(RID, "%d", rand() % 9000 + 1000);
        if (Fop[0] == 'R' || Fop[0] == 'U' || Fop[0] == 'D') {
            sscanf(str, "%s %s %s", command, Fop, Fname);
            cout << UID << "----" << endl;
            const char *args[9] = {"REQ ", UID, " ", RID, " ", Fop, " ", Fname, "\n"};
            sendToServer(ASClientTCP, createString(args, 9));
        }
        else if (!strcmp(Fop, "L") || !strcmp(Fop, "X")) {
            const char *args[7] = {"REQ ", UID, " ", RID, " ", Fop, "\n"};
            sendToServer(ASClientTCP, createString(args, 7));
        }
    }
    else if (!strcmp(command, "val")) {
        sscanf(str, "%s %s", command, VC);
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
        sscanf(str, "%s %s", command, filename);
        strcpy(Fname, filename);
        const char *args[7] = {"RTV ", UID, " ", TID, " ", Fname, "\n"};
        sendToServer(FSClientTCP, createString(args, 7));
    }
    
    else if (!strcmp(command, "upload") || !strcmp(command, "u")) {
        openFSConnection();
        sscanf(str, "%s %s", command, filename);
        strcpy(Fname, filename);
        FILE *file;
        file = fopen(Fname, "rb");

        fseek(file, 0, SEEK_END);
        int auxFilesize = ftell(file);
        itoa(auxFilesize, Fsize, 10);

        cout << auxFilesize << "-aux" << endl;

        const char *args[9] = {"UPL ", UID, " ", TID, " ", Fname, " ", Fsize, "\n"};
        sendToServer(FSClientTCP, createString(args, 9));
        
        int reading = 1024;
        cout << "antes do while" << endl;

        nRead = fread(buffer, 1, reading, file);
        if (nRead < reading)
                nRead -= 1;
            buffer[nRead] = '\0';
            auxFilesize -= nRead;
            fwrite(buffer, sizeof(char), reading, file);
            strcpy(buffer, "\0");

        while (auxFilesize) {
            cout << "dentro do while" << endl;
            


            nRead = fread(buffer, 1, reading, file);
            if (!nRead)
                break;
            cout << nRead << endl;
            cout << buffer << endl;
            buffer[nRead] = '\0';
            cout << buffer << endl;
            sendToServer(FSClientTCP, buffer);
            auxFilesize -= nRead;
            cout << auxFilesize << endl;
            break;
        }
        cout << "depois do while" << endl;
        cout << buffer << endl;
        buffer[auxFilesize] = '\n';
        sendToServer(FSClientTCP, buffer);
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
    
    while (1) {
        timeout.tv_sec = 120;
        timeout.tv_usec = 0;
        // FD_ZERO(&readfds);
        FD_SET(afd, &readfds); // i.e reg 92427 ...
        FD_SET(ASClientTCP, &readfds); // i.e VLC 9999
        // FD_SET(FSClientTCP, &readfds); // i.e REG OK
        maxfd = max(ASClientTCP, FSClientTCP);
        out_fds = select(maxfd + 1, &readfds, (fd_set *) NULL, (fd_set *) NULL, &timeout);
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