#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/*
Server TCP iterativo von vonnessioni transient:
- iterativo: gestisce un client alla volta in sequenza.
- Transient: ogni connessione viene aperta, gestita e chiusa.
- Protocollo applicativo: il client manda una stringa, il server risponde con la lunghezza.
*/

int main(int argc, char **argv)
{
        int sd, err, on;
        struct addrinfo hints, *res;

        /* Parameter check */
        if (argc != 2)
        {
                fprintf(stderr, "Usage: %s port\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        /* Ignora segnale SIGPIPE:
        Se il client chiude il socket e il server prova a scriver --> SIGPIPE --> default = kill del server.
        Ignorandolo evita la terminazione del processo.
        */
        signal(SIGPIPE, SIG_IGN);

        /* Prepare getaddrinfo */
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;     // compatibile IPv4 e IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_flags = AI_PASSIVE;     // produce un indirizzo "bindabile" (equivalende a 0.0.0.0 o ::)

        /* Passando NULL come hostname, AI_PASSIVE fa si che il server si vincoli a tutte le interfacce di rete. */
        if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
        {
                fprintf(stderr, "Error setting up bind address: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

        /* Crea un passivo (listening) socket TCP nella famiglia fornita da getaddrinfo (IPv4 o IPv6) */
        if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
                perror("Error creating socket");
                exit(EXIT_FAILURE);
        }

        /* Serve per:
        - permettere di riavviare il server subito dopo la chiusura
        - evitare errori "address already in use"

        Altrimenti TCP mantiene la porta in stato TIME_WAIT per 2 minuti
        */
        on = 1;
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        {
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }

        /* Il socket viene associato:
        - all'indirizzo IP scelto (tutti, con AI_PASSIVE)
        - alla porta indicata da argv[1]
        Se fallisce --> molto probabilmente la porta è occupata (o permessi insufficienti)
        */
        if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
        {
                perror("Error binding socket");
                exit(EXIT_FAILURE);
        }

        /* At this point, I can free the memory allocated by getaddrinfo */
        freeaddrinfo(res);

        /*
        Trasformo il socket in listening socket: riceve richieste di connessione.
        SOMAXCONN è la massima dimensione possibile della coda di connessioni pendenti consentita dal sistema.
        */
        if (listen(sd, SOMAXCONN) < 0)
        {
                perror("listen");
                exit(EXIT_FAILURE);
        }

        /* Il server entra nel loop delle accettazioni: un client alla volta! */
        for (;;)
        {
                char request[4096], response[256];
                int ns, nread, length_of_string;

                /*
                - blocca finchè arriva un client
                - crea un nuovo socket attivo (ns)
                - il socket passivo (sd) rimane in ascolto

                Gestione EINTR:
                se arriva un segnale --> accept() fallisce con EINTR --> si deve ripetere l'operazione
                Questo è lo standard POSIX
                */
                ns = accept(sd, NULL, NULL);
                if (ns < 0)
                {
                        /* I have not installed SIGCHLD handler with SA_RESTART,
                         * so I have to explicitly check and handle the
                         * EINTR case */
                        if (errno == EINTR)
                                continue;
                        perror("accept");
                        exit(EXIT_FAILURE);
                }

                /**
                 * Il buffer viene azzerato per garantire che sia sempre null-terminated:
                 * - utile per usare strlen(request)
                 * - evita problemi se il client non manda \0
                 */
                memset(request, 0, sizeof(request));

                /*
                Il server legge al massimo 4095 byte, lasciando 1 spazio per \0
                Il client può inviare una stringa arbitraria, la prima parte verrà presa come la richiesta
                */
                if ((nread = read(ns, request, sizeof(request) - 1)) < 0)
                {
                        perror("read");
                        close(ns);
                        continue;
                }

                /* String length computation */
                length_of_string = strlen(request);

                /* Prepare the response buffer */
                snprintf(response, sizeof(response), "%d\n", length_of_string);

                /*
                Invio la risposta:
                Se il client ha chiuso --> write fallisce con EPIPE --> ma SIGPIPE è ignorato, quindi il server continua a funzionare
                */
                if (write(ns, response, strlen(response)) < 0)
                {
                        perror("write");
                        close(ns);
                        continue;
                }

                /*
                Chiusura della scoket: torna ad attendere il prossimo client
                Questo è un server transient: una connessione --> una richiesta --> una risposta --> chiudi
                */
                close(ns);
        }

        /* Close passive socket (just in case) */
        close(sd);

        return 0;
}
