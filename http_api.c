/**
 * @file http_api.c
 * @author Francisco José Juan Quintanilla
 * @date May 2018
 * @brief Http Api that allows in-place adjusting of the video coding
 * @see https://github.com/jjaranda13/LHE_Pi
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "include/http_api.h"
#include "include/globals.h"

int init_http_api(int port)
{
    int status, enable = 1;

    http_info.listener = socket(AF_INET, SOCK_STREAM, 0);
    if(http_info.listener== -1)
    {
        fprintf(stderr, "ERROR: Error creating the socket: %s.\n", strerror(errno));
		return -1;
    }

    status =fcntl(http_info.listener, F_SETFL, O_NONBLOCK);
    if (status == -1)
    {
        fprintf(stderr, "ERROR: Error setting the socket as non-blocking: %s.\n", strerror(errno));
		return -1;
    }

    if (setsockopt(http_info.listener, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
        fprintf(stderr, "ERROR: Error setting the socket as reusable: %s.\n", strerror(errno));
		return -1;
    }

    http_info.server.sin_family = AF_INET;
	http_info.server.sin_addr.s_addr = INADDR_ANY;
    http_info.server.sin_port = htons(port);
    memset(&(http_info.server.sin_zero), 0, 8);

    status = bind(http_info.listener, (struct sockaddr*)&(http_info.server),sizeof(struct sockaddr));
    if(status == -1)
    {
        fprintf(stderr, "ERROR: Error binding the socket: %s.\n", strerror(errno));
		return -1;
    }


    status = listen( http_info.listener,1);
    if(status == -1)
    {
        fprintf(stderr, "ERROR: Error listening the socket: %s.\n", strerror(errno));
		return -1;
    }

    http_info.client_size = sizeof(struct sockaddr_in);
    if (http_info.client_size == -1)
    {
        fprintf(stderr, "ERROR: Error setting the size: %s.\n", strerror(errno));
		return -1;
    }

    fprintf(stderr, "INFO: Listening on port %d.\n",port );

    return 0;
}

int process_http_api()
{
    int size, number;
    char * pos;
    http_info.client_listener = accept(http_info.listener,(struct sockaddr *)&(http_info.client), (socklen_t *)&http_info.client_size);
    if(http_info.client_listener== -1 && errno == EWOULDBLOCK)
    {
		return 0;
    }
    if(http_info.client_listener== -1)
    {
        fprintf(stderr, "ERROR: Error accepting the request: %s.\n", strerror(errno));
		return -1;
    }

    size = recv(http_info.client_listener, http_info.buffer, 500, 0);
    if(size == -1) {
    fprintf(stderr, "ERROR: Error retrieving the request: %s.\n", strerror(errno));
		return -1;
    }

    pos=strchr(http_info.buffer,'/');
    if (pos == NULL)
    {
        send(http_info.client_listener,"HTTP/1.1 400 Bad Request",24,0);
        shutdown (http_info.client_listener, SHUT_RDWR);
        close(http_info.client_listener);
        return 0;
    }
    pos++;
    *(pos+1) = '\0';
    number = atoi(pos);
    if(number >= 0 && number <=5){
        inteligent_discard_mode = number;
        fprintf(stderr, "INFO: Discard set to mode %d.\n",number );
        send(http_info.client_listener,"HTTP/1.1 200 OK",15,0);
    }
    else
    {
        send(http_info.client_listener,"HTTP/1.1 400 Bad Request",24,0);
    }
    shutdown (http_info.client_listener, SHUT_RDWR);
    close(http_info.client_listener);
    return 0;
}

int close_http_api()
{
    close(http_info.listener);
    return 0;
}
