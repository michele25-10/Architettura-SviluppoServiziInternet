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
    char nazione[1024], vacanza[1024], budget[1024];
    char end_response[] = "--- END RESPONSE ---";
    char response[MAX_REQUEST_SIZE];
    size_t len_response;
    rxb_t rxb;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: client <server> <porta>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(argv[1], argv[2], &hints, &res)) < 0)
    {
        fprintf(stderr, "Errore nel BINDING del name\n");
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
            perror("socket");
            continue;
        }

        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
        {
            fprintf(stdout, "Socket creata e connessa correttamente al server!\n");
            break;
        }

        close(sd);
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "Errore risoluzione name e creazione/connessione socket\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    rxb_init(&rxb, MAX_REQUEST_SIZE);

    for (;;)
    {
        /* lettura da tastiera NAZIONE*/
        memset(nazione, 0, sizeof(nazione));
        puts("Inserisci la nazione:");
        if (fgets(nazione, sizeof(nazione), stdin) == NULL)
        {
            close(sd);
            exit(EXIT_FAILURE);
        }

        if (strcmp(vacanza, "fine\n") == 0)
        {
            break;
        }

        /* lettura da tastiera VACANZA*/
        memset(vacanza, 0, sizeof(vacanza));
        puts("Inserisci la vacanza:");
        if (fgets(vacanza, sizeof(vacanza), stdin) == NULL)
        {
            close(sd);
            exit(EXIT_FAILURE);
        }

        /* lettura da tastiera BUDGET*/
        memset(budget, 0, sizeof(budget));
        puts("Inserisci il budget:");
        if (fgets(budget, sizeof(budget), stdin) == NULL)
        {
            close(sd);
            exit(EXIT_FAILURE);
        }

        /** INVIO AL SERVER DI TUTTI I DATI OFFERTI DAL CLIENTE */
        if (write_all(sd, nazione, strlen(nazione)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (write_all(sd, vacanza, strlen(vacanza)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (write_all(sd, budget, strlen(budget)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        // LETTURA RISPOSTA DEL SERVER

        puts("--- START RESPONSE ---");
        for (;;)
        {
            memset(response, 0, sizeof(response));
            len_response = sizeof(response) - 1;

            if (rxb_readline(&rxb, sd, response, &len_response) < 0)
            {
                rxb_destroy(&rxb);
                perror("rxb");
                exit(EXIT_FAILURE);
            }

#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)nazione, len_nazione) != NULL)
            {
                fprintf(stderr, "RESPONSE is not valid UTF-8!\n");
                close(sd);
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }
#endif

            if (strcmp(response, end_response) == 0)
            {
                break;
            }

            puts(response);
        }
    }

    rxb_destroy(&rxb);
    close(sd);

    return 0;
}