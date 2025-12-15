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
    rxb_t rxb;
    char categoria[1024], nome_produttore[1024], ordinamento[16];
    char response[1024];
    size_t len_response;

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
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
        {
            fprintf(stderr, "Errore nella risoluzione del nome");
            continue;
        }

        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
        {
            fprintf(stdout, "Connesso con successo");
            break;
        }

        close(sd);
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "ERRORE NELLA RISOLUZIONE DEL NAMING\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    rxb_init(&rxb, MAX_REQUEST_SIZE);

    for (;;)
    {
        puts("Inserisci la categoria ('fine' per terminare): ");
        if (fgets(categoria, sizeof(categoria), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine\n", categoria) == 0)
            break;
        if (write_all(sd, categoria, strlen(categoria)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        puts("Inserisci il nome_produttore ('fine' per terminare): ");
        if (fgets(nome_produttore, sizeof(nome_produttore), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine\n", nome_produttore) == 0)
            break;
        if (write_all(sd, nome_produttore, strlen(nome_produttore)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        puts("Inserisci la categoria ('fine' per terminare): ");
        if (fgets(ordinamento, sizeof(ordinamento), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine\n", ordinamento) == 0)
            break;
        if (write_all(sd, ordinamento, strlen(ordinamento)) < 0)
        {
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        for (;;)
        {
            memset(response, 0, sizeof(response));
            len_response = sizeof(response) - 1;

            if (rxb_readline(&rxb, sd, response, &len_response) < 0)
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

    close(sd);
    return 0;
}