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

void receiveFromServer(int sfd, char *buf) {
    if (recvfrom(sfd, receiverBuf, BUFFER, 0, (struct sockaddr*) &addr, &addrlen) == -1) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd); 
        exit(EXIT_FAILURE);
    }
    cout << buf << endl;
    strcpy(buf, "\0");
}

void processCommands() {
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
            if (!checkUID(UID) || !checkPass(pass))
                exit(EXIT_FAILURE);
            const char *args[10] = {"REG", " ", UID, " ", pass, " ", PDIP, " ", PDport, "\n"};
            sendToServer(sfd, createString(args, 10));
        }
        receiveFromServer(sfd, receiverBuf);
    }
}

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        exit(1);

    FD_ZERO(&inputs);
    FD_SET(0, &inputs);
    //state = idle;
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
        testfds = inputs;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        out_fds = select(FD_SETSIZE, &testfds, (fd_set *) NULL, (fd_set *) NULL, &timeout);
        switch (out_fds) {
            case 0:
                printf("Timeout event\n");
                break;
            case -1:
                perror("Select");
                exit(1);
            default:
                if (FD_ISSET(0, &testfds))
                    processCommands();
        }
    }
}
        /*
        FD_ZERO(&rfds);
        FD_SET(sfd, &rfds);
        maxfd = sfd;
            cout << maxfd << endl;

        if (state == busy){
            FD_SET(afd, &rfds);
            maxfd = max(maxfd, afd);
            cout << afd << endl;
        }
        counter = select(maxfd + 1, &rfds, (fd_set*) NULL, (fd_set*) NULL, (struct timeval*) NULL);
        cout << counter << endl;
        if (counter <= 0)
            exit(1);
        if (FD_ISSET(sfd, &rfds)) {
            addrlen = sizeof(addr);
            if ((newfd = accept(sfd, (struct sockaddr*) &addr, &addrlen)) == -1)
                exit(1);
            switch (state) {
                case idle:
                    afd = newfd;
                    state = busy;
                    break;
                case busy:
                    close(newfd);
                    break;
            }
        }
        if (FD_ISSET(afd, &rfds)) {
            if ((n = read(afd, receiverBuf, 128)) != 0) {
                if (n == -1)
                    exit(1);
            }
            else {
                close(afd);
                state = idle;
            }
        }
        */