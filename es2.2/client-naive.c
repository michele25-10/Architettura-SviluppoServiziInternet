#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char buff[2048];
    int sd, err, nread;
    struct addrinfo hints, *ptr, *res;

    // Controllo del numero di argomenti: il client necessita di host, porta e due stringhe
    if (argc != 5)
    {
        fprintf(stderr, "Uso: %s hostname port string1 string2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Ignora il segnale SIGPIPE: evita la terminazione del processo
    // se si tenta di scrivere su una socket chiusa dal peer.
    signal(SIGPIPE, SIG_IGN);

    // Inizializzazione della struttura hints per getaddrinfo()
    // che risolve nome e servizio in indirizzi IP e porta.
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // accetta sia IPv4 che IPv6
    hints.ai_socktype = SOCK_STREAM; // socket di tipo TCP (affidabile, orientato alla connessione)

    // Risoluzione del nome host e porta in una lista di indirizzi utilizzabili
    err = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "Errore in getaddrinfo: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    // Tentativo di connessione ai possibili indirizzi (meccanismo di fallback)
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        // Creazione della socket secondo il dominio e il protocollo indicato
        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sd < 0)
            continue;

        // Connessione al server: se riesce, interrompe il ciclo
        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
            break;

        // In caso di errore nella connessione, chiude la socket e prova l'indirizzo successivo
        close(sd);
    }

    // Se nessuna connessione è andata a buon fine, termina con errore
    if (ptr == NULL)
    {
        fprintf(stderr, "Errore di connessione!\n");
        exit(EXIT_FAILURE);
    }

    // Libera la memoria allocata da getaddrinfo()
    freeaddrinfo(res);

    // Invio della prima stringa al server
    // write() scrive sul canale di comunicazione associato alla socket
    if (write(sd, argv[3], strlen(argv[3])) < 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    // Attesa dell’ACK (acknowledgment) dal server
    // read() blocca il processo fino alla ricezione di dati o chiusura della connessione
    if (read(sd, buff, sizeof(buff)) < 0)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // Invio della seconda stringa al server
    if (write(sd, argv[4], strlen(argv[4])) < 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    // Ricezione del risultato dal server.
    // Il ciclo continua finché read() restituisce un numero positivo di byte letti.
    while ((nread = read(sd, buff, sizeof(buff))) > 0)
    {
        // Scrive i dati ricevuti sullo standard output (file descriptor 1)
        if (write(1, buff, nread) < 0)
        {
            perror("write su stdout");
            exit(EXIT_FAILURE);
        }
    }

    // Controllo finale di eventuali errori nella lettura
    if (nread < 0)
    {
        perror("read result");
        exit(EXIT_FAILURE);
    }

    // Aggiunge un a capo finale per formattare l’output
    if (write(1, "\n", 1) < 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    // Chiusura ordinata della socket: rilascia la connessione TCP
    close(sd);

    return 0;
}
