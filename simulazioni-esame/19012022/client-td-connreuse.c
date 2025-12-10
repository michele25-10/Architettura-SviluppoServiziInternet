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
    int sd, err;
    struct addrinfo hints, *res, *ptr;
    char month[8], n[64];
    char response[1024];
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

    if ((err = getaddrinfo(argv[1], argv[2], &hints, &res)) == -1)
    {
        fprintf(stderr, "Errore Resolution Naming\n");
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1)
        {
            fprintf(stderr, "Errore creazione della socket\n");
            continue;
        }

        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
        {
            fprintf(stdout, "Socket creata con successo\n");
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
        memset(month, 0, sizeof(month));
        puts("Inserisci il mese nel formato YYYYMM ('fine' per terminare):");
        if (fgets(month, sizeof(month), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(month, "fine\n") == 0)
            break;
        if (write_all(sd, month, strlen(month)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        memset(n, 0, sizeof(n));
        puts("Inserisci n righe in output ('fine' per terminare):");
        if (fgets(n, sizeof(n), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(n, "fine\n") == 0)
            break;
        if (write_all(sd, n, strlen(n)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        /** LEGGO LA RISPOSTA DAL SERVER */
        for (;;)
        {
            memset(response, 0, sizeof(response));
            len_response = sizeof(response) - 1;

            if (rxb_readline(&rxb, sd, response, &len_response) == -1)
            {
                perror("rxb_readline");
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }

#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)response, len_response) != NULL)
            {
                fprintf(stderr, "Request is not valid UTF-8!\n");
                close(sd);
                exit(EXIT_FAILURE);
            }
#endif

            puts(response);

            if (strcmp(response, "--- END REQUEST ---") == 0)
                break;
        }
    }

    fprintf(stdout, "Programma terminato");

    close(sd);
    return 0;
}