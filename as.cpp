#include "aux.h"

fd_set readfds;
int out_fds, sfd, afd = 0, UDP, TCP, s, states[10], usersFD[10];
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
  
int master_socket, addrlen, new_socket, client_socket[10], max_clients = 10 , activity, i , valread , sd;
int max_sd;   
struct sockaddr_in address;   

socklen_t addrlenUDP, addrlenTCP;
struct addrinfo hintsUDP, hintsTCP, *resUDP, *resTCP;
struct sockaddr_in addrUDP, addrTCP;

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

void send(int socket, char *buf) {
    int n = 0;
    if (socket == UDP)
        n = sendto(socket, buf, strlen(buf), 0, (struct sockaddr*) &addrUDP, addrlenUDP);
    else if (socket == TCP)
        n = write(socket, buf, strlen(buf));
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(socket);
        exit(EXIT_FAILURE);
    }
    memset(buf, '\0', strlen(buf));
}

void receiveUDP(int socket) {
    memset(buffer, '\0', strlen(buffer));
    memset(command, '\0', strlen(command));
    memset(base, '\0', strlen(base));
    memset(newdir, '\0', strlen(base));
    int n = recvfrom(socket, buffer, BUFSIZE, 0, (struct sockaddr*) &addrUDP, &addrlenUDP);
    buffer[n] = '\0';
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "REG")) {
        sscanf(buffer, "%s %s %s %s %s", command, UID, pass, PDIP, PDport);
        // strcat(UID, "\0");
        strcat(pass, "\0");
        strcat(PDIP, "\0");
        strcat(PDport, "\0");
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
        sscanf(buffer, "%s %s %s", command, UID, pass);
        if (checkDir(UID)) {
            const char *args[5] = {"USERS/", UID, "/", UID, "_reg.txt"};
            strcpy(newdir, createString(args, 5));
            remove(newdir);
            strcat(status, "OK\n");
        }
        else {
            printf("Unable to remove USERS/%s/%s_reg.txt", UID, UID);
            strcat(status, "NOK\n");
        }
        strcat(auxBuffer, "RUN ");
        strcat(auxBuffer, status);
    }
    send(UDP, auxBuffer);
}

