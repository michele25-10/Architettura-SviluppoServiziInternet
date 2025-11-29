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

void handler(int signo)
{
    int status;
    (void)signo;

    while (waitpid(-1, &status, WNOHANG) > 0)
    {
        continue;
    }
}

int main(int argc, char **argv)
{
    int pid, err, sd, on, ns;
    struct addrinfo hints, *res;
    struct sigaction sa;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: server-cuncurrent <porta>\n");
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

    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) < 0)
    {
        fprintf(stderr, "Errore risoluzione name\n");
        exit(EXIT_FAILURE);
    }

    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        perror("socket");
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

    freeaddrinfo(res);

    if (listen(sd, SOMAXCONN) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto\n");
    for (;;)
    {
        if ((ns = accept(sd, NULL, NULL)) < 0)
        {
            perror("accept");
            close(sd);
            exit(EXIT_FAILURE);
        }

        if ((pid = fork()) == -1)
        {
            perror("fork");
            continue;
        }
        else if (pid == 0)
        {
            /*FIGLIO */
            int pid2, status;
            char nazione[1024];
            char vacanza[1024];
            char budget[1024];
            size_t len_nazione;
            size_t len_vacanza;
            size_t len_budget;
            const char *END_RESPONSE = "\n--- END RESPONSE ---\n";
            char path[4096];
            rxb_t rxb;

            printf("Nuova connessione accettata!\n");

            close(sd);

            memset(&sa.sa_mask, 0, sizeof(sa.sa_mask));
            sa.sa_handler = SIG_DFL;

            if (sigaction(SIGCHLD, &sa, NULL) < 0)
            {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }

            rxb_init(&rxb, MAX_REQUEST_SIZE);

            for (;;)
            {
                /* Leggo NAZIONE */
                memset(nazione, 0, sizeof(nazione));
                len_nazione = sizeof(nazione) - 1;
                if (rxb_readline(&rxb, ns, nazione, &len_nazione) < 0)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)nazione, len_nazione) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                if (strcmp(nazione, "fine") == 0)
                {
                    break;
                }

                /**Leggo VACANZA */
                memset(vacanza, 0, sizeof(vacanza));
                len_vacanza = sizeof(vacanza) - 1;
                if (rxb_readline(&rxb, ns, vacanza, &len_vacanza) < 0)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)vacanza, len_vacanza) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                /**Leggo il BUDGET */
                memset(budget, 0, sizeof(budget));
                len_budget = sizeof(budget) - 1;
                if (rxb_readline(&rxb, ns, budget, &len_budget) < 0)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)budget, len_budget) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                /** HO LETTO TUTTE QUANTE LE INFORMAZIONI ORA DEVO ESEGUIRE LA LOGICA */
                snprintf(path, sizeof(path), "holiday_packages/%s/%s.txt", nazione, vacanza);

                if ((pid2 = fork()) == -1)
                {
                    perror("fork");
                    close(ns);
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0)
                {
                    /* NIPOTE */
                    fclose(stdout);

                    if (dup(ns) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }

                    close(ns);

                    execlp("grep", "grep", "v", path, (char *)NULL);
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }

                /** PADRE */
                wait(&status);

                if (write_all(ns, END_RESPONSE, sizeof(END_RESPONSE)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }

            rxb_destroy(&rxb);
            close(ns);
            exit(EXIT_SUCCESS);
        }

        /* NONNO */
        close(ns);
    }

    close(sd);

    return 0;
}