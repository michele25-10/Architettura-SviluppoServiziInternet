#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
        int sd, err, nread;
        struct addrinfo hints, *res, *ptr;
        char line[4096], len[2048];

        /* Parameter check */
        if (argc != 3)
        {
                fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        /* Ignora SIGPIPE
        Quando un client scrive su un socket chiuso dal server il kernel invia SIGPIPE:
        - Comportamento di default --> terminazione del processo
        - Qui si vuole evitare crash improvvisi
        */
        signal(SIGPIPE, SIG_IGN);

        /* Prepare getaddrinfo */
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC; // Compatibilità dual-stack IPv4/IPv6
        hints.ai_socktype = SOCK_STREAM;

        /* Invoke getaddrinfo */
        err = getaddrinfo(argv[1], argv[2], &hints, &res);
        if (err != 0)
        {
                fprintf(stderr, "Name resolution error: %s\n", gai_strerror(err));
                exit(2);
        }

        /* Read input */
        printf("Enter string:\n");
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
                perror("fgets");
                exit(EXIT_FAILURE);
        }

        /* fgets inserts also \n into the buffer,
        so I replace it with a null byte (string terminator \0) */
        line[strlen(line) - 1] = '\0';

        while (strcmp(line, "fine") != 0)
        {
                /* Fallback connection
                un hostname può avere:
                - più indirizzi IPv4
                - più indirizzi IPv6
                - indirizzi temporaneamente non raggiungibili
                Se un indirizzo fallisce tento con il successivo, nonappena uno funziona esco dal loop
                */
                for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
                {
                        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
                        /* If the socket fails, move to the next address */
                        if (sd < 0)
                                continue;

                        /* If connect is successful, exit the loop */
                        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
                                break;

                        /* Otherwise, close the socket and move to the next address */
                        close(sd);
                }

                /* Control connection */
                /* Significa che ho esaurito tutti gli indirizzi senza riuscire a connettermi */
                if (ptr == NULL)
                {
                        fprintf(stderr, "Errore di connessione!\n");
                        exit(EXIT_FAILURE);
                }

                /* Invio al server:
                Il server deve sapere:
                - la lunghezza prevista, oppure
                - che st aleggendo fino a un delimitatore, oppure
                - che la prima stringa finisce quando invia l'ACK
                */
                if (write(sd, line, strlen(line)) < 0)
                {
                        perror("write");
                        exit(EXIT_FAILURE);
                }

                /* Lettura dal server */
                memset(len, 0, sizeof(len));
                nread = read(sd, len, sizeof(len) - 1);
                if (nread < 0)
                {
                        perror("read");
                        exit(EXIT_FAILURE);
                }
                printf("Number of characters in the input string: %s\n", len);

                /* Chiusura socket --> fine della sessione TCP */
                close(sd);

                /* Read new input */
                printf("Enter string:\n");
                if (fgets(line, sizeof(line), stdin) == NULL)
                {
                        perror("fgets");
                        exit(EXIT_FAILURE);
                }

                /* fgets inserts also \n into the buffer,
                so I replace it with a null byte (string terminator \0) */
                line[strlen(line) - 1] = '\0';
        }

        /* At this point, I can free the memory allocated by getaddrinfo */
        freeaddrinfo(res);

        return 0;
}
