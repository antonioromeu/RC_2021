#include "aux.h"

struct timeval timeout;
fd_set readfds;
int out_fds, sfd;
int usersFD[];
int states[];

int afd = 0, UDP, TCP, s;
string newdir = "USERS/", status;
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

void send(int socket, string buf) {
    int n = 0;
    if (socket == UDP)
        n = sendto(socket, buffer, strlen(buffer), 0, (struct sockaddr*) &addrUDP, addrlenUDP);
    else if (socket == TCP)
        n = write(socket, buffer, strlen(buffer));
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(socket);
        exit(EXIT_FAILURE);
    }
    memset(buffer, '\0', strlen(buffer));
}

void receiveUDP(int socket) {
    char matrix[4][FILENAMESIZE];
    memset(buffer, '\0', strlen(buffer));
    memset(command, '\0', strlen(command));
    // memset(pass, '\0', strlen(pass));
    // memset(PDIP, '\0', strlen(PDIP));
    // memset(PDport, '\0', strlen(PDport));
    recvfrom(socket, buffer, BUFSIZE, 0, (struct sockaddr*) &addrUDP, &addrlenUDP);
    sscanf(buffer, "%s ", command);
    if (!strcmp(command, "REG")) {
        sscanf(buffer, "%s %s %s %s %s", command, UID, pass, PDIP, PDport);
        if (!checkDir(UID)) {
            const char *args[2] = {"./USERS/", UID};
            newdir = createString(args, 2);
            // newdir = string("USERS/") + string(UID) + string("\0");
            if (!mkdir(newdir, 0777)) {
                // cout << "USERS/" << UID << " directory created" << endl;
                printf("./USERS/%s directory created\n", UID);
                matrix = createPathFiles(UID);
                ofstream passFile((matrix.at(0)).c_str());
                passFile << pass << endl;
                passFile.close();
                ofstream regFile((matrix.at(1)).c_str());
                regFile << PDIP << " " << PDport << endl;
                regFile.close();
                status += string("OK\n");
            }
            else {
                cout << "Unable to create USERS/" << UID << " directory" << endl;
                status += string("NOK\n");
            }
        }
        auxBuffer += "RRG " + status;
    }
    if (command == "UNR") {
        sscanf(buffer, "%s %s %s", command, UID, pass);
        if (checkDir(UID)) {
            newdir = string("USERS/") + string(UID) + string("/") + string(UID) + string("_reg.txt");
            remove(newdir.c_str());
            status += string("OK\n");
        }
        else {
            cout << "Unable to remove USERS/" << UID << "/" << UID << "_reg.txt" << endl;
            status += string("NOK\n");
        }
        auxBuffer += "RUN " + status;
    }
    send(UDP, auxBuffer);
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
    
    if (bind(UDP, resUDP->ai_addr, resUDP->ai_addrlen) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    /*------------TCP Socket---------*/
    TCP = socket(AF_INET, SOCK_STREAM, 0);
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
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(UDP, &readfds);
        FD_SET(TCP, &readfds);
        maxfd = max(UDP, TCP);
        out_fds = select(maxfd + 1, &readfds, (fd_set *) NULL, (fd_set *) NULL, NULL);
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
    return 0;
}