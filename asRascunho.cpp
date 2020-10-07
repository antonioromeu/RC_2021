
// #include <unistd.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #define PORT "58011"

// int fd, newfd, errcode;
// ssize_t n;
// socklen_t addrlen;
// struct addrinfo hints, *res;
// struct sockaddr_in addr;
// char buffer[128];

// int main(void) {
//     fd = socket(AF_INET, SOCK_STREAM, 0); //TCP socket
//     if (fd == -1) 
//         exit(1); //Error

//     memset(&hints, 0, sizeof hints);
//     hints.ai_family = AF_INET; //IPv4
//     hints.ai_socktype = SOCK_STREAM; //TCP socket
//     hints.ai_flags = AI_PASSIVE;    

//     errcode = getaddrinfo(NULL, PORT, &hints, &res);
//     if ((errcode) != 0) //Error
//         exit(1);

//     n = bind(fd, res->ai_addr, res->ai_addrlen);
//     if (n == -1) //Error 
//         exit(1);

//     if (listen(fd, 5) == -1) //Error
//         exit(1);

//     while (1) {
//         addrlen = sizeof(addr);
//         if ((newfd = accept(fd, (struct sockaddr*) &addr, &addrlen)) == -1) //Error 
//             exit(1);
        
//         n = read(newfd,buffer,128);
//         if (n == -1) //Error
//             exit(1);
        
//         write(1,"received: ", 10);
//         write(1, buffer, n);
        
//         n = write(newfd, buffer, n);
//         if(n == -1) //Error
//             exit(1);
        
//         close(newfd);
//     }
    
//     freeaddrinfo(res); 
//     close(fd);
// }
// /*


// #include <sys/types.h>
// #include <time.h>
// #include <stdio.h>
// #include <sys/ioctl.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <unistd.h>

// #include <unistd.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #define PORT "58011"

// int main(void) {
//     char in_str[128];
//     fd_set inputs, testfds;
//     struct timeval timeout;
//     int i, out_fds, n;
//     FD_ZERO(&inputs); // Clear inputs
//     FD_SET(0, &inputs); // Set standard input channel on
//     printf("Size of fd_set: %ld\n", sizeof(fd_set));  
//     printf("Value of FD_SETSIZE: %d\n", FD_SETSIZE);
//     while(1) {
//         testfds = inputs;
//         timeout.tv_sec = 10;
//         timeout.tv_usec = 0;
//         printf("testfds byte: %d\n", ((char *)&testfds)[0]);
//         out_fds = select(FD_SETSIZE,&testfds, (fd_set *) NULL, (fd_set *)NULL, &timeout);
//         printf("Time = %ld and %ld\n", timeout.tv_sec, timeout.tv_usec);
//         printf("testfds byte: %d\n", ((char *)&testfds)[0]);
//         switch(out_fds) {
//             case 0:
//                 printf("Timeout event\n");
//                 break;
//             case -1:
//                 perror("select");
//                 exit(1);
//             default:
//                 if(FD_ISSET(0, &testfds)) {
//                     if ((n=read(0,in_str,127))!=0) {
//                         if(n == -1)
//                             exit(1);
//                         in_str[n] = 0;
//                         printf("From keyboard: %s\n",in_str);
//                     }
//                 }
//         }
//     }
// }

// void main(int argc, char **argv){
//   if(argc != 2){
//     printf("Usage: %s <port>\n", argv[0]);
//     exit(0);
//   }

//   int port = atoi(argv[1]);
//   int sockfd;
//   struct sockaddr_in si_me, si_other;
//   char buffer[1024];
//   socklen_t addr_size;

//   sockfd = socket(AF_INET, SOCK_DGRAM, 0);

//   memset(&si_me, '\0', sizeof(si_me));
//   si_me.sin_family = AF_INET;
//   si_me.sin_port = htons(port);
//   si_me.sin_addr.s_addr = inet_addr("127.0.0.1");

//   bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me));
//   addr_size = sizeof(si_other);
//   recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)& si_other, &addr_size);
//   printf("[+]Data Received: %s", buffer);

// }
// */

//udp

