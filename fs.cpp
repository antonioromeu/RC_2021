#include "aux.h"

fd_set readfds;
int out_fds, sfd, afd = 0, serverUDP, clientUDP, masterTCP, s, states[10], usersFD[10];
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

socklen_t addrlenUDP, addrlenClientUDP, addrlenTCP;
struct addrinfo hintsUDP, hintsClientUDP, hintsTCP, *resUDP, *resClientUDP, *resTCP;
struct sockaddr_in addrUDP, addrClientUDP, addrTCP;

int main(int argc, char **argv) {
    parseArgs(argc, argv);
    int maxFD;
    char aux[6] = "USERS";
    if (!mkdir(aux, 0777))
        printf("Created ./USERS directory\n");
    
    /*------------serverUDP Socket---------*/
    serverUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverUDP == -1)
        exit(1);
    memset(&hintsUDP, 0, sizeof hintsUDP);
    hintsUDP.ai_family = AF_INET;
    hintsUDP.ai_socktype = SOCK_DGRAM;
    hintsUDP.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, ASport, &hintsUDP, &resUDP);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo AS: %s\n", gai_strerror(s));
        close(serverUDP);
        exit(EXIT_FAILURE);
    }
    addrlenUDP = sizeof(addrUDP);
    if (bind(serverUDP, resUDP->ai_addr, resUDP->ai_addrlen) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}