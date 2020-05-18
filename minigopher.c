/***********************************
ASSIGMENT 5: PROBLEM 1 (MINIGOPHER)
************************************/
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>


int main(int argc, char** argv) {
	struct ifreq ifr;
	struct sockaddr_in overlayaddr, coverlayaddr;
	int sockfd, k;
	char readbuf[100], sendbuf[100], ip[20], hash[2], k_char[5];
	socklen_t len;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	//Getting the IP address of eth0 interface used by the host 
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFADDR, &ifr);
	strcpy(ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	coverlayaddr.sin_family = AF_INET; 
	coverlayaddr.sin_port = htons(0); 
	coverlayaddr.sin_addr.s_addr = inet_addr(ip); 

        overlayaddr.sin_family = AF_INET; 
	overlayaddr.sin_port = htons(atoi(argv[2])); 
	overlayaddr.sin_addr.s_addr = inet_addr(argv[1]);
	
	len = sizeof(overlayaddr);

			
	if ( bind(sockfd, (const struct sockaddr *)&coverlayaddr, sizeof(coverlayaddr)) < 0 ){ 	    
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

	
        ip[strlen(ip)] = '\0';
	printf("\n Minigopher IP: %s\n", ip);
	memset(sendbuf, '\0', sizeof(sendbuf)); 

	//Preparing the payload for sending the request  

	k = (argc-2)/2;
	//printf("\n#k: %d", k);
	sprintf(k_char, "%d", k);
	k_char[strlen(k_char)]='#';
	strcpy(sendbuf, k_char);
	

        for(int i=1;i<argc;i++){
		strcat(sendbuf, argv[i]);
		sendbuf[strlen(sendbuf)]='#';
		//printf("\n#Msg sending: %s", sendbuf);
	}

        sendbuf[strlen(sendbuf)]='@';
	sendto(sockfd, (const char *)sendbuf, sizeof(sendbuf), 0, (const struct sockaddr *) &overlayaddr, sizeof(overlayaddr));
	//printf("\n#Msg sent: %s\n", sendbuf);


        //Receiving the ack from the supergopher
	while(recvfrom(sockfd, (char *)readbuf, sizeof(readbuf), 0, (struct sockaddr *) &overlayaddr, &len) > 0){

		//printf("\n#Msg rcv: %s", readbuf);
		if(readbuf[0]== '3')
			printf("\n#Transit port for client: %.*s\n", 10, readbuf+1);
		printf("\n#Closing...\n");
		exit(0);
	}

	close(sockfd);

}