void receiveTCP(int socket) {
    memset(buffer, '\0', strlen(buffer));
    memset(command, '\0', strlen(command));
    int n = read(socket, buffer, BUFSIZE);
    buffer[n] = '\0';
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "LOG")) {
        sscanf(buffer, "%s %s %s", command, recvUID, recvPass);
        cout << recvUID << endl;
        cout << recvPass << endl;
        if (!strcmp(UID, recvUID) && !strcmp(pass, recvPass)) {
            const char *args[5] = {"USERS/", UID, "/", UID, "_login.txt\0"};
            strcpy(loginFile, createString(args, 5));
            FILE *file;
            file = fopen(loginFile, "w");
            fclose(file);
        }
    }
    if (!strcmp(command, "REQ")) {
        srand(time(0));
        sprintf(VC, "%d", rand() % 9000 + 1000);
        sscanf(buffer, "%s %s %s %s", command, UID, RID, Fop);
        if (Fop[0] == 'R' || Fop[0] == 'U' || Fop[0] == 'D') {
            sscanf(buffer, "%s %s %s %s %s", command, UID, RID, Fop, Fname);
            const char *args[9] = {"VLC ", UID, " ", VC, " ", Fop, " ", Fname, "\n"};
            send(UDP, createString(args, 9));
        }
        else if (!strcmp(Fop, "L") || !strcmp(Fop, "X")) {
            const char *args[7] = {"VLC ", UID, " ", VC, " ", Fop, "\n"};
            send(UDP, createString(args, 7));
        }
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    int maxfd;
    char aux[6] = "USERS";
    if (!mkdir(aux, 0777))
        printf("Created ./USERS directory\n");
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
        fprintf(stderr, "getaddrinfo AS: %s\n", gai_strerror(s));
        close(UDP);
        exit(EXIT_FAILURE);
    }
    addrlenUDP = sizeof(addrUDP);
    
    if (bind(UDP, resUDP->ai_addr, resUDP->ai_addrlen) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    /*------------TCP Socket---------*/
    for (i = 0; i < max_clients; i++) {   
        client_socket[i] = 0;   
    }

    TCP = socket(AF_INET, SOCK_STREAM, 0);
    if (TCP == -1)
        exit(1);
    
    memset(&hintsTCP, 0, sizeof hintsTCP);
    hintsTCP.ai_family = AF_INET;
    hintsTCP.ai_socktype = SOCK_STREAM;
    hintsTCP.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, ASport, &hintsTCP, &resTCP);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo TCP : %s\n", gai_strerror(s));
        close(TCP);
        exit(EXIT_FAILURE);
    }

    if (bind(TCP, (struct sockaddr *) &addrTCP, sizeof(addrTCP)) < 0) {   
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(TCP, 3) < 0) {   
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    addrlenTCP = sizeof(addrTCP);

    while (1) {
        FD_ZERO(&readfds);   
        FD_SET(UDP, &readfds);
        FD_SET(TCP, &readfds);
        max_sd = TCP;   
             
        for (i = 0  i < max_clients; i++) {
            sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd , &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        maxfd = max(UDP, max_sd);
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }
        if (FD_ISSET(UDP, &readfds)) {
                receiveUDP(UDP);
                break;
            }
        else if (FD_ISSET(master_socket, &readfds)) {   
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {   
                perror("accept");
                exit(EXIT_FAILURE);
            }
            for (i = 0; i < max_clients; i++) {   
                if (client_socket[i] == 0 ) {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
                    break;
                }
            }
        }
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (FD_ISSET( sd , &readfds)) {
                if ((valread = read(sd, buffer, 1024)) == 0) {   
                    receiveTCP(TCP);
                    //Somebody disconnected , get his details and print  
                    getpeername(sd, (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    close(sd);   
                    client_socket[i] = 0;   
                }   
                else {
                    buffer[valread] = '\0';
                    send(sd, buffer, strlen(buffer) , 0);
                }
            }   
        }
    }

    // if (bind(TCP, resTCP->ai_addr, resTCP->ai_addrlen) < 0) {
    //     perror("Bind failed");
    //     exit(EXIT_FAILURE);
    // }
    // if ((listen(TCP, 10)) == -1) {
    //     printf("Listen failed\n");
    //     exit(EXIT_FAILURE);
    // }
    // if (accept(TCP, (struct sockaddr*) &addrTCP, &addrlenTCP) < 0) {
    //     printf("server acccept failed\n");
    //     exit(EXIT_FAILURE);
    // }

    // while (1) {
    //     FD_ZERO(&readfds);
    //     FD_SET(UDP, &readfds);
    //     FD_SET(TCP, &readfds);
    //     maxfd = max(UDP, TCP);
    //     out_fds = select(maxfd + 1, &readfds, (fd_set *) NULL, (fd_set *) NULL, NULL);
    //     switch (out_fds) {
    //         case 0:
    //             printf("Timeout\n");
    //             break;
    //         case -1:
    //             perror("Select\n");
    //             exit(EXIT_FAILURE);
    //         default:
    //             if (FD_ISSET(UDP, &readfds)) {
    //                 receiveUDP(UDP);
    //                 break;
    //             }
    //             if (FD_ISSET(TCP, &readfds)) {
    //                 // cout << "receive tcp" << endl;
    //                 // if ((listen(TCP, 1)) == -1) {
    //                 //     printf("Listen failed\n");
    //                 //     exit(EXIT_FAILURE);
    //                 // }
    //                 if (accept(TCP, (struct sockaddr*) &addrTCP, &addrlenTCP) < 0) { 
    //                     printf("server acccept failed\n");
    //                     exit(EXIT_FAILURE);
    //                 }
    //                 receiveTCP(TCP);
    //                 break;
    //             }
    //     }
    // }
    return 0;
}