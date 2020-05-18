/***********************************
ASSIGMENT 5: PROBLEM 1 (SUPERGOPHER)
************************************/
#define _GNU_SOURCE // F_SETSIG
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
#include <fcntl.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <stdlib.h>
#include <errno.h>
#define MAXSOCKIND 10
#define TABLEUPDATE 1

struct sockaddr_in preoverlay[10], postoverlay[10], overlayaddr, createoverlay, preaddr[MAXSOCKIND], postaddr[MAXSOCKIND], newaddr; 
int sockfd, sock_post[MAXSOCKIND],sock_pre[MAXSOCKIND], req, port_num, n, activity, read_bytes, flag, no_hash, one;
socklen_t len;
char readbuf[100], sendbuf[100], buf[100], ack[10], port_char[10], ip[15], postport[5];
fd_set readfds;
struct ifreq ifr;

void request_handler();
void communicate();
void request_received();
void request_prev();

//Loookup table structure
struct table{
	
	int index;
	int post_port;
	int pre_port;
	char pre_ip[20];
	char post_ip[20];
	int transit_pre_port;
	int transit_post_port;

} t[10];


//Quit handler
void quit_handler(int signo) {
	printf("\n Exiting....\n");
	exit(0);

}

//New request handler
void io_handler(){

	printf("\n ****New req!****");
	req++; 
	t[req].index++;
	request_handler();	

}


//Handles initialization request
void request_handler(){
	flag = 0;
	memset(buf, '\0', sizeof(buf)); 

	if(req<10){
		
		while(recvfrom(sockfd, (char *)buf, sizeof(buf), 0, (struct sockaddr *) &createoverlay, &len)>0)
			request_received();
	}
	else
		exit(0);
}


