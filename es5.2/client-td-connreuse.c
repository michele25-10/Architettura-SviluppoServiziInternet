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
    char *end_request = "--- END REQUEST ---";
    char email[256], password[1024], rivista[1024];
    char response[MAX_REQUEST_SIZE];
    size_t len_response;
    struct addrinfo hints, *res, *ptr;
    rxb_t rxb;

    if (argc < 3)
    {
        perror("Usage: client <server> <port>");
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(argv[1], argv[2], &hints, &res)) < 0)
    {
        fprintf(stderr, "Errore Resolution Naming\n");
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
        {
            fprintf(stdout, "Socket creata con successo\n");
            break;
        }

        close(sd);
    }

    freeaddrinfo(res);

    if (ptr == NULL)
    {
        fprintf(stderr, "Errore nella creazione socket e resolution naming\n");
        exit(EXIT_FAILURE);
    }

    rxb_init(&rxb, MAX_REQUEST_SIZE);

    for (;;)
    {
        puts("Inserisci l'email ('fine' per terminare):");
        if (fgets(email, sizeof(email), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(email, "fine\n") == 0)
        {
            break;
        }
        if (write_all(sd, email, strlen(email)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        puts("Inserisci la password ('fine' per terminare):");
        if (fgets(password, sizeof(password), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(password, "fine\n") == 0)
        {
            break;
        }
        if (write_all(sd, password, strlen(password)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        puts("Inserisci la rivista ('fine' per terminare):");
        if (fgets(rivista, sizeof(rivista), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(rivista, "fine\n") == 0)
        {
            break;
        }
        if (write_all(sd, rivista, strlen(rivista)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        // RISPOSTA
        for (;;)
        {
            memset(response, 0, sizeof(response));
            len_response = sizeof(response) - 1;

            if (rxb_readline(&rxb, sd, response, &len_response) < 0)
            {
                rxb_destroy(&rxb);
                perror("read");
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

            if (strcmp(response, end_request) == 0)
                break;
        }
    }

    close(sd);
    return 0;
}