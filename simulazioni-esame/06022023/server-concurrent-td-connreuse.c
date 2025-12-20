#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#ifdef USE_LIBUNISTRING
#include <unistr.h>
#endif
#include "rxb.h"
#include "utils.h"

#define MAX_REQUEST_SIZE (64 * 1024)

void handler(int signo)
{
    int status;
    (void)signo;

    while (waitpid(-1, &status, WNOHANG) > 0)
        continue;
}

int main(int argc, char **argv)
{
    struct addrinfo hints, *res;
    int err, sd, ns, pid, on;
    struct sigaction sa;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./server <porta>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
    {
        fprintf(stderr, "Errore setup indirizzo bind %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        perror("Errore in socket");
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
        perror("Errore in bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if (listen(sd, SOMAXCONN) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        printf("Server in ascolto...\n");

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
            int status, pid1, pid2, p1p2[2];
            const char *end_request = "\n--- END REQUEST ---\n";
            rxb_t rxb;

            close(sd);

            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_DFL;
            if (sigaction(SIGCHLD, &sa, NULL) == -1)
            {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }

            rxb_init(&rxb, MAX_REQUEST_SIZE * 2);

            for (;;)
            {
                char nome_giocatore[MAX_REQUEST_SIZE], squadra[MAX_REQUEST_SIZE], anno[MAX_REQUEST_SIZE];
                char path[MAX_REQUEST_SIZE + 256];
                size_t len_nome_giocatore, len_squadra, len_anno;

                memset(nome_giocatore, 0, sizeof(nome_giocatore));
                len_nome_giocatore = sizeof(nome_giocatore) - 1;
                if (rxb_readline(&rxb, ns, nome_giocatore, &len_nome_giocatore) < 0)
                {
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)nome_giocatore, len_nome_giocatore) != NULL)
                {
                    fprintf(stderr, "Richiesta non è in formato UTF-8 valido!\n");
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                memset(squadra, 0, sizeof(squadra));
                len_squadra = sizeof(squadra) - 1;
                if (rxb_readline(&rxb, ns, squadra, &len_squadra) < 0)
                {
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)squadra, len_squadra) != NULL)
                {
                    fprintf(stderr, "Richiesta non è in formato UTF-8 valido!\n");
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                memset(anno, 0, sizeof(anno));
                len_anno = sizeof(anno) - 1;
                if (rxb_readline(&rxb, ns, anno, &len_anno) < 0)
                {
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)anno, len_anno) != NULL)
                {
                    fprintf(stderr, "Richiesta non è in formato UTF-8 valido!\n");
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                /* INIZIO DELLA LOGICA */
                snprintf(path, sizeof(path), "6_nazioni/%s.txt", squadra);

                if (pipe(p1p2) < 0)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                if ((pid1 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid1 == 0)
                {
                    close(p1p2[0]);
                    close(ns);
                    close(1);
                    if (dup(p1p2[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[1]);

                    execlp("grep", "grep", nome_giocatore, path, (char *)NULL);
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }

                close(p1p2[1]);

                if ((pid2 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0)
                {
                    close(0);
                    if (dup(p1p2[0]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[0]);

                    close(1);
                    if (dup(ns) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(ns);

                    execlp("grep", "grep", anno, (char *)NULL);
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }

                close(p1p2[0]);

                waitpid(pid1, &status, 0);
                waitpid(pid2, &status, 0);

                if (write_all(ns, end_request, strlen(end_request)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }

            close(ns);
            rxb_destroy(&rxb);
            exit(EXIT_SUCCESS);
        }

        close(ns);
    }

    close(sd);
    return 0;
}