void request_received(){
	no_hash=0;
        char nextip[15], nextport[15];
	memset(nextip, '\0', sizeof(nextip));
	memset(nextport, '\0', sizeof(nextport));
	//printf("\n msg rcv: %s", buf);
	int k, i, j;
	k=0;

	for(i=0;buf[i]!='\0';i++){
		if(buf[i] == '@'){
			flag=1;
			buf[i]='\0';
			break;
		}
	}
	

	//processing the payload
	for(i=0;buf[i]!='#';i++);
	buf[i]='0';
	for(j=i+1;buf[j]!='#';j++)
		buf[j]='0';
	buf[j]='0';
	for(i=j+1;buf[i]!='#';i++)
		buf[i]='0';

	//printf("\n msg rcv: %s", buf);
	for(i=0;buf[i]!='#';i++);
	for(j=i+1;buf[j]!='#';j++){
		nextip[k]=buf[j];
		
		k++;	
	}
	
	k=0;
	
	nextip[j]='\0';

	for(i=j+1;buf[i]!='#';i++){
		nextport[k]=buf[i];
		
		k++;	
	}
	nextport[i]='\0';
	//printf("\n msg rcv: %s", buf);
	//printf("\n next IP: %s", nextip);
	strcpy(t[req].post_ip, nextip);
	
	
	//printf("\n next port: %s", nextport);
	t[req].post_port = atoi(nextport);
	//printf("\n next port: %d", t[req].post_port);	

	postaddr[req].sin_family = AF_INET;  
    	postaddr[req].sin_addr.s_addr = inet_addr(nextip); 
    	postaddr[req].sin_port = htons(atoi(nextport));
	//printf("\n next port: %d", ntohs(postaddr[req].sin_port));	
	//printf("\n next IP: %s", inet_ntoa(postaddr[req].sin_addr));

	ack[0] = '3';
	ack[1] = '\0';

	postoverlay[req].sin_family = AF_INET;  
    	postoverlay[req].sin_addr.s_addr = inet_addr(ip); 
    	postoverlay[req].sin_port = htons(0);

	
	if ((sock_post[req] = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	}

	if (bind(sock_post[req], (const struct sockaddr *)&postoverlay[req], sizeof(postoverlay[req])) < 0 ) { 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
    	} 

	if (getsockname(sock_post[req], (struct sockaddr *)&postoverlay[req], &len) == -1)
	    	perror("getsockname failed");
   	else
	    	fprintf(stdout,"\n Transit port number on overlay for next host:%d\n", ntohs(postoverlay[req].sin_port));
	t[req].transit_post_port = ntohs(postoverlay[req].sin_port);
	request_prev();
}

void request_prev(){

	preoverlay[req].sin_family = AF_INET;  
    	preoverlay[req].sin_addr.s_addr = inet_addr(ip); 
    	preoverlay[req].sin_port = htons(0);
	

	if ((sock_pre[req] = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	}

	if (bind(sock_pre[req], (const struct sockaddr *)&preoverlay[req], sizeof(preoverlay[req])) < 0 ) { 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
    	} 

	if (getsockname(sock_pre[req], (struct sockaddr *)&preoverlay[req], &len) == -1)
	    	perror("getsockname failed");
   	else
	    	fprintf(stdout,"\n Transit port number on overlay for previous host:%d\n", ntohs(preoverlay[req].sin_port));

	t[req].transit_pre_port = ntohs(preoverlay[req].sin_port);

	sprintf(port_char, "%d", ntohs(preoverlay[req].sin_port));
	strcat(ack, port_char);
	ack[strlen(ack)]='\0';
	//printf("Sending:%s\n", ack);
	//printf("Sending:%s\n", buf);
	sendto(sockfd, (const char *)ack, sizeof(ack), 0, (const struct sockaddr *) &createoverlay, sizeof(createoverlay));
	for(int i=0;buf[i]!='\0';i++){
		if(buf[i]=='#')
			no_hash++;
	}
	//printf("\n hash: %d", no_hash);

	//forwarding the request payload and sending back an ack
	if(no_hash>3){
		sendto(sock_post[req], (const char *)buf, sizeof(buf), 0, (const struct sockaddr *) &postaddr[req], sizeof(postaddr[req]));
		recvfrom(sock_post[req], (char *)ack, sizeof(ack), 0, (struct sockaddr *) &postaddr[req], &len);
		strncpy(postport, ack+1 ,5);
		postaddr[req].sin_port = htons(atoi(postport));
		t[req].post_port = atoi(postport);
		//printf("Received:%s\n", ack);
		//printf("postport:%s\n", postport);
		
	}
	//printf("\n i sent & recvd ack");


	//waiting for a message from the client
	while(recvfrom(sock_pre[req], (char *)readbuf, sizeof(readbuf), 0, (struct sockaddr *) &preaddr[req], &len) > 0){
                
		t[req].pre_port = ntohs(preaddr[req].sin_port);
		strcpy(t[req].pre_ip, inet_ntoa(preaddr[req].sin_addr));	
		sendto(sock_post[req], (const char *)readbuf, sizeof(readbuf), 0, (const struct sockaddr *) &postaddr[req], sizeof(postaddr[req]));
		break;
	}
	
	communicate();	

}

void communicate(){
	int sd;
	signal(SIGQUIT, quit_handler);

	if(sock_pre[req] > sock_post[req])
        	n = sock_pre[req] + 1;
	else
		n = sock_post[req] + 1;

	int max = req;
	struct sigaction handler;

        //Set signal handler for SIGIO 
        handler.sa_handler = io_handler;
        handler.sa_flags = 0;

        //Create mask that mask all signals 
        if (sigfillset(&handler.sa_mask) < 0) 
                printf("sigfillset() failed");
        //No flags 
        handler.sa_flags = 0;

        if (sigaction(SIGIO, &handler, 0) < 0)
                printf("sigaction() failed for SIGIO");

        //We must own the socket to receive the SIGIO message
        if (fcntl(sockfd, F_SETOWN, getpid()) < 0)
                printf("Unable to set process owner to us");

        //Arrange for nonblocking I/O and SIGIO delivery
        if (fcntl(sockfd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
                printf("Unable to put client sock into non-blocking/async mode");
	int status1 = fcntl(sock_post[req], F_SETFL, fcntl(sock_post[req], F_GETFL, 0) | O_NONBLOCK);
	int status2 = fcntl(sock_pre[req], F_SETFL, fcntl(sock_pre[req], F_GETFL, 0) | O_NONBLOCK);

	//look up table
	if(TABLEUPDATE == 1){
		printf("____________________________________________________________________________\n");
		printf("IND. CLI-PORT    CLI-IP     SER-PORT    SER-IP    TRAN-PORT-1    TRAN-PORT-2");
		for(int i=0;i<req+1;i++){
			printf("\n %d   %d    %s %d %s   %d        %d\n", t[i].index+1, t[i].pre_port, t[i].pre_ip, t[i].post_port, t[i].post_ip, t[i].transit_pre_port, t[i].transit_post_port);
			printf("____________________________________________________________________________\n");
		}
	}
	

	//enabling communication between client and server
	while(1){

		fd_set readfds;
		FD_ZERO(&readfds);
		for (int i = 0 ; i < MAXSOCKIND; i++)   
		{   
		    
		    if(sock_post[i] > 0)   
		        FD_SET( sock_post[i] , &readfds);   
		         
		}   
		for (int i = 0 ; i < MAXSOCKIND; i++)   
		{   
		    
		    if(sock_pre[i] > 0)   
		        FD_SET( sock_pre[i] , &readfds);   
		         
		} 

		activity = select(n, &readfds, NULL, NULL, NULL);

        	if (activity < 0){
            		printf("select error");
        	}
          
        	//If something happened on the master socket , then its an incoming connection
        	if (FD_ISSET(sockfd, &readfds)){
			printf("\nNew request sock....");
			req++; 
			t[req].index++;
			request_handler();     
		}        
		for (int i = 0; i < MAXSOCKIND; i++) {
            		sd = sock_post[i];
              
            		if (FD_ISSET(sd , &readfds)){
			printf("\n Post sock....");
			
			
                	//Check if it was for closing , and also read the incoming message
		        	if ((read_bytes = recvfrom(sock_post[i], (char *)readbuf, sizeof(readbuf), 0, (struct sockaddr *) &postaddr[i], &len)) == 0)
		        	{
				    //Somebody disconnected , get his details and print
				    
				    printf("Server disconnected");
				      
				    //Close the socket and mark as 0 in list for reuse
				    close(sd);
				    sock_post[i] = 0;
				}
		          
		        	//Echo back the message that came in
				else
				{
				    if((i == t[i].index)&&(strcmp(t[i].pre_ip,inet_ntoa(preaddr[i].sin_addr))==0)){
					    //set the string terminating NULL byte on the end of the data read
					    printf("\n Server sent something....");
					    readbuf[read_bytes] = '\0';
					    sendto(sock_pre[i], (const char *)readbuf, sizeof(readbuf), 0, (const struct sockaddr *) &preaddr[i], sizeof(preaddr[i]));
						
					    
				     }
				}

		        }
	
		}  

		for (int i = 0; i < MAXSOCKIND; i++) {
            		sd = sock_pre[i];
              		if ((read_bytes = recvfrom(sock_pre[i], (char *)readbuf, sizeof(readbuf), 0, (struct sockaddr *) &preaddr[i], &len)) > 0)
				sendto(sock_post[i], (const char *)readbuf, sizeof(readbuf), 0, (const struct sockaddr *) &postaddr[i], sizeof(postaddr[i]));

            		if (FD_ISSET(sd , &readfds)){
			printf("\n Previous sock....");
                	//Check if it was for closing , and also read the incoming message
		        	if ((read_bytes = recvfrom(sock_pre[i], (char *)readbuf, sizeof(readbuf), 0, (struct sockaddr *) &newaddr, &len)) == 0)
		        	{
				    //Somebody disconnected , get his details and print
				    
				    printf(" Client disconnected");
				      
				    //Close the socket and mark as 0 in list for reuse
				    close(sd);
				    sock_pre[i] = 0;
				}
		          
		        	//Echo back the message that came in
				else
				{   if((i == t[i].index)&&(strcmp(t[i].pre_ip,inet_ntoa(preaddr[i].sin_addr))==0)){
				    //set the string terminating NULL byte on the end of the data read
					    printf("\nPrevious host sent something....");
					    readbuf[read_bytes] = '\0';
					    sendto(sock_post[i], (const char *)readbuf, sizeof(readbuf), 0, (const struct sockaddr *) &postaddr[i], sizeof(postaddr[i]));
				     }
				}
		        }
	
		}

		
		

	}	
	
}


int main(int argc, char** argv) {

        FD_ZERO(&readfds);
        
	flag=0;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 


	t[req].index=0;
	//Getting the IP address of eth0 interface used by the host 
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFADDR, &ifr);
	strcpy(ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	
        ip[strlen(ip)] = '\0';
	printf("\n OVERLAY IP: %s\n", ip);

	overlayaddr.sin_family = AF_INET; 
	overlayaddr.sin_port = htons(atoi(argv[1])); 
	overlayaddr.sin_addr.s_addr = inet_addr(ip); 
	
	len = sizeof(overlayaddr);

	req = 0;

	
			
	if ( bind(sockfd, (const struct sockaddr *)&overlayaddr, sizeof(overlayaddr)) < 0 ){ 	    
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

	request_handler();

}
