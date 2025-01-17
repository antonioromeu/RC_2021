#include "aux.h"

fd_set readfds;
int out_fds, sfd, afd = 0, clientUDP, masterTCP, s, states[10], usersFD[10], nRead;
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
    // strcpy(PDIP, argv[1]);
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

int sendTCP(int socket, char *buf) {
    int n = write(socket, buf, strlen(buf));
    memset(buf, '\0', strlen(buf));
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        closeAllConnections();
        exit(EXIT_FAILURE);
    }
    return n;
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
        FILE *file;
        file = fopen(filename, "r");
        if (file == NULL) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
            return;
        }
        n = fread(auxBuffer, 1, 128, file);
        fclose(file);
        memset(base, '\0', strlen(base));
        memset(command, '\0', strlen(command));
        sscanf(auxBuffer, "%s %s", base, command);
        s = atoi(base);
        if (!strcmp(Fop, "E")) {
            if (!strcmp(command, "L")) {
                const char *args1[1] = {"RLS INV\n"};
                sendTCP(s, createString(args1, 1));
            }
            if (!strcmp(command, "U")) {
                sscanf(buffer, "%s %s %s %s %s", command, UID, TID, Fop, Fname);
                const char *args1[5] = {"./fsUSERS/", UID, "/files/", Fname, "\0"};
                strcpy(newdir, createString(args1, 5));
                remove(newdir);
                const char *args2[1] = {"RUP INV\n"};
                sendTCP(s, createString(args2, 1));
            }
            if (!strcmp(command, "R")) {
                const char *args1[1] = {"RRT INV\n"};
                sendTCP(s, createString(args1, 1));
            }
            if (!strcmp(command, "D")) {
                const char *args1[1] = {"RDL INV\n"};
                sendTCP(s, createString(args1, 1));
            }
            if (!strcmp(command, "X")) {
                const char *args1[1] = {"REM INV\n"};
                sendTCP(s, createString(args1, 1));
            }
        }
        else if (!strcmp(Fop, "L")) {
            memset(auxBuffer, '\0', strlen(auxBuffer));
            memset(newdir, '\0', strlen(newdir));
            const char *args1[4] = {"./fsUSERS/", UID, "/files/", "\0"};
            strcpy(newdir, createString(args1, 4));
            DIR *d = opendir(newdir);
            if (!d)
                perror("Could not open dir");
            char N[128];
            itoa(countFiles(newdir), N, 128);
            if (!strcmp(N, "0")) {
                const char *args5[1] = {"RLS EOF\n"};
                sendTCP(s, createString(args5, 1));
            }
            else {
                const char *args2[4] = {"RLS ", N, " ", "\0"};
                strcpy(buffer, createString(args2, 4));
                sendTCP(s, buffer);
                struct dirent *dir;
                while ((dir = readdir(d)) != NULL) {
                    if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
                        continue;
                    memset(filename, '\0', strlen(filename));
                    memset(Fsize, '\0', strlen(Fsize));
                    const char *args4[5] = {"./fsUSERS/", UID, "/files/", dir->d_name, "\0"};
                    strcpy(filename, createString(args4, 5));
                    file = fopen(filename, "rb");
                    if (file == NULL) {
                        int errnum = errno;
                        fprintf(stderr, "Value of errno: %d\n", errno);
                        fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
                        return;
                    }
                    fseek(file, 0, SEEK_END);
                    int intFilesize = ftell(file);
                    rewind(file);
                    fclose(file);
                    itoa(intFilesize, Fsize, 10);
                    const char *args3[4] = {dir->d_name, " ", Fsize, " \0"};
                    memset(auxBuffer, '\0', strlen(auxBuffer));
                    strcpy(auxBuffer, createString(args3, 4));
                    sendTCP(s, auxBuffer);
                }
                closedir(d);
                memset(auxBuffer, '\0', strlen(auxBuffer));
                strcpy(auxBuffer, "\n");
                sendTCP(s, auxBuffer);
            }
        }
        else if (Fop[0] == 'U') {
            const char *args[1] = {"RUP OK\n"};
            sendTCP(s, createString(args, 1));
        }
        else if (Fop[0] == 'R') {
            const char *args[1] = {"RRT OK "};
            sendTCP(s, createString(args, 1));
            memset(Fname, '\0', strlen(Fname));
            sscanf(buffer, "%s %s %s %s %s", command, UID, TID, Fop, Fname);
            const char *args1[5] = {"./fsUSERS/", UID, "/files/", Fname, "\0"};
            memset(filename, '\0', strlen(filename));
            strcpy(filename, createString(args1, 5));
            file = fopen(filename, "rb");
            if (file == NULL) {
                int errnum = errno;
                fprintf(stderr, "Value of errno: %d\n", errno);
                fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
            }
            else {
                fseek(file, 0, SEEK_END);
                int intFilesize = ftell(file);
                rewind(file);
                itoa(intFilesize, Fsize, 10);
                const char *args1[2] = {Fsize, " "};
                memset(buffer, '\0', strlen(buffer));
                strcpy(buffer, createString(args1, 2));
                sendTCP(s, buffer);
                int size = 0, n = 0;
                char c[2];
                do {
                    memset(c, '\0', strlen(c));
                    fread(c, 1, 1, file);
                    fflush(stdout);
                    n = write(s, c, 1);
                    if (n < 0) {
                        perror("error writing");
                        break;
                    }
                    size += n;
                } while (intFilesize > size);
                fclose(file);
                memset(c, '\0', strlen(c));
                strcpy(c, "\n");
                sendTCP(s, c);
            }
        }
        else if (Fop[0] == 'D') {
            memset(Fname, '\0', strlen(Fname));
            sscanf(buffer, "%s %s %s %s %s", command, UID, TID, Fop, Fname);
            const char *args1[5] = {"./fsUSERS/", UID, "/files/", Fname, "\0"};
            memset(filename, '\0', strlen(filename));
            strcpy(filename, createString(args1, 5));
            remove(filename);
            const char *args[1] = {"RDL OK\n"};
            sendTCP(s, createString(args, 1));
        }
        else if (Fop[0] == 'X') {
            /*-----------Verifica se a diretoria existe---------*/
            const char *args5[3] = {"rm -r ./fsUSERS/", UID, "\0"};
            memset(newdir, '\0', strlen(newdir));
            strcpy(newdir, createString(args5, 3));
            removeDir(newdir);
            const char *args[1] = {"RRM OK\n"};
            sendTCP(s, createString(args, 1));
            return;
        }
        memset(buffer, '\0', strlen(buffer));
        const char *args4[5] = {"./fsUSERS/", UID, "/", UID, "_fd.txt\0"};
        strcpy(filename, createString(args4, 5));
        file = fopen(filename, "w");
        itoa(s, buffer, 128);
        fwrite(buffer, 1, 128, file);
        fclose(file);
        memset(auxBuffer, '\0', strlen(auxBuffer));
    }

}

