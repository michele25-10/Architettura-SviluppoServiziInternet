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

/**
 * Il protocollo LPD è un riferimento al protocollo storico LPD (Line Printer Daemon), usato nei sistemi UNIX per
 * inviare dati a un print server.
 *
 * Il protocollo LPD ha alcune caratteristiche precise:
 * - E' TCP-based
 * - Usa lunghezze e comandi codificati binariamente
 * - Le lunghezze sono espresse in network byte order
 * - Fa parte dei protocolli considerati semplici ma rigorosi
 *
 * Questo esercizio NON implementa davvero LPD ma si ispira al suo stile di protocollo:
 * - si inviano stringhe con lunghezza codificata a 16 bit
 * - si usano formati binari e non semplici stringhe testuali
 * - si usa un pattern "send length --> send data" tipico dei protocolli di stampa
 *
 * Per questo motivo è stato chiamato client-lpd.c
 *  --> per richiamare uno stile di protocollo "alla LPD", con messaggi preceduti da lunghezza e strutturata ben definita.
 */

int main(int argc, char **argv)
{
        char *stringa1, *stringa2;
        size_t dim_stringa1, dim_stringa2;
        uint8_t buff[2048];
        uint8_t len[2];
        int sd, err, nread;
        struct addrinfo hints, *ptr, *res;

        if (argc != 5)
        {
                fprintf(stderr, "Sintassi: rstrcmp hostname porta stringa1 stringa2\n");
                exit(EXIT_FAILURE);
        }

        /* Ignoro SIGPIPE sempre per evitare crash nel caso in cui il client
        scriva su una socket chiusa dal server*/
        signal(SIGPIPE, SIG_IGN);

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;       // Forzo IPv4
        hints.ai_socktype = SOCK_STREAM; // TCP

        err = getaddrinfo(argv[1], argv[2], &hints, &res);
        if (err != 0)
        {
                fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

        /* connessione con fallback */
        for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
        {
                sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
                /* se la socket fallisce, passo all'indirizzo successivo */
                if (sd < 0)
                        continue;

                /* se la connect va a buon fine, esco dal ciclo */
                if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
                        break;

                /* altrimenti, chiudo la socket e passo all'indirizzo
                 * successivo */
                close(sd);
        }

        /* controllo che effettivamente il client sia connesso */
        if (ptr == NULL)
        {
                fprintf(stderr, "Errore di connessione!\n");
                exit(EXIT_FAILURE);
        }

        /* a questo punto, posso liberare la memoria allocata da getaddrinfo */
        freeaddrinfo(res);

        /**
         * Perchè limite a 65535?
         * Perchè la lunghezza viene codificata in un intero uint16_t, cioè 2 byte -> max = 65535
         */
        stringa1 = argv[3];
        dim_stringa1 = strlen(stringa1);
        if (dim_stringa1 > UINT16_MAX)
        {
                fprintf(stderr, "Stringa 1 troppo grande (massimo %d byte)!\n", UINT16_MAX);
                exit(EXIT_FAILURE);
        }

        stringa2 = argv[4];
        dim_stringa2 = strlen(stringa2);
        if (dim_stringa2 > UINT16_MAX)
        {
                fprintf(stderr, "Stringa 2 troppo grande (massimo %d byte)!\n", UINT16_MAX);
                exit(EXIT_FAILURE);
        }

        /** Codifico lunghezza stringa1 come intero unsigned a 16 bit in formato
         * big endian (AKA network byte order)
         * Questa è la forma manuale di: uint16_t -> network byte ordedr (big endian)
         * Il network byte order è sempre big endian, indipendentemente dall'architettura
         */
        len[0] = (dim_stringa1 & 0xFF00) >> 8;
        len[1] = (dim_stringa1 & 0xFF);

        /**
         * Perchè write_all() e non write()?
         * Perchè write() NON garantisce di inviare tutti i byte richiesti.
         * TCP è uno stram, non un message protocol.
         * write() può inviare metà dei dati.
         *
         * write_all() è una funzione che:
         * - richiama write()
         * - se non invia tutto, ripete
         * - until all bytes are written
         *
         * Questa è una best practice assoluta nei protocolli TCP
         */

        /* Invio lunghezza prima stringa */
        if (write_all(sd, len, 2) < 0)
        {
                perror("write");
                exit(EXIT_FAILURE);
        }

        /* Invio prima stringa */
        if (write_all(sd, stringa1, dim_stringa1) < 0)
        {
                perror("write");
                exit(EXIT_FAILURE);
        }

        /* Codifico lunghezza stringa2 come intero unsigned a 16 bit in formato
         * big endian (AKA network byte order) */
        len[0] = (dim_stringa2 & 0xFF00) >> 8;
        len[1] = (dim_stringa2 & 0xFF);

        /* Invio lunghezza seconda stringa */
        if (write_all(sd, len, 2) < 0)
        {
                perror("write");
                exit(EXIT_FAILURE);
        }

        /* Invio seconda stringa */
        if (write_all(sd, stringa2, dim_stringa2) < 0)
        {
                perror("write");
                exit(EXIT_FAILURE);
        }

        /**
         * Il client riceve fino a EOF (chiusura della connessione del server)
         * Poi stampa a stdout
         */
        while ((nread = read(sd, buff, sizeof(buff))) > 0)
        {
                if (write_all(1, buff, nread) < 0)
                {
                        perror("write");
                        exit(EXIT_FAILURE);
                }
        }

        /* Controllo errori di lettura */
        if (nread < 0)
        {
                perror("read");
                exit(EXIT_FAILURE);
        }

        /* Stampo un \n prima di terminare */
        if (write(1, "\n", 1) < 0)
        {
                perror("write");
                exit(EXIT_FAILURE);
        }

        /* Chiudo la socket */
        close(sd);

        return 0;
}
