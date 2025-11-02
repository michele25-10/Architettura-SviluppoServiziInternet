#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Gestore del segnale SIGCHLD: serve a evitare la creazione di processi zombie
void handler(int signo)
{
    int status;
    (void)signo; // Evita warning per parametro inutilizzato

    // Chiamata non bloccante per attendere i figli terminati
    // WNOHANG impedisce che il padre resti sospeso se non ci sono figli da attendere
    while (waitpid(-1, &status, WNOHANG) > 0)
        continue;
}

int main(int argc, char **argv)
{
    int sd, err, on;
    struct addrinfo hints, *res;
    struct sigaction sa;

    // Controllo dei parametri: il server richiede solo il numero di porta
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Ignora SIGPIPE per evitare terminazione in caso di scrittura su socket chiusa
    signal(SIGPIPE, SIG_IGN);

    // Installazione di un gestore di segnali robusto tramite sigaction()
    // Sigaction è preferibile a signal() perché conforme a POSIX e con comportamento prevedibile
    sigemptyset(&sa.sa_mask); // Nessun segnale bloccato durante l’esecuzione dell’handler
    sa.sa_flags = SA_RESTART; // Riavvia automaticamente le system call interrotte da segnali
    sa.sa_handler = handler;  // Funzione gestore associata al segnale

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Preparazione dei parametri per getaddrinfo()
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // Supporta sia IPv4 che IPv6
    hints.ai_socktype = SOCK_STREAM; // Socket TCP (connessione affidabile)
    hints.ai_flags = AI_PASSIVE;     // Usata per socket server (accetta connessioni in ingresso)

    // Risoluzione dell'indirizzo locale e della porta
    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
    {
        fprintf(stderr, "Errore nella configurazione dell’indirizzo: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    // Creazione della socket di ascolto
    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        perror("Errore creazione socket");
        exit(EXIT_FAILURE);
    }

    // Abilita il riutilizzo immediato della porta dopo la chiusura (evita TIME_WAIT)
    on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Associa la socket alla porta specificata (fase di BIND)
    if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("Errore nel bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res); // Libera la memoria allocata da getaddrinfo

    // Trasforma la socket in socket passiva (LISTEN)
    // listen() prepara una coda di connessioni pendenti, tipica del server TCP
    if (listen(sd, SOMAXCONN) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Ciclo principale del server: accetta e gestisce più client
    for (;;)
    {
        int ns, pid, nread;

        // accept() blocca il processo finché arriva una nuova richiesta di connessione
        if ((ns = accept(sd, NULL, NULL)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Creazione di un processo figlio per gestire il client
        // fork() duplica il processo corrente: il figlio eredita la socket connessa
        if ((pid = fork()) < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // === PROCESSO FIGLIO ===
            char str1[4096], str2[4096], response[80];
            const char *ack = "ACK";

            // Il figlio chiude la socket passiva: non gli serve accettare altre connessioni
            close(sd);

            // Lettura della prima stringa inviata dal client
            memset(str1, 0, sizeof(str1));
            if ((nread = read(ns, str1, sizeof(str1) - 1)) < 0)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }

            // Invio di un messaggio di conferma (ACK)
            if (write(ns, ack, strlen(ack)) < 0)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }

            // Lettura della seconda stringa dal client
            memset(str2, 0, sizeof(str2));
            if ((nread = read(ns, str2, sizeof(str2) - 1)) < 0)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }

            // Confronto tra le due stringhe e preparazione della risposta
            if (strcmp(str1, str2) == 0)
                strncpy(response, "YES", sizeof(response));
            else
                strncpy(response, "NO", sizeof(response));

            // Invio del risultato al client
            if (write(ns, response, strlen(response)) < 0)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }

            // Chiusura della socket di comunicazione e terminazione del figlio
            close(ns);
            exit(EXIT_SUCCESS);
        }

        // === PROCESSO PADRE ===
        // Il padre chiude la socket attiva: continuerà ad accettare nuove connessioni
        close(ns);
    }

    // Chiusura della socket di ascolto (teoricamente mai raggiunta)
    close(sd);

    return 0;
}
