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
#include <stdbool.h>

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
    int size, number, status;
    char * pos, * pos2;
    char first_letter;
    bool is_getter;

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
    if(size == -1)
    {
        fprintf(stderr, "ERROR: Error retrieving the request: %s.\n", strerror(errno));
        return -1;
    }

    pos = strstr(http_info.buffer,"GET");
    pos2 = strstr(http_info.buffer,"POST");
    if (pos != NULL && pos2 == NULL) // GET request
    {
        is_getter = true;
    }
    else if (pos == NULL && pos2 != NULL) // POST request
    {
        is_getter = false;
    }
    else
    {
        send_HTTP_400(http_info.client_listener);
        return 0;
    }

    pos=strchr(http_info.buffer,'/');
    if (pos == NULL)
    {
        send_HTTP_400(http_info.client_listener);
        return 0;
    }
    pos++;
    first_letter = *pos;
    if (pos == NULL)
    {
        send_HTTP_400(http_info.client_listener);
        return 0;
    }

    if (is_getter) // Get request
    {
        number = get_parameter(first_letter);
        if (number != -1)
        {
            send_HTTP_response(http_info.client_listener, number);
        }
        else
        {
            send_HTTP_400(http_info.client_listener);
        }
    }
    else // Post request
    {
        pos=strchr(pos,'/');
        pos++;
        *(pos+2) = '\0';
        number = atoi(pos);
        status = set_parameter(first_letter, number);
        if (status == 0)
        {
           send_HTTP_200(http_info.client_listener);
        }
        else
        {
            send_HTTP_400(http_info.client_listener);
        }
    }
    return 0;
}


int set_parameter (char first_letter, int number)
{
    if (first_letter == 'd') // Means that the word is discard
    {
        if(number >= 0 && number <=5)
        {
            inteligent_discard_mode = number;
            fprintf(stderr, "INFO: Discard set to mode %d.\n",number );
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else if (first_letter == 's')  // Means that the word is skip
    {
        if(number >= 0 && number <=29)
        {
            frame_skipping_mode = number;
            fprintf(stderr, "INFO: Skipping set to mode %d.\n",number );
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

int get_parameter(char first_letter)
{
    if (first_letter == 'd') // Means that the word is discard
    {
        return inteligent_discard_mode;
    }
    else if (first_letter == 's')  // Means that the word is skip
    {
        return frame_skipping_mode;
    }
    else
    {
        return -1;
    }

}

int close_http_api()
{
    close(http_info.listener);
    return 0;
}

void send_HTTP_400(int socket)
{
    send(socket,"HTTP/1.1 400 Bad Request",24,0);
    shutdown (socket, SHUT_RDWR);
    close(socket);
}

void send_HTTP_200(int socket)
{
    send(socket,"HTTP/1.1 200 OK",15,0);
    shutdown (socket, SHUT_RDWR);
    close(socket);
}

void send_HTTP_response(int socket, int number)
{
    char message[22];

    sprintf(message,"HTTP/1.1 200 OK\r\n\r\n%02d",number);
    send(socket,message,21,0);
    shutdown (socket, SHUT_RDWR);
    close(socket);
}
