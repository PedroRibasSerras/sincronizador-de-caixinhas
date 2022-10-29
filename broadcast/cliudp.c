// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
	
#define PORT	 8080
#define MAXLINE 1024
	
// Driver code
int main() {
	struct timeval stop[2], start[2];
	
	
	

	int sockfd;
    int yes = 1;
	char buffer[MAXLINE];
	char *hello = "Hello from client";
	struct sockaddr_in	 servaddr;
	
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

    int ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&yes, sizeof(yes));
    if (ret == -1) {
        perror("setsockopt error");
        return 0;
    }
	
	memset(&servaddr, 0, sizeof(servaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	servaddr.sin_port = htons(PORT);
		
	int n, len;
	while(1){
		gettimeofday(&start[0], NULL);
		sendto(sockfd, (const char *)hello, strlen(hello),
			0, (const struct sockaddr *) &servaddr,
				sizeof(servaddr));			
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
					0, (struct sockaddr *) &servaddr,
					&len);
		gettimeofday(&stop[0], NULL);

		
		printf("1 - took %lu us\n", (stop[0].tv_sec - start[0].tv_sec) * 1000000 + stop[0].tv_usec - start[0].tv_usec);
		
	}

	
	close(sockfd);
	return 0;
}
