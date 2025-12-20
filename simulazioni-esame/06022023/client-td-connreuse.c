#define _POSIX_C_SOURCE 200809L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#ifdef USE_LIBUNISTRING
#include <unistr.h>
#endif
#include "rxb.h"
#include "utils.h"

#define MAX_REQUEST_SIZE (64 * 1024)

int main(int argc, char **argv)
{
    int err, sd;
    struct addrinfo hints, *res, *ptr;
    char *host_remoto;
    char *servizio_remoto;
    rxb_t rxb;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: statistiche_6_nazioni <hostname> <porta>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    host_remoto = argv[1];
    servizio_remoto = argv[2];
    if ((err = getaddrinfo(host_remoto, servizio_remoto, &hints, &res)) < 0)
    {
        fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
        {
            fprintf(stderr, "Creazione socket fallita\n");
            continue;
        }

        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
        {
            fprintf(stdout, "connect riuscita!\n");
            break;
        }

        close(sd);
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "Errore risoluzione nome: nessun indirizzo corrispondente trovato\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    rxb_init(&rxb, MAX_REQUEST_SIZE * 2);

    for (;;)
    {
        char nome_giocatore[1024], squadra[1024], anno[8];

        puts("Inserisci il nome del giocatore ('fine' per terminare): ");
        if (fgets(nome_giocatore, sizeof(nome_giocatore), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(nome_giocatore, "fine\n") == 0)
            break;

        puts("Inserisci la squadra ('fine' per terminare): ");
        if (fgets(squadra, sizeof(squadra), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(squadra, "fine\n") == 0)
            break;

        puts("Inserisci l'anno ('fine' per terminare): ");
        if (fgets(anno, sizeof(anno), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(anno, "fine\n") == 0)
            break;

        if (write_all(sd, nome_giocatore, strlen(nome_giocatore)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (write_all(sd, squadra, strlen(squadra)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (write_all(sd, anno, strlen(anno)) < 0)
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
                perror("rxb_readline");
                exit(EXIT_FAILURE);
            }
#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)response, len_response) != NULL)
            {
                fprintf(stderr, "Risposta non è in formato UTF-8 valido!\n");
                rxb_destroy(&rxb);
                close(ns);
                exit(EXIT_FAILURE);
            }
#endif

            puts(response);

            if (strcmp(response, "--- END REQUEST ---") == 0)
                break;
        }
    }

    close(sd);
    rxb_destroy(&rxb);
    return 0;
}