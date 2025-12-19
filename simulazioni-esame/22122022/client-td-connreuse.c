#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#ifdef USE_LIBUNISTRING
#include <unistr.h> /* per libunistring */
#endif
#include "utils.h"
#include "rxb.h"

#define MAX_REQUEST_SIZE (64 * 1024)

int main(int argc, char **argv)
{
    char username[1024], password[1024], tipologia[1024];
    char response[MAX_REQUEST_SIZE];
    size_t len_response;
    struct addrinfo hints, *res, *ptr;
    int sd, err;
    rxb_t rxb;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: client <server> <port>\n");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0)
    {
        fprintf(stderr, "Errore getaddrinfo %s", gai_strerror(err));
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
            fprintf(stdout, "Creata socket con successo\n");
            break;
        }

        close(sd);
    }

    if (ptr != NULL)
    {
        fprintf(stderr, "Errore creazione socket e risoluzione naming\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    rxb_init(&rxb, MAX_REQUEST_SIZE);

    for (;;)
    {
        puts("Inserisci username ('fine' per terminare): ");
        if (fgets(username, sizeof(username), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(username, "fine\n"))
            break;
        if (write_all(sd, username, strlen(username)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        puts("Inserisci password ('fine' per terminare): ");
        if (fgets(password, sizeof(password), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(password, "fine\n"))
            break;
        if (write_all(sd, password, strlen(password)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        puts("Inserisci tipologia ('fine' per terminare): ");
        if (fgets(tipologia, sizeof(tipologia), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp(tipologia, "fine\n"))
            break;
        if (write_all(sd, tipologia, strlen(tipologia)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        for (;;)
        {
            memset(response, 0, sizeof(response));
            len_response = sizeof(response) - 1;
            if (rxb_readline(ns, &rxb, response, &len_response) < 0)
            {
                perror("rxb_readline");
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }
#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)response, len_response) != NULL)
            {
                fprintf(stderr, "Richiesta non è in formato UTF-8 valido!\n");
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