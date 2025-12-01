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
    int sd;
    char category[1024];
    struct addrinfo hints, *ptr, *res;
    char response[MAX_REQUEST_SIZE];
    size_t len_response;
    rxb_t rxb;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: client-connreuse <server> <port>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(argv[1], argv[2], &hints, &res) < 0)
    {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
        {
            perror("Socket");
            continue;
        }

        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
        {
            fprintf(stdout, "Connesso al server con successo!\n");
            break;
        }

        close(sd);
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "Errore risoluzione nome e connessione\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    rxb_init(&rxb, MAX_REQUEST_SIZE);

    for (;;)
    {
        puts("Inserisci la categoria della spesa che ti interessa");
        if (fgets(category, sizeof(category), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        if (strcmp(category, "fine\n") == 0)
        {
            break;
        }

        if (write_all(sd, category, strlen(category)) < 0)
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
                perror("rxb readline");
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }

#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)response, response) != NULL)
            {
                fprintf(stderr, "Response is not valid UTF-8!\n");
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