// int main(void) {
//     struct addrinfo hints,*res;
//     int fd, errcode;
//     ssize_t n;

//     fd = socket(AF_INET,SOCK_DGRAM,0);//UDP socket
//     if (fd == -1)/*error*/
//         exit(1);

//     memset(&hints, 0, sizeof hints);

// // 	memset(&cliaddr, 0, sizeof(cliaddr));   
//     hints.ai_family=AF_INET;//IPv4
//     hints.ai_socktype=SOCK_DGRAM;//UDP socket

//     errcode = getaddrinfo("tejo.tecnico.ulisboa.pt","58011",&hints,&res);
//     if (errcode != 0)/*error*/
//         exit(1);
//     n = sendto(fd,"Hello!\n",7,0,res->ai_addr,res->ai_addrlen);
//     if (n==-1)/*error*/
//         exit(1);


//     freeaddrinfo(res);
//     exit(0);
//  }

// cd DO    








// // Server side implementation of UDP client-server model 
// #include <stdio.h> 
// #include <stdlib.h> 
// #include <unistd.h> 
// #include <string.h> 
// #include <sys/types.h> 
// #include <sys/socket.h> 
// #include <arpa/inet.h> 
// #include <netinet/in.h> 

// #define PORT 58011
// #define MAXLINE 1024 

// // Driver code 
// int main() { 
// 	int sockfd; 
// 	char buffer[MAXLINE]; 
// 	char *hello = "Hello from server"; 
// 	struct sockaddr_in servaddr, cliaddr; 
	
// 	// Creating socket file descriptor 
// 	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
// 		perror("socket creation failed"); 
// 		exit(EXIT_FAILURE); 
// 	} 
	
// 	memset(&servaddr, 0, sizeof(servaddr)); 
// 	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
// 	// Filling server information 
// 	servaddr.sin_family = AF_INET; // IPv4 
// 	servaddr.sin_addr.s_addr = INADDR_ANY; 
// 	servaddr.sin_port = htons(PORT); 
	
// 	// Bind the socket with the server address 
// 	if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
// 			sizeof(servaddr)) < 0 ) 
// 	{ 
// 		perror("bind failed"); 
// 		exit(EXIT_FAILURE); 
// 	} 
	
// 	int len, n; 

// 	len = sizeof(cliaddr); //len is value/resuslt 

// 	n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
// 				MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
// 				&len); 
// 	buffer[n] = '\0'; 
// 	printf("Client : %s\n", buffer); 
// 	sendto(sockfd, (const char *)hello, strlen(hello), 
// 		MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
// 			len); 
// 	printf("Hello message sent.\n"); 
	
// 	return 0; 
// } 






#include <iostream>
#include <string.h>
#include <fstream>
#include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
#include <unistd.h>
using namespace std;

#define PORT 58011

int main() {
  int serSockDes, readStatus;
  struct sockaddr_in serAddr, cliAddr;
  socklen_t cliAddrLen;
  char buff[1024] = {0};
  char msg[] = "Hello to you too!!!\n";

  //creating a new server socket
  if ((serSockDes = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation error...\n");
    exit(-1);
  }

  //binding the port to ip and port
  serAddr.sin_family = AF_INET;
  serAddr.sin_port = htons(PORT);
  serAddr.sin_addr.s_addr = INADDR_ANY;

  if ((bind(serSockDes, (struct sockaddr*)&serAddr, sizeof(serAddr))) < 0) {
    perror("binding error...\n");
    close(serSockDes);
    exit(-1);
  }

  cliAddrLen = sizeof(cliAddr);
  readStatus = recvfrom(serSockDes, buff, 1024, 0, (struct sockaddr*)&cliAddr, &cliAddrLen);
  if (readStatus < 0) { 
    perror("reading error...\n");
    close(serSockDes);
    exit(-1);
  }

  cout.write(buff, readStatus);
  cout << endl;

  if (sendto(serSockDes, msg, strlen(msg), 0, (struct sockaddr*)&cliAddr, cliAddrLen)) < 0) { 
    perror("sending error...\n");
    close(serSockDes);
    exit(-1);
  }

  close(serSockDes);
  return 0;
}
  