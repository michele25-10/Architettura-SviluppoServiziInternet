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
    int sd, err;
    char username[4096], password[4096], categoria[4096];
    char response[MAX_REQUEST_SIZE];
    size_t len_response;
    struct addrinfo hints, *res, *ptr;
    rxb_t rxb;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: client <server> <port>");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0)
    {
        fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
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
            fprintf(stdout, "Creazione connessione socket");
            break;
        }

        close(sd);
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "ERRORE risoluzione del nome e connessione alla socket\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    rxb_init(&rxb, 2 * MAX_REQUEST_SIZE);

    for (;;)
    {
        puts("Inserisci username ('fine' per terminare):");
        if (fgets(username, sizeof(username), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(username, "fine\n") == 0)
            break;
        if (write_all(sd, username, strlen(username)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        puts("Inserisci password ('fine' per terminare):");
        if (fgets(password, sizeof(password), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(password, "fine\n") == 0)
            break;
        if (write_all(sd, password, strlen(password)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        puts("Inserisci categoria ('fine' per terminare):");
        if (fgets(categoria, sizeof(categoria), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(categoria, "fine\n") == 0)
            break;
        if (write_all(sd, categoria, strlen(categoria)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        for (;;)
        {
            memset(response, 0, sizeof(response));
            len_response = sizeof(response) - 1;
            if (rxb_readline(&rxb, sd, response, &len_response) < 0)
            {
                perror("readline");
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }
#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)response, len_response) != NULL)
            {
                fprintf(stderr, "Richiesta non è in formato UTF-8 valido!\n");
                close(ns);
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
