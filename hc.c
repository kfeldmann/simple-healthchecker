/*
** hc: Simple Healthchecker
**
** A very minimal http client for healthchecking web apps
** inside Docker containers or instances.
**
** Copyright (c) 2019, Kris Feldmann
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**   1. Redistributions of source code must retain the above copyright
**      notice, this list of conditions and the following disclaimer.
**
**   2. Redistributions in binary form must reproduce the above
**      copyright notice, this list of conditions and the following
**      disclaimer in the documentation and/or other materials provided
**      with the distribution.
**
**   3. Neither the name of the copyright holder nor the names of its
**      contributors may be used to endorse or promote products derived
**      from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
** PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
** OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset, strchr */
#include <sys/socket.h> /* socket, connect, AF_INET */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <errno.h> /* errno */

#define HOSTLEN 256
#define PATHLEN 256
#define PORTLEN 8

void error(const char *msg) { perror(msg); exit(1); }

int main(int argc, char *argv[])
{
    long portno = 80;
    char ipaddr[HOSTLEN];
    char *tail;
    char status[4];
    char *message_fmt = "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: simple-healthchecker\r\nConnection: close\r\n\r\n";

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char message[1024], response[4096];

    if (argc != 5)
    {
        puts("Usage: hc server port host-header path");
        exit(1);
    }

    if (strlen(argv[1]) > HOSTLEN)
    {
        error("ERROR: The provided 'server' string is too long");
    }
    strcpy(ipaddr, argv[1]);

    if (strlen(argv[3]) > HOSTLEN)
    {
        error("ERROR: The provided 'host-header' string is too long");
    }

    if (strlen(argv[4]) > PATHLEN)
    {
        error("ERROR: The provided 'path' string is too long");
    }

    if (strlen(argv[2]) > PORTLEN)
    {
        error("ERROR: The provided 'port' string is too long");
    }
    errno = 0;
    portno = strtol(argv[2], &tail, 0);
    if (errno)
    {
        error("ERROR: Failed to parse port number");
    }

    /* fill in the request parameters */
    sprintf(message, message_fmt, argv[4], argv[3]);
    /* printf("DEBUG: Request:\n%s\n", message); */

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR: Failed to open socket");

    /* lookup the server address*/
    server = gethostbyname(ipaddr);
    if (server == NULL) error("ERROR: No such host");

    /* fill in the structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR: Failed to make socket connection");

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd, message+sent, total-sent);
        if (bytes < 0)
            error("ERROR: Failed to write message to socket");
        if (bytes == 0)
            break;
        sent += bytes;
    } while (sent < total);

    /* receive the response */
    memset(response, 0, sizeof(response));
    total = sizeof(response)-1;
    received = 0;
    do {
        bytes = read(sockfd, response+received, total-received);
        if (bytes < 0)
            error("ERROR: Failed to read response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    /* close the socket */
    close(sockfd);

    /* process response */
    strncpy(status, strchr(response, ' ') + 1, 3);
    status[3] = '\0';
    /* printf("DEBUG: status = %s\n", status); */
    /* printf("DEBUG: Response:\n%s\n", response); */
    if (strncmp("2", status, 1) != 0)
    {
        printf("ERROR: Received unhealthy status code: %s\n", status);
        exit(1);
    }

    return 0;
}
