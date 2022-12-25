#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *cookie, char *token)
{
    char *message = malloc(BUFLEN * sizeof(char));
    char *line = malloc(LINELEN * sizeof(char));

    memset(line, 0, LINELEN);
    memset(message, 0, BUFLEN);
   
    sprintf(line, "GET %s HTTP/1.1", url);
    compute_message(message, line);
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if(cookie){
        sprintf(line, "Cookie: %s", cookie);
        compute_message(message, line);
    }
    if(token){
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    
    compute_message(message, "");
    //printf("%s\n", message);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char *from_client, char *cookie, char *token)
{
    char *message = malloc(BUFLEN * sizeof(char));
    char *line = malloc(LINELEN * sizeof(char));

    memset(line, 0, LINELEN);
    memset(message, 0, BUFLEN);

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
 
    int sizee = strlen(from_client);
    sprintf(line, "Content-Type: %s\r\nContent-Length: %d", content_type, sizee);
    compute_message(message, line);

   if(cookie){
        sprintf(line, "Cookie: %s", cookie);
        compute_message(message, line);
    }
    if(token){
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    
    
    // Step 5: add new line at end of header
    compute_message(message, "");
    // Step 6: add the actual payload data
    memset(line, 0, LINELEN);
    compute_message(message, from_client);

    //free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *cookie, char *token)
{
    char *message = malloc(BUFLEN * sizeof(char));
    char *line = malloc(LINELEN * sizeof(char));
    memset(line, 0, LINELEN);
    memset(message, 0, BUFLEN);
   
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if(cookie){
        sprintf(line, "Cookie: %s", cookie);
        compute_message(message, line);
    }
    if(token){
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    
    compute_message(message, "");
    //printf("%s\n", message);
    return message;
}