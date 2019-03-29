#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect, AF_INET */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <errno.h> /* errno */

#define HOSTLEN 256
#define PATHLEN 256
/* #define IPADDRLEN 16 */
#define PORTLEN 8

void error(const char *msg) { perror(msg); exit(1); }

int main(int argc, char *argv[])
{
    long portno = 80;
    char ipaddr[HOSTLEN];
    char *tail;
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
        error("ERROR the provided 'server' string is too long");
    }
    strcpy(ipaddr, argv[1]);

    if (strlen(argv[3]) > HOSTLEN)
    {
        error("ERROR the provided 'host-header' string is too long");
    }

    if (strlen(argv[4]) > PATHLEN)
    {
        error("ERROR the provided 'path' string is too long");
    }

    if (strlen(argv[2]) > PORTLEN)
    {
        error("ERROR the provided 'port' string is too long");
    }
    errno = 0;
    portno = strtol(argv[2], &tail, 0);
    if (errno)
    {
        error("ERROR parsing port number");
    }

    /* fill in the request parameters */
    sprintf(message, message_fmt, argv[4], argv[3]);
    /* printf("Request:\n%s\n", message); */

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    /* lookup the server address*/
    server = gethostbyname(ipaddr);
    if (server == NULL) error("ERROR, no such host");

    /* fill in the structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd, message+sent, total-sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
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
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    /*
    if (received == total)
        error("ERROR storing complete response from socket");
    */

    /* close the socket */
    close(sockfd);

    /* process response */
    printf("[simple-healthchecker] DEBUG: Response:\n%s\n", response);

    return 0;
}
