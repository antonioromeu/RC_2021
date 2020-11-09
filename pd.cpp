#include "aux.h"

int afd = 0, clientUDP, serverUDP;
socklen_t addrlenClient, addrlenServer;
struct addrinfo hintsClient, hintsServer, *resClient, *resServer;
struct sockaddr_in addrClient, addrServer;
string PDIP, PDport = "57032", ASIP = "localhost", ASport = "58032";
string command, UID, recvUID, VC, Fop, buffer, status, pass;
int s;

struct timeval timeout;
fd_set readfds;
int out_fds, sfd;
char cFsize[10];
char cPDIP[50];
char cPDport[6]= "57032";
char cASIP[50] = "localhost";
char cASport[6] = "58032";
char ccommand[128];
char cUID[6];
char crecvUID[6];
char cstatus[4] = "";
char cpass[9];
char cbuffer[1024];
char cFSIP[50] = "localhost";
char cFSport[6]= "59032";
char cFop[50];
char cFname[50];
char cRID[5];
char cVC[5];
char cTID[5];
char cfilename[128];

void parseArgs(int argc, char *argv[]) {
    if (argc < 2 || argc > 8) {
        cout << "Usage: " << argv[0] << " host port msg...\n";
        exit(EXIT_FAILURE);
    }
    PDIP = argv[1];
    for (int i = 2; i < argc; i += 2) {
        if (!strcmp(argv[i], "-d"))
            PDport = argv[i + 1];
        else if (!strcmp(argv[i], "-n"))
            ASIP = argv[i + 1];
        else if (!strcmp(argv[i], "-p"))
            ASport = argv[i + 1];
    }
}

void sendToServer(int socket, string buf) {
    int n = 0;
    if (socket == clientUDP)
        n = sendto(socket, buf.c_str(), buf.length(), 0, resClient->ai_addr, resClient->ai_addrlen);     //target
    else if (socket == serverUDP)
        n = sendto(socket, buf.c_str(), buf.length(), 0, (struct sockaddr*) &addrServer, addrlenServer);     //target
    if (n == -1) {
        cout << "Failed write" << endl;
        close(socket);
        exit(EXIT_FAILURE);
    }
    buf.clear();
}

void processASAnswer(string buf) {
    sscanf(buf.c_str(), "%s %s %s %s", ccommand, crecvUID, cVC, cFop);
    if (Fop.at(0) == 'R' || Fop.at(0) == 'U' || Fop.at(0) == 'D')
        sscanf(Fop.c_str(), "%s %s", cFop, cFname);
    if (command == "VLC" && UID == recvUID) {
        cout << "Validaton code: " << VC << endl;
        vector<string> args = {"RVC ", UID, " OK\n"};
        sendToServer(serverUDP, createString(args));
    }
    else if (command == "VLC" && UID != recvUID) {
        cout << "Validaton: invalid user ID" << endl;
        vector<string> args = {"RVC ", UID, " NOK\n"};
        sendToServer(serverUDP, createString(args));
    }
}

string receiveFromSocket(int socket) {
    int n = 0;
    buffer.clear();
    if (socket == clientUDP) {
        n = recvfrom(socket, &buffer, BUFSIZE, 0, (struct sockaddr*) &addrClient, &addrlenClient);
        buffer[n] = '\0';
        sscanf(buffer.c_str(), "%s ", ccommand);
        command = ccommand;
        if (command == "RRG") {
            sscanf(buffer.c_str(), "%s %s", ccommand, cstatus);
            status = cstatus;
            if (status == "OK")
                cout << "Registration: successful" << endl;
            else if (status == "NOK")
                cout << "Registration: not accepted" << endl;
        }
        if (command == "RVC") {
            sscanf(buffer.c_str(), "%s %s %s", ccommand, cUID, cstatus);
            UID = cUID;
            status = cstatus;
            if (status == "OK")
                cout << "Validation: valid user" << endl;
            else if (status == "NOK")
                cout << "Validation: invalid user" << endl;
        }
        if (command == "RUN") {
            sscanf(buffer.c_str(), "%s %s", ccommand, cstatus);
            if (status == "OK") {
                cout << "Unregister: successful" << endl;
                close(clientUDP);
                close(serverUDP);
                exit(EXIT_SUCCESS);
            }
            if (status == "NOK")
                cout << "Unregister: not accepted" << endl;
        }
    }
    if (socket == serverUDP) {
        n = recvfrom(socket, &buffer, BUFSIZE, 0, (struct sockaddr*) &addrServer, &addrlenServer);   //addr =A pointer to a socket address structure from which data is received. If address is nonzero, the source address is returned.
        buffer[n] = '\0';
        if (n != -1)
            processASAnswer(buffer);
    }
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(socket); 
        exit(EXIT_FAILURE);
    }
    return buffer;
}

void processCommands() {
    std::cin >> buffer;
    sscanf(buffer.c_str(), "%s ", ccommand);
    command = ccommand;
    if (command == "exit") {
        vector<string> args = {"UNR ", cUID, " ", cpass, "\n"};
        sendToServer(clientUDP, createString(args));
    }
    else if (command == "reg") {
        sscanf(buffer.c_str(), "%s %s %s", ccommand, cUID, cpass);
        command = ccommand;
        UID = cUID;
        pass = cpass;
        if (!checkUID(UID))
            cout << "Register: invalid UID" << endl;
        else if (!checkPass(pass))
            cout << "Register: invalid password" << endl;
        else {
            vector<string> args = {"REG ", cUID, " ", cpass, " ", cPDIP, " ", cPDport, "\n"};
            sendToServer(clientUDP, createString(args));
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
    s = getaddrinfo(ASIP.c_str(), ASport.c_str(), &hintsClient, &resClient);
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
    hintsServer.ai_family = AF_INET;
    hintsServer.ai_socktype = SOCK_DGRAM;
    hintsServer.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, PDport.c_str(), &hintsServer, &resServer);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close(serverUDP);
        exit(EXIT_FAILURE);
    }
    addrlenServer = sizeof(addrServer);
    
    if (bind(serverUDP, resServer->ai_addr, resServer->ai_addrlen) < 0 ) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        timeout.tv_sec = 120;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(afd, &readfds); // i.e reg 92427 ...
        FD_SET(clientUDP, &readfds); // i.e REG OK
        FD_SET(serverUDP, &readfds); // i.e VLC 9999
        maxfd = max(clientUDP, serverUDP);
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