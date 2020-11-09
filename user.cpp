#include "aux.h"

struct timeval timeout;
fd_set readfds;
int out_fds, sfd, afd = 0, ASClientTCP, FSClientTCP, maxfd, nRead, s;
struct addrinfo hintsASClient, hintsFSClient, *resASClient, *resFSClient;
string command, UID, recvUID, VC, Fop, buffer, status, pass, aux;
string PDIP, PDport = "57032", ASIP = "localhost", ASport = "58032";
string FSIP, FSport, Fname, RID, TID, nrFiles, filesize, filename, Fsize;

char cPDIP[50];
char cPDport[6]= "57032";
char cASIP[50] = "localhost";
char cASport[6] = "58032";
char ccommand[128];
char cUID[6];
char crecvUID[6];
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
char cFsize[10];
char cstatus[6];
char cnrFiles[4];
char cfilesize[128];
char caux[1];

void parseArgs(int argc, char *argv[]) {
    if (argc < 1 || argc > 9) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-n"))
            ASIP = argv[i + 1];
        else if (!strcmp(argv[i], "-p"))
            ASport = argv[i + 1];
        else if (!strcmp(argv[i], "-m"))
            FSIP = argv[i + 1];
        else if (!strcmp(argv[i], "-q"))
            FSport = argv[i + 1];
    }
}

void sendToServer(int sfd, string buf) {
    if (write(sfd, buf.c_str(), buf.length()) == -1) {
        cout << "Failed write to server" << endl;
        close(sfd); 
        exit(EXIT_FAILURE);
    }
    buf.clear();
}

void closeFSConnection() {
    FD_CLR(FSClientTCP, &readfds);
    close(FSClientTCP);
}

void receiveFromServer(int sfd) {
    nRead = read(sfd, ccommand, 4);
    command = ccommand;
    if (nRead == -1) {
        cout << "Failed read from server" << endl;
        close(sfd);
        exit(EXIT_FAILURE);
    }
    command += "\0";
    if (command == "RLO ") {
        nRead = read(sfd, cstatus, 4);
        status = cstatus;
        if (status == "OK\n")
            cout << "Login: successful" << endl;
        else if (status == "NOK\n") {
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
    if (command == "RRQ ") {
        nRead = read(sfd, cstatus, 6);
        status = cstatus;
        if (status == "OK\n")
            cout << "Request: successful" << endl;
        else if (status == "NOK\n") {
            cout << "Request: not successful" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "ELOG\n") {
            cout << "Request: successful login not previously done" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "EPD\n") {
            cout << "Request: message couldnt be sent by AS to PD" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "EUSER\n") {
            cout << "Request: UID incorrect" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "EFOP\n") {
            cout << "Request: Fop is invalid" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else {
            close(sfd);
            exit(EXIT_FAILURE);
        }
    }
    if (command == "RAU ") {
        nRead = read(sfd, cTID, 4);
        TID = cTID;
        TID += "\0";
        if (TID == "0") {
            cout << "Authentication: failed" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        cout << "Authentication: successful" << endl;
    }
    if (command == "RLS ") {
        string aux;
        while (1) {
            aux.clear();
            nRead = read(sfd, caux, 1);
            aux = caux;
            if (aux == " " || aux == "\n")
                break;
            nrFiles += aux;
        }
        if (nrFiles == "EOF") {
            cout << "List: no files available" << endl;
            close(sfd);
        }
        else if (nrFiles == "INV") {
            cout << "List: AS validation error" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (nrFiles == "ERR") {
            cout << "List: LST request not correctly formulated" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        for (int i = 1; i <= (int) stoi(nrFiles); i++) {
            int nSpaces = 2;    
            filename.clear();
            while (nSpaces) {
                aux.clear();
                nRead = read(sfd, caux, 1);
                aux = caux;
                if (nRead <= 0)
                    break;
                if (aux == "\n")
                    nSpaces = 0;
                if (aux == " ") {
                    nSpaces--;
                    if (!nSpaces)
                        break;
                }
                if (isalnum(aux.at(0)) || aux == "." || aux == "-" || aux == "_" || aux == " ") {
                    aux += "\0";
                    filename += aux; 
                }
                else if (aux == "\n")
                    break;
            }
            aux.clear();
            filename += "\0";
            cout << i << " " << filename << endl;
        }
        closeFSConnection();
    }
    if (command == "RRT ") {
        nRead = read(sfd, cstatus, 3);
        status = cstatus;
        status += "\0";
        if (status == "OK ") {
            /*------Reads filesize-----*/
            filesize.clear();
            char c[1];
            do {
                read(sfd, c, 1);
                filesize += c;
            } while (isdigit(c[0]));

            /*------Reads data------*/
            int reading = 1024;
            int intFilesize = stoi(filesize);
            ofstream file(Fname.c_str());
            do {
                buffer.clear();
                nRead = read(sfd, cbuffer, reading);
                buffer = cbuffer;
                intFilesize -= nRead;
                file << buffer;
            } while (intFilesize > 0);
            file.close();
            cout << "Retrieve: successful" << endl;
        }
        else {
            string aux;
            nRead = read(sfd, caux, 1);
            aux = caux;
            if (!nRead) {
                cout << "Retrieve: cloud not read" << endl;
                close(sfd);
                exit(EXIT_FAILURE);
            }
            status += aux;
            if (status == "EOF\n")
                cout << "Retrieve: file not available" << endl;
            else if (status == "NOK\n") {
                cout << "Retrieve: no content available in the FS for respective user" << endl;
                close(sfd);
                exit(EXIT_FAILURE);
            }
            else if (status == "INV\n") {
                cout << "Retrieve: AS validation error of the provided TID" << endl;
                close(sfd);
                exit(EXIT_FAILURE);
            }
            else if (status == "ERR\n") {
                cout << "Retrieve: request is not correctly formulated" << endl;
                close(sfd);
                exit(EXIT_FAILURE);
            }
        }
        closeFSConnection();
    }
    if (command == "RUP ") {
        nRead = read(sfd, cstatus, 5);
        status = cstatus;
        if (status == "OK\n")
            cout << "Upload: successful" << endl;
        else if (status == "NOK\n") {
            cout << "Upload: not successful" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "DUP\n") {
            cout << "Upload: file already existed" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "FULL\n") {
            cout << "Upload: 15 files were previously uploaded by this User" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "INV\n") {
            cout << "Upload: AS validation error of the provided TID" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "ERR\n") {
            cout << "Upload: UPL request is not correctly formulated" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else {
            close(sfd);
            exit(EXIT_FAILURE);
        }
        closeFSConnection();
    }
    if (command == "RDL ") {
        nRead = read(sfd, cstatus, 4);
        status = cstatus;
        if (status == "OK\n")
            cout << "Delete: successful" << endl;
        else if (status == "EOF\n") {
            cout << "Delete: file not available" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "NOK\n") {
            cout << "Delete: UID does not exist" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "INV\n") {
            cout << "Delete: AS validation error of the provided TID" << endl;
            close(sfd);
            exit(EXIT_FAILURE);
        }
        else if (status == "ERR\n") {
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
    if (command == "RRM ") {
        read(sfd, cstatus, 4);
        status = cstatus;
        if (status == "OK\n")
            cout << "Remove: successful" << endl;
        else if (status == "NOK\n") {
            cout << "Remove: UID does not exist" << endl;
            close(sfd);
            // exit(EXIT_FAILURE);
        }
        else if (status == "INV\n") {
            cout << "Remove: AS validation error of the provided TID" << endl;
            close(sfd);
            // exit(EXIT_FAILURE);
        }
        else if (status == "ERR\n") {
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
    s = getaddrinfo(FSIP.c_str(), FSport.c_str(), &hintsFSClient, &resFSClient);
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
    std::cin >> buffer;
    sscanf(buffer.c_str(), "%s ", ccommand);
    command = ccommand;
    if (command == "exit") {
        for (int fd = 0; fd < maxfd + 1; fd++)
            if (FD_ISSET(fd, &readfds))
                close(fd);
        exit(EXIT_SUCCESS);
    }
    else if (command == "login") {
        sscanf(buffer.c_str(), "%s %s %s", ccommand, cUID, cpass);
        UID = cUID;
        pass = cpass;
        if (!checkUID(UID) || !checkPass(pass))
            exit(EXIT_FAILURE);
        vector<string> args = {"LOG ", UID, " ", pass, "\n"};
        sendToServer(ASClientTCP, createString(args));
    }
    else if (command == "req") {
        sscanf(buffer.c_str(), "%s %s", ccommand, cFop);
        Fop = cFop;
        srand(time(0));
        sprintf(cRID, "%d", rand() % 9000 + 1000);
        RID = cRID;
        if (Fop.at(0) == 'R' || Fop.at(0) == 'U' || Fop.at(0) == 'D') {
            sscanf(buffer.c_str(), "%s %s %s", ccommand, cFop, cFname);
            Fname = cFname;
            vector<string> args = {"REQ ", UID, " ", RID, " ", Fop, " ", Fname, "\n"};
            sendToServer(ASClientTCP, createString(args));
        }
        else if (Fop == "L" || Fop == "X") {
            vector<string> args = {"REQ ", UID, " ", RID, " ", Fop, "\n"};
            sendToServer(ASClientTCP, createString(args));
        }
    }
    else if (command == "val") {
        sscanf(buffer.c_str(), "%s %s", ccommand, cVC);
        command = ccommand;
        VC = cVC;
        vector<string> args = {"AUT ", UID, " ", RID, " ", VC, "\n"};
        sendToServer(ASClientTCP, createString(args));
    }
    else if (command == "list" || command == "l") {
        openFSConnection();
        vector<string> args = {"LST ", UID, " ", TID, "\n"};
        sendToServer(FSClientTCP, createString(args));
    }
    else if (command == "retrieve" || command == "r") {
        openFSConnection();
        sscanf(buffer.c_str(), "%s %s", ccommand, cfilename);
        Fname = cfilename;
        vector<string> args = {"RTV ", UID, " ", TID, " ", Fname, "\n"};
        sendToServer(FSClientTCP, createString(args));
    }
    else if (command == "upload" || command == "u") {
        openFSConnection();
        sscanf(buffer.c_str(), "%s %s", ccommand, cfilename);
        command = ccommand;
        filename = cfilename;
        Fname = filename;
        ifstream file(Fname);
        if (!file.is_open()) {
            cout << "Upload: file does not exist" << endl;
            closeFSConnection();
            exit(EXIT_FAILURE); 
        }
        file.seekg(0, file.end);
        int auxFilesize = file.tellg();
        file.seekg(0, file.beg);
        Fsize = to_string(auxFilesize);

        vector<string> args = {"UPL ", UID, " ", TID, " ", Fname, " ", Fsize, " "};
        sendToServer(FSClientTCP, createString(args));
        
        while (getline(file, buffer)) {
            sendToServer(FSClientTCP, buffer);
        }
        // int reading = 1024;
        // if (auxFilesize < reading)
        //     reading = auxFilesize;
        // while (auxFilesize) {
        //     getline(file, buffer);
        //     if (!nRead)
        //         break;
        //     auxFilesize -= nRead;
        //     if (auxFilesize)
        //         buffer[nRead] = '\0';
        //     sendToServer(FSClientTCP, buffer);
        // }
    }
    else if (command == "delete" || command == "d") {
        openFSConnection();
        sscanf(buffer.c_str(), "%s %s", ccommand, cfilename);
        Fname = cfilename;
        vector<string> args = {"DEL ", UID, " ", TID, " ", Fname, "\n"};
        sendToServer(FSClientTCP, createString(args));
    }
    else if (command == "remove" || command == "x") {
        openFSConnection();
        vector<string> args = {"REM ", UID, " ", TID, "\n"};
        sendToServer(FSClientTCP, createString(args));
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
    s = getaddrinfo(ASIP.c_str(), ASport.c_str(), &hintsASClient, &resASClient);
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