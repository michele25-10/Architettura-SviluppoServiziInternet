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
#include "rxb.h"

#define MAX_REQUEST_SIZE (64 * 1024)

int main(int argc, char **argv)
{
    int err, sd;
    struct addrinfo hints, *res, *ptr;
    char vine[1024], year[1024], response[MAX_REQUEST_SIZE];
    size_t len_response;
    rxb_t rxb;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: client <server> <port>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(argv[1], argv[2], &hints, &res)) < 0)
    {
        fprintf(stdout, "Errore Naming Resolution\n");
        exit(EXIT_FAILURE);
    }

    // CONNECT E SOCKET NEL FOR
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
        {
            perror("socket");
            continue;
        }

        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
        {
            fprintf(stdout, "Connessione al server!\n");
            break;
        }

        close(sd);
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "Errore connessione al server\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    rxb_init(&rxb, MAX_REQUEST_SIZE);

    for (;;)
    {
        puts("Inserisci il vino (per uscire 'fine'):");
        if (fgets(vine, sizeof(vine), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(vine, "fine\n") == 0)
        {
            break;
        }
        if (write_all(sd, vine, strlen(vine)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        puts("Inserisci l'annata (per uscire 'fine'):");
        if (fgets(year, sizeof(year), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(year, "fine\n") == 0)
        {
            break;
        }
        if (write_all(sd, year, strlen(year)) < 0)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        for (;;)
        {
            memset(response, 0, sizeof(response));
            len_response = sizeof(response) - 1;

            if (rxb_readline(&rxb, sd, response, &len_response) < 0)
            {
                rxb_destroy(&rxb);
                perror("rxb_readline");
                exit(EXIT_FAILURE);
            }

#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)response, len_response) != NULL)
            {
                fprintf(stderr, "Request is not valid UTF-8!\n");
                close(ns);
                exit(EXIT_SUCCESS);
            }
#endif

            puts(response);

            if (strcmp(response, "--- END RESPONSE ---") == 0)
            {
                break;
            }
        }
    }

    close(sd);
    return 0;
}
