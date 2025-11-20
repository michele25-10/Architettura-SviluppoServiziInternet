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

/** SIGCHLD handler */
void handler(int signo)
{
    int status;

    (void)signo;

    /* handle terminated children */
    while (waitpid(-1, &status, WNOHANG) > 0)
        continue;
}

int main(int argc, char **argv)
{
    struct addrinfo hints, *res;
    int err, sd, ns, pid, on;
    struct sigaction sa;
    rxb_t rxb;

    if (argc != 2)
    {
        print("Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    /* Inizializza un insieme di seganli a vuoto */
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_RESTART; // dice al sistema di riavviare automaticamente alcune system call
    sa.sa_handler = handler;  // quale funzione chiamare quando arriva SIGCHLD

    /* Registra il gestore handler per il segnale SIGCHLD */
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) < 0)
    {
        fprintf(stderr, "Errore bind address\n");
        exit(EXIT_FAILURE);
    }

    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        perror("Errore creazione socket");
        exit(EXIT_FAILURE);
    }

    on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    freeadrrinfo(res);

    if (listen(sd, SOMAXCONN) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        printf("\nServer listening\n");

        if ((ns = accept(sd, NULL, NULL)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        if ((pid = fork()) < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            int status;
            int pid2;
            const char *end_request = "\n--- END REQUEST ---\n";
            rxb_t rxb;
            char option[MAX_REQUEST_SIZE];

            close(sd);

            /* Disabilito la gestione del segnale SIGCHLD*/
            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_DFL;

            if (sigaction(SIGCHLD, &sa, NULL) == -1)
            {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }

            rxb_init(&rxb, MAX_REQUEST_SIZE);

            for (;;)
            {
                size_t option_len;

                memset(option, 0, sizeof(option));
                option_len = sizeof(option) - 1;

                if (rxb_readline(&rxb, sd, option, &option_len) < 0)
                {
                    rxb_destroy(&rxb);
                    break;
                }

#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)option, option_len) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_SUCCESS);
                }
#endif

                if ((pid2 = fork()) < 0)
                {
                    perror("fork pid2");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0)
                {
                    close(stdout);

                    if (dup(ns) < 0)
                    {
                        perror("dup");
                        exit("EXIT_FAILURE");
                    }

                    close(ns);

                    if (strlen(option) == 0)
                    {
                        execlp("ps", "ps", (char *)NULL);
                        perror("execlp ps 1");
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        execlp("ps", "ps", option, (char *)NULL);
                        perror("execlp ps 2");
                        exit(EXIT_FAILURE);
                    }
                }

                // Attendo che invii la risposta
                wait(&status);

                if (write_all(ns, end_request, strlen(end_request)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }

            close(ns);
            exit(EXIT_SUCCESS);
        }

        /* padre */
        close(ns);
    }

    close(sd);
    return 0;
}