#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

#define MAX_REQUEST_SIZE (64 * 1024)

int main(int argc, char **argv)
{
    struct addrinfo hints, *res, *ptr;
    int sd, err;
    rxb_t rxb;

    if (argc < 5)
    {
        fprintf(stderr, "Usage: rgrep hostname porta stringa nomefile\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hinst.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(argv[1], argv[2], &hints, &res)) < 0)
    {
        fprintf("Errore nella risoluzione del name\n");
        exit(EXIT_FAILURE);
    }

    for (ptr = res; *ptr != NULL; ptr = ptr->ai_next)
    {
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
        {
            fprintf(stderr, "Socket creation failure\n");
            continue;
        }

        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
        {
            printf("Connessione avvenuta con successo\n");
            break;
        }

        close(sd);
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "Name resolution error\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(&res);

    rxb_init(&rxb, MAX_REQUEST_SIZE);

    /* Invio la stirnga da cercare*/
    if (write_all(sd, argv[3], sizeof(argv[3])) < 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* Invio il nome del file */
    if (write_all(sd, argv[4], sizeof(argv[4])) < 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        char response[MAX_REQUEST_SIZE];
        size_t len_response;

        memset(response, 0, sizeof(response));
        len_response = sizeof(response) - 1;

        if (rxb_readline(&rxb, sd, response, &len_response) < 0)
        {
            rxb_destroy(&rxb);
            fprintf(stderr, "Connessione chiusa dal server");
        }

        puts(response);

        if (strcmp(response, "--- END REQUEST ---") == 0)
        {
            break;
        }
    }

    close(sd);
    return 0;
}