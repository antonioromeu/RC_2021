#include "aux.h"

void parseArgs(int argc, char *argv[]) {
    if (argc < 2 || argc > 8) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    strcpy(PDIP, argv[1]);
    for (int i = 2; i < argc; i += 2) {
        if (!strcmp(argv[i], "-d"))
            strcpy(PDport, argv[i + 1]);
        else if (!strcmp(argv[i], "-n"))
            strcpy(ASIP, argv[i + 1]);
        else if (!strcmp(argv[i], "-p"))
            strcpy(ASport, argv[i + 1]);
    }
}

void sendToServer(int sfd, char *buf) {
    if (sendto(sfd, buf, strlen(buf), 0, res->ai_addr, res->ai_addrlen) == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd); 
        exit(EXIT_FAILURE);
    }
    cout << buf << endl;
    strcpy(buf, "\0");
}

int receiveFromServer(int sfd) {
    int n = recvfrom(sfd, receiverBuf, BUFFER, 0, (struct sockaddr*) &addr, &addrlen);
    if (n == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd); 
        exit(EXIT_FAILURE);
    }
    cout << receiverBuf << endl;
    strcpy(receiverBuf, "\0");
    return n;
}

void sendCommands() {
    addrlen = sizeof(addr);
    while (1) {
        fgets(str, 50, stdin);
        sscanf(str, "%s ", command);
        if (!strcmp(command, "exit")) {
            const char *args[5] = {"UNR", " ", UID, " ", pass};
            sendToServer(sfd, createString(args, 5));
            close(sfd);
            exit(EXIT_SUCCESS);
            break;
        }
        else if (!strcmp(command, "reg")) {
            sscanf(str, "%s %s %s", command, UID, pass);
            if (!checkUID(UID) || !checkPass(pass)) {
                close(sfd);
                exit(EXIT_FAILURE);
            }
            const char *args[10] = {"REG", " ", UID, " ", pass, " ", PDIP, " ", PDport, "\n"};
            sendToServer(sfd, createString(args, 10));
        }
        receiveFromServer(sfd);
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        exit(1);

    state = idle;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    s = getaddrinfo(ASIP, ASport, &hints, &res);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close(sfd);
        exit(EXIT_FAILURE);
    }
    while (1) {
        timeout.tv_sec = 120;
        timeout.tv_usec = 0;
        // clear descriptor 
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(sfd, &wfds);

        maxfd = sfd;
        if (state == busy) {
            //FD_SET(afd, &wfds);
            maxfd = max(maxfd, afd);
        }

        counter = select(maxfd + 1, &rfds, &wfds, NULL, &timeout);
        if (counter <= 0)
            exit(EXIT_FAILURE);

        if (FD_ISSET(sfd, &wfds)) {
            addrlen = sizeof(addr);
            switch (state) {
                case idle:
                    afd = newfd;
                    state = busy;
                    sendCommands();
                    FD_SET(sfd, &rfds);
                    break;
                case busy:
                    close(newfd);
                    break;
            }
        }

        if (FD_ISSET(sfd, &rfds)) {
            cout << "dentro do if de receber" << endl;
            if ((n = receiveFromServer(afd)) != 0) {
                if (n == -1)
                    exit(1);
            }
            else {
                close(maxfd);
                state = idle;
            }
        }
    }
}