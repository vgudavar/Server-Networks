/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define ECHO_PORT 9999
#define BUF_SIZE 4096

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int sock, maxfd;
    int optval = 1;
    fd_set readfds, master;
    FD_ZERO(&readfds);
    FD_ZERO(&master);
    struct sockaddr_in addr;
    char buf[BUF_SIZE];

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;


    fprintf(stdout, "----- Echo Server -----\n");
    
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    maxfd = sock;
    FD_SET(sock, &master);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

    	

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(sock, 10))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    /* finally, loop waiting for input and then write it back */
    while (1)
    { 
       memcpy(&readfds, &master, sizeof(master));
       select(maxfd+1, &readfds, NULL, NULL, &tv);
       for(int i = 0; i < maxfd+1; i++){
          if(FD_ISSET(i, &readfds)){
		struct sockaddr stor;
                socklen_t sockadd_len = sizeof(stor);
                if(i == sock){
			int new_sock = accept(sock, &stor, &sockadd_len);
                        if(new_sock == -1){
				close(sock);
           			fprintf(stderr, "Error accepting connection.\n");
           			return EXIT_FAILURE;
			}
                        printf("this is the accepting socket : %d\n", i);
                        if(new_sock > maxfd) maxfd = new_sock;
                        FD_SET(new_sock, &master);
                }
      		else{
			memset(buf, 0, BUF_SIZE);
                        int bytes_received = recv(i, buf, BUF_SIZE, 0);
			if(bytes_received == 0){
				FD_CLR(i, &master);
                                close_socket(i);
			}
			else{
			    buf[bytes_received] = '\0';
			    send(i, buf, bytes_received, 0); 			
			}
		}
	}
      }
    }
    for(int i = 0; i < maxfd+1; i++){
    	close_socket(i);
    }   

    return EXIT_SUCCESS;
}
