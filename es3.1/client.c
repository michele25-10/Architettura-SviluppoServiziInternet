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

#define MAX_REQUEST_SIZE (64 * 1024)

int main(int argc, char **argv)
{
    char *option, *host_remoto, *servizio_remoto;
    int sd, err, nread;
    struct addrinfo hints, *ptr, *res;
    rxb_t rxb;

    if (argc < 2)
    {
        printf("Usage: rps <server> <options> ...\n ");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints)); // la getaddrinfo vuole hints settato tutto a 0
    hints.ai_family = AF_UNSPEC;      // IPv4 ed IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP

    host_remoto = argv[1];
    servizio_remoto = "5000";
    if ((err = getaddrinfo(host_remoto, servizio_remoto, &hints, &res)) < 0)
    {
        perror("Errore getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // Prima connessione riuscita valida
    for (ptr = res; *ptr != NULL, ptr = ptr->ai_next)
    {
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
        {
            fprintf(stderr, "Socket creation failure\n");
            continue;
        }

        if (connect(sd, ptr->ai_address, ptr->ai_addrlen) == 0)
        {
            printf("\nConnessione avvenuta con successo!\n");
            break;
        }

        close(sd);
    }

    // se entro qui significa che non sono riuscito a connettermi a nulla
    if (ptr == NULL)
    {
        fprintf(stderr, "Errore nella risoluzione del nome!\n");
        exit(EXIT_FAILURE);
    }

    // dealloco res che non mi serve più
    freeaddrinfo(res);

    // inizializzo rxb_init
    rxb_init(&rxb, MAX_REQUEST_SIZE);

    for (;;)
    {
        char option[4096];

        /** INSERIMENTO dell'option */
        puts("Inserisci l'option per ps oppure '.' per uscire");
        if (fgets(option, sizeof(option), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        if (strcmp(option, ".\n") == 0)
        {
            break;
        }

        // Invio al server
        if (write_all(sd, option, strlen(option)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        // Leggo finchè non trovo la stringa di fine richiesta
        for (;;)
        {
            char response[MAX_REQUEST_SIZE];
            size_t response_len;

            /**
             * Inizializzo il buffer di risposta a zero e non uso l’ultimo byte,
             * così sono sicuro che il contenuto del buffer sarà sempre terminato dal
             * carattere nullo (\0). In questo modo posso interpretarlo come una stringa C.
             * Questa operazione deve essere fatta prima di leggere ogni nuova risposta.
             */
            memset(response, 0, sizeof(response));
            response_len = sizeof(response) - 1;

            if (rxb_readline(&rxb, sd, response, &response_len) < 0)
            {
                /**
                 * Se siamo qui significa che rxb_readline ha restituito <0, cioè EOF.
                 * L'EOF indica che il server ha chiuso la connessione.
                 * Per sicurezza si dealloca il buffer gestito da rxb con rxb_destroy(&rxb)
                 */
                rxb_destroy(&rxb);
                fprintf(stderr, "Connessione chiusa dal server!\n");
                exit(EXIT_FAILURE);
            }

#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)response, response_len) != NULL)
            {
                fprintf(stderr, "UTF-8 in risposta non valido!\n");
                close(sd);
                exit(EXIT_FAILURE);
            }
#endif

            puts(response);

            if (strcmp(response, "--- END REQUEST ---") == 0)
            {
                break;
            }
        }
    }

    close(sd);

    return 0;
}
