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
    char regione[1024], n_localita[1024], response[MAX_REQUEST_SIZE];
    size_t len_response;
    rxb_t rxb;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: client-td-connreuse <server> <porta>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(argv[1], argv[2], &hints, &res)) < 0)
    {
        fprintf(stderr, "Errore nella risoluzione del naming!\n");
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
        {
            fprintf(stderr, "Errore creazione socket\n");
            continue;
        }

        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
            break;

        close(sd);
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "Errore nella risoluzione del naming e creazione della socket!\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Creazione della socket avvenuta con successo!\n");

    freeaddrinfo(res);
    rxb_init(&rxb, MAX_REQUEST_SIZE);

    for (;;)
    {
        puts("Inserisci la regione('fine' per uscire): ");
        if (fgets(regione, sizeof(regione), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(regione, "fine\n") == 0)
            break;
        if (write_all(sd, regione, strlen(regione)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        puts("Inserisci il numero di località('fine' per uscire): ");
        if (fgets(n_localita, sizeof(n_localita), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(n_localita, "fine\n") == 0)
            break;
        if (write_all(sd, n_localita, strlen(n_localita)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        /**LEGGO LE RISPOSTE DEL SERVER */
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

            if (strcmp(response, "--- END REQUEST ---") == 0)
                break;
        }
    }

    close(sd);
    return 0;
}