void receiveTCP(int socket) {
    FILE *file;
    memset(buffer, '\0', strlen(buffer));
    memset(command, '\0', strlen(command));
    memset(UID, '\0', strlen(UID));
    memset(TID, '\0', strlen(TID));
    memset(Fname, '\0', strlen(Fname));
    memset(Fsize, '\0', strlen(Fsize));
    memset(filename, '\0', strlen(filename));
    int n = read(socket, buffer, 4);
    buffer[n] = '\0';
    strcpy(command, buffer);
    if (!strcmp(command, "LST ")) {
        memset(buffer, '\0', strlen(buffer));
        read(socket, buffer, BUFSIZE);
        sscanf(buffer, "%s %s", UID, TID);
        if (!checkUID(UID) || !(strlen(TID) == 4)) {
            const char *args[1] = {"RLS ERR\n"};
            sendTCP(socket, createString(args, 1));
            return;
        }
        const char *args5[3] = {"./fsUSERS/", UID, "\0"};
        strcpy(filename, createString(args5, 3));
        if (!mkdir(filename, 0777))
            std::cout << "Created ./fsUSERS/" << UID << " directory" << std::endl;
        memset(filename, '\0', strlen(filename));
        const char *args3[4] = {"./fsUSERS/", UID, "/files/", "\0"};
        strcpy(filename, createString(args3, 4));
        if (!mkdir(filename, 0777))
            std::cout << "Created ./fsUSERS/" << UID << "/files/" << " directory" << std::endl;

        /*------Guarda socket no fd--------*/
        memset(filename, '\0', strlen(filename));
        const char *args7[5] = {"./fsUSERS/", UID, "/", UID, "_fd.txt\0"};
        strcpy(filename, createString(args7, 5));
        itoa(socket, auxBuffer, 128);
        file = fopen(filename, "w");
        fwrite(auxBuffer, 1, strlen(auxBuffer), file);
        fclose(file);

        /*-----------Responde para o as-------*/
        const char *args1[5] = {"VLD ", UID, " ", TID, "\n"};
        sendUDP(clientUDP, createString(args1, 5));
    }
    if (!strcmp(command, "UPL ")) {
        memset(auxBuffer, '\0', strlen(auxBuffer));
        memset(buffer, '\0', strlen(buffer));
        int nSpaces = 4;
        while (nSpaces) {
            memset(auxBuffer, '\0', strlen(auxBuffer));
            nRead = read(socket, auxBuffer, 1);
            if (nRead <= 0)
                break;
            if (auxBuffer[0] == '\n')
                nSpaces = 0;
            if (auxBuffer[0] == ' ') {
                nSpaces--;
                if (!nSpaces)
                    break;
            }
            if (isalnum(auxBuffer[0]) || auxBuffer[0] == '.' || auxBuffer[0] == '-' || auxBuffer[0] == '_' || auxBuffer[0] == ' ') {
                auxBuffer[1] = '\0';
                strcat(buffer, auxBuffer);
            }
            else if (auxBuffer[0] == '\n')
                break;
        }
        sscanf(buffer, "%s %s %s %s ", UID, TID, Fname, Fsize);
        if (!checkUID(UID) || !(strlen(TID) == 4) || !checkFilename(Fname) || (strlen(Fsize) > 10)) {
            const char *args[1] = {"RUP ERR\n"};
            sendTCP(socket, createString(args, 1));
            return;
        }
        const char *args1[5] = {"./fsUSERS/", UID, "/files/", Fname, "\0"};
        memset(filename, '\0', strlen(filename));
        strcpy(filename, createString(args1, 5));
        if (fopen(filename, "r")) {
            const char *args5[1] = {"RUP DUP\n"};
            sendTCP(socket, createString(args5, 1));
            return;
        }

        /*-----------Verifica se as diretoriias existem---------*/
        const char *args5[3] = {"./fsUSERS/", UID, "\0"};
        strcpy(filename, createString(args5, 3));
        if (!mkdir(filename, 0777))
            std::cout << "Created ./fsUSERS/" << UID << " directory" << std::endl;
        memset(filename, '\0', strlen(filename));
        const char *args6[4] = {"./fsUSERS/", UID, "/files/", "\0"};
        strcpy(filename, createString(args6, 4));
        if (!mkdir(filename, 0777))
            std::cout << "Created ./fsUSERS/" << UID << "/files/" << " directory" << std::endl;

        /*------Guarda socket no fd--------*/
        memset(filename, '\0', strlen(filename));
        const char *args7[5] = {"./fsUSERS/", UID, "/", UID, "_fd.txt\0"};
        strcpy(filename, createString(args7, 5));
        itoa(socket, auxBuffer, 128);
        file = fopen(filename, "w");
        if (file == NULL) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
        }
        fwrite(auxBuffer, 1, strlen(auxBuffer), file);
        fclose(file);

        /*--------Escreve file na pasta--------*/
        memset(newdir, '\0', strlen(newdir));
        const char *args8[4] = {"./fsUSERS/", UID, "/files/", "\0"};
        strcpy(newdir, createString(args8, 4));
        char N[128];
        itoa(countFiles(newdir), N, 128);
        if (!strcmp(N, "15")) {
            const char *args9[1] = {"RUP FULL\n"};
            sendTCP(socket, createString(args9, 1));
            return;
        }
        memset(UID, '\0', strlen(UID));
        memset(TID, '\0', strlen(TID));
        memset(Fname, '\0', strlen(Fname));
        memset(Fsize, '\0', strlen(Fsize));
        sscanf(buffer, "%s %s %s %s ", UID, TID, Fname, Fsize);
        const char *args10[5] = {"./fsUSERS/", UID, "/files/", Fname, "\0"};
        memset(newdir, '\0', strlen(newdir));
        strcpy(newdir, createString(args10, 5));
        file = fopen(newdir, "w+b");
        if (file == NULL) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
        }
        int intFilesize = atoi(Fsize);
        int size = 0;
        char c[2];
        do {
            memset(c, '\0', 1);
            size += read(socket, c, sizeof(char));
            fwrite(c, sizeof(char), 1, file);
            c[1] = '\0';
        } while (size < intFilesize);
        fclose(file);

        /*-----------Responde para o as-------*/
        const char *args11[5] = {"VLD ", UID, " ", TID, "\n"};
        fflush(stdout);
        sendUDP(clientUDP, createString(args11, 5));
    }
    if (!strcmp(command, "RTV ")) {
        memset(buffer, '\0', strlen(buffer));
        read(socket, buffer, BUFSIZE);
        sscanf(buffer, "%s %s %s", UID, TID, Fname);
        if (!checkUID(UID) || !(strlen(TID) == 4) || !checkFilename(Fname)) {
            const char *args[1] = {"RRT ERR\n"};
            sendTCP(socket, createString(args, 1));
            return;
        }
        /*-----------Verifica se as diretoriias existem---------*/
        const char *args5[3] = {"./fsUSERS/", UID, "\0"};
        strcpy(filename, createString(args5, 3));
        if (!mkdir(filename, 0777))
            std::cout << "Created ./fsUSERS/" << UID << " directory" << std::endl;
        memset(filename, '\0', strlen(filename));
        const char *args6[4] = {"./fsUSERS/", UID, "/files/", "\0"};
        strcpy(filename, createString(args6, 4));
        if (!mkdir(filename, 0777))
            std::cout << "Created ./fsUSERS/" << UID << "/files/" << " directory" << std::endl;

        /*-----------Verifica se file existe-------*/
        const char *args10[5] = {"./fsUSERS/", UID, "/files/", Fname, "\0"};
        memset(newdir, '\0', strlen(newdir));
        strcpy(newdir, createString(args10, 5));
        file = fopen(newdir, "rb");
        if (file == NULL) {
            const char *args[1] = {"RRT EOF\n"};
            sendTCP(socket, createString(args, 1));
            return;
        }
        fclose(file);

        /*------Guarda socket no fd--------*/
        memset(filename, '\0', strlen(filename));
        const char *args7[5] = {"./fsUSERS/", UID, "/", UID, "_fd.txt\0"};
        strcpy(filename, createString(args7, 5));
        itoa(socket, auxBuffer, 128);
        file = fopen(filename, "w");
        if (file == NULL) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
            return;
        }
        fwrite(auxBuffer, 1, strlen(auxBuffer), file);
        fclose(file);
        /*--------Envia mensagem ao AS--------*/
        const char *args1[5] = {"VLD ", UID, " ", TID, "\n"};
        sendUDP(clientUDP, createString(args1, 5));
    }
    if (!strcmp(command, "DEL ")) {

        memset(buffer, '\0', strlen(buffer));
        read(socket, buffer, BUFSIZE);
        sscanf(buffer, "%s %s %s", UID, TID, Fname);
        if (!checkUID(UID) || (strlen(TID) != 4) || !checkFilename(Fname)) {
            const char *args[1] = {"RDL ERR\n"};
            sendTCP(socket, createString(args, 1));
            return;
        }
        /*-----------Verifica se as diretoriias existem---------*/
        const char *args5[3] = {"./fsUSERS/", UID, "\0"};
        strcpy(filename, createString(args5, 3));
        if (!mkdir(filename, 0777))
            std::cout << "Created ./fsUSERS/" << UID << " directory" << std::endl;
        memset(filename, '\0', strlen(filename));
        const char *args6[4] = {"./fsUSERS/", UID, "/files/", "\0"};
        strcpy(filename, createString(args6, 4));
        if (!mkdir(filename, 0777))
            std::cout << "Created ./fsUSERS/" << UID << "/files/" << " directory" << std::endl;


        /*-----------Verifica se a diretoria existe---------*/
        const char *args8[3] = {"./fsUSERS/", UID, "\0"};
        memset(newdir, '\0', strlen(newdir));
        strcpy(newdir, createString(args8, 3));
        DIR* dir = opendir(newdir);
        if (!dir) {
            const char *args[1] = {"RDL NOK\n"};
            sendTCP(socket, createString(args, 1));
            return;
        }

        /*---------Guarda socket no fd--------*/
        memset(filename, '\0', strlen(filename));
        const char *args[5] = {"./fsUSERS/", UID, "/", UID, "_fd.txt\0"};
        strcpy(filename, createString(args, 5));
        itoa(socket, auxBuffer, 128);
        file = fopen(filename, "w");
        if (file == NULL) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
        }
        fwrite(auxBuffer, 1, strlen(auxBuffer), file);
        fclose(file);

        /*-----------Verifica se file existe-------*/
        closedir(dir);
        const char *args1[5] = {"./fsUSERS/", UID, "/files/", Fname, "\0"};
        memset(filename, '\0', strlen(filename));
        strcpy(filename, createString(args1, 5));
        file = fopen(filename, "rb");
        if (file == NULL) {
            const char *args[1] = {"RDL EOF\n"};
            sendTCP(socket, createString(args, 1));
            return;
        }

        /*--------Envia mensagem ao AS--------*/
        const char *args2[5] = {"VLD ", UID, " ", TID, "\n"};
        sendUDP(clientUDP, createString(args2, 5));
    }
    if (!strcmp(command, "REM ")) {
        memset(buffer, '\0', strlen(buffer));
        read(socket, buffer, BUFSIZE);
        sscanf(buffer, "%s %s", UID, TID);
        if (!checkUID(UID) || (strlen(TID) != 4)) {
            const char *args[1] = {"RRM ERR\n"};
            sendTCP(socket, createString(args, 1));
            return;
        }

        /*---------Guarda socket no fd--------*/
        memset(filename, '\0', strlen(filename));
        const char *args[5] = {"./fsUSERS/", UID, "/", UID, "_fd.txt\0"};
        strcpy(filename, createString(args, 5));
        itoa(socket, auxBuffer, 128);
        file = fopen(filename, "w");
        if (file == NULL) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
        }
        fwrite(auxBuffer, 1, strlen(auxBuffer), file);
        fclose(file);

        /*--------Envia mensagem ao AS--------*/
        const char *args2[5] = {"VLD ", UID, " ", TID, "\n"};
        sendUDP(clientUDP, createString(args2, 5));
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