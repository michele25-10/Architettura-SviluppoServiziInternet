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
    int err, sd, i = 1;
    struct addrinfo hints, *res, *ptr;
    char *host_remoto, *servizio_remoto;
    rxb_t rxb;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: verifica_compilazione server porta\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    host_remoto = argv[1];
    servizio_remoto = argv[2];
    if ((err = getaddrinfo(host_remoto, servizio_remoto, &hints, &res)) != 0)
    {
        fprintf(stderr, "Errore risoluzione nome %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
        {
            fprintf(stderr, "creazione socket fallita\n");
            continue;
        }

        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
        {
            printf("Connect riuscita al tenantivo: %d\n", i);
            break;
        }

        i++;
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
        char username[1024];
        char nome_progetto[1024];
        char versione[1024];

        /** Prelevo i dati inseriti da tastiera dall'utente */
        puts("Inserisci lo username ('fine' per terminare): ");
        if (fgets(username, sizeof(username), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(username, "fine\n") == 0)
            break;

        puts("Inserisci il nome del progetto ('fine' per terminare): ");
        if (fgets(nome_progetto, sizeof(nome_progetto), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(nome_progetto, "fine\n") == 0)
            break;

        puts("Inserisci la versione ('fine' per terminare): ");
        if (fgets(versione, sizeof(versione), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(versione, "fine\n") == 0)
            break;

        /* Invio dei dati nella socket */
        if (write_all(sd, username, strlen(username)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }
        if (write_all(sd, nome_progetto, strlen(nome_progetto)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }
        if (write_all(sd, versione, strlen(versione)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        for (;;)
        {
            char response[MAX_REQUEST_SIZE];
            size_t len_response;

            /** LETTURA DELLA RISPOSTA DAL SERVER */
            memset(response, 0, sizeof(response));
            len_response = sizeof(response) - 1;
            if (rxb_readline(&rxb, sd, response, &len_response) < 0)
            {
                rxb_destroy(&rxb);
                fprintf(stderr, "Connessione chiusa dal server\n");
                close(sd);
                exit(EXIT_FAILURE);
            }
#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)response, len_response) < 0)
            {
                fprintf(stderr, "Request is not valid UTF-8!\n");
                rxb_destroy(&rxb);
                close(sd);
                exit(EXIT_FAILURE);
            }
#endif

            puts(response);

            if (strcmp(response, "--- END REQUEST ---") == 0)
                break;
        }
    }

    rxb_destroy(&rxb);
    close(sd);
    return 0;
}