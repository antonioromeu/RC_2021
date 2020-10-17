#include "aux.h"

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

void receiveFromServer(int sfd, char *buf) {
    if (read(sfd, buf, BUFFER) == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd); 
        exit(EXIT_FAILURE);
    }
    cout << buf << endl;
    strcpy(buf, "\0");
}

void processComands() {
    addrlen = sizeof(addr);
    while (1) {
        char str[50];
        cin.getline(str, 50);
        sscanf(str, "%s ", command);
        if (!strcmp(command, "exit")) {
            const char *args[6] = {"UNR", " ", UID, " ", pass, "\n"};
            sendToServer(sfd, createString(args, 6));
            close(sfd);
            exit(EXIT_SUCCESS);
            break;
        }
        else if (!strcmp(command, "login")) {
            sscanf(str, "%s %s %s", command, UID, pass);
            if (!checkUID(UID) || !checkPass(pass))
                exit(EXIT_FAILURE);
            const char *args[6] = {"LOG", " ", UID, " ", pass, "\n"};
            sendToServer(sfd, createString(args, 6));
        }
        else if (!strcmp(command, "req")) {
            sscanf(str, "%s %s", command, Fop);
            srand(time(NULL));
            sprintf(RID, "%d", rand() % 9000 + 1000);
            if (Fop[0] == 'R' || Fop[0] == 'U' || Fop[0] == 'D') {
                sscanf(str, "%s %s %s", command, Fop, Fname);
                const char *args[10] = {"REQ", " ", UID, " ", RID, " ", Fop, " ", Fname, "\n"};
                sendToServer(sfd, createString(args, 10));
            }
            else if (Fop[0] == 'L') {
                const char *args[8] = {"REQ", " ", UID, " ", RID, " ", Fop, "\n"};
                sendToServer(sfd, createString(args, 8));
            }
        }
        receiveFromServer(sfd, receiverBuf);
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) 
        exit(EXIT_FAILURE);
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    s = getaddrinfo(ASIP, ASport, &hints, &res);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    if (connect(sfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Nao conectou");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    processComands();
}

/*
req Fop [Fname]
REQ UID RID Fop [Fname]
else if (!strcmp(command, "val")) {
    
}
else if (!strcmp(command, "list")) {
    
}
else if (!strcmp(command, "l-")) {
    
}
else if (!strcmp(command, "retrieve")) {
    
}
else if (!strcmp(command, "r")) {
    
}
else if (!strcmp(command, "upload")) {
    
}
else if (!strcmp(command, "u")) {
    
}
else if (!strcmp(command, "delete")) {
    
}
else if (!strcmp(command, "d")) {
    
}
else if (!strcmp(command, "remove")) {
    
}
else if (!strcmp(command, "x")) {
    
}
n = write(sfd, "Hello!\n", 7);
if(n == -1)
    exit(1);

n = read(sfd, buffer, 128);
if(n == -1) 
    exit(1);

write(1, "echo: ", 6); 
write(1, buffer, n);
freeaddrinfo(res);
*/