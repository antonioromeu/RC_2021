#include "aux.h"

struct timeval timeout;
fd_set readfds;
int out_fds, sfd;
string PDIP, PDport = "57032", ASIP = "localhost", ASport = "58032";
string command, UID, recvUID, VC, Fop, buffer, pass, auxBuffer;

char cbuffer[128];
char cFSIP[50] = "localhost";
char cFSport[6]= "59032";
char cFname[50];
char cRID[5];
char cTID[5];
char cFsize[10];
char cPDIP[50];
char cPDport[6]= "57032";
char cASIP[50] = "localhost";
char cASport[6] = "58032";
char ccommand[128];
char cUID[6];
char crecvUID[6];
char cpass[9];
char cFop[50];
char cVC[5];
char cfilename[128];

int afd = 0, UDP, TCP, s;
string newdir = "USERS/", status;
socklen_t addrlenUDP, addrlenTCP;
struct addrinfo hintsUDP, hintsTCP, *resUDP, *resTCP;
struct sockaddr_in addrUDP, addrTCP;

void parseArgs(int argc, char *argv[]) {
    if (argc < 1 || argc > 9) {
        cout << "Usage: " << argv[0] << " host port msg..." << endl;
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-p"))
            ASport = argv[i];
    }
}

void send(int socket, string buf) {
    int n = 0;
    if (socket == UDP)
        n = sendto(socket, buf.c_str(), buf.length(), 0, (struct sockaddr*) &addrUDP, addrlenUDP);
    else if (socket == TCP)
        n = write(socket, buf.c_str(), buf.length());
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(socket);
        exit(EXIT_FAILURE);
    }
    buf.clear();
}

void receiveUDP(int socket) {
    buffer.clear();
    command.clear();
    pass.clear();
    PDIP.clear();
    PDport.clear();
    auxBuffer.clear();
    status.clear();
    vector<string> matrix;
    memset(cbuffer, '\0', strlen(cbuffer));
    memset(ccommand, '\0', strlen(cbuffer));
    memset(cpass, '\0', strlen(cbuffer));
    memset(cPDIP, '\0', strlen(cbuffer));
    memset(cPDport, '\0', strlen(cbuffer));

    recvfrom(socket, cbuffer, BUFSIZE, 0, (struct sockaddr*) &addrUDP, &addrlenUDP);
    sscanf(cbuffer, "%s ", ccommand);
    buffer = cbuffer;
    buffer += "\0";
    command = ccommand;
    if (command == "REG") {
        sscanf(buffer.c_str(), "%s %s %s %s %s", ccommand, cUID, cpass, cPDIP, cPDport);
        UID = cUID;
        pass = cpass;
        PDIP = cPDIP;
        PDport = cPDport;
        if (!checkDir(UID)) {
            newdir = string("USERS/") + string(UID) + string("\0");
            if (!mkdir(newdir.c_str(), 0777)) {
                cout << "USERS/" << UID << " directory created" << endl;
                string UIDaux = UID;
                matrix = createPathFiles(UIDaux);
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
        sscanf(buffer.c_str(), "%s %s %s", ccommand, cUID, cpass);
        UID = cUID;
        pass = cpass;
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
    s = getaddrinfo(NULL, ASport.c_str(), &hintsUDP, &resUDP);
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
   /* TCP = socket(AF_INET, SOCK_STREAM, 0);
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
    }*/

    while (1) {
        timeout.tv_sec = 120;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(UDP, &readfds);
        FD_SET(TCP, &readfds);
        maxfd = max(UDP, TCP);
        out_fds = select(maxfd + 1, &readfds, (fd_set *) NULL, (fd_set *) NULL, &timeout);
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
}