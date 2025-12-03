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

    while (waitpid(-1, &status, WNOHANG) == -1)
        continue;
}

int main(int argc, char **argv)
{
    int sd, ns, err, on, pid;
    struct addrinfo hints, *res;
    struct sigaction sa;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: server-concurrent-td-connreuse <porta>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) < 0)
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
        fprintf(stderr, "Errore risoluzione del naming\n");
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

    fprintf(stdout, "Server in ascolto!\n");

    for (;;)
    {
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
            /* FIGLIO*/
            char regione[1024], n_localita[1024], path[2048];
            size_t len_regione, len_n_localita;
            char *end_request = "\n--- END REQUEST ---\n";
            int pid2, pid3, pid4, status;
            int p2p3[2], p3p4[2];
            rxb_t rxb;

            close(sd);

            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_IGN;

            if (sigaction(SIGCHLD, &sa, NULL) < 0)
            {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }

            rxb_init(&rxb, MAX_REQUEST_SIZE);

            for (;;)
            {
                memset(regione, 0, sizeof(regione));
                len_regione = sizeof(regione) - 1;
                if (rxb_readline(&rxb, ns, regione, &len_regione) < 0)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)regione, len_regione) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_SUCCESS);
                }
#endif

                memset(n_localita, 0, sizeof(n_localita));
                len_n_localita = sizeof(n_localita) - 1;
                if (rxb_readline(&rxb, ns, n_localita, &len_n_localita) < 0)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)n_localita, len_n_localita) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_SUCCESS);
                }
#endif

                /** LOGICA BUSINESS del server */
                sprintf(path, "data/%s.txt", regione);
                path[strlen(path)] = '\0';

                if (pipe(p2p3) < 0)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                if ((pid2 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0)
                {
                    close(p2p3[0]);
                    close(ns);
                    close(1);

                    if (dup(p2p3[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p2p3[1]);

                    execlp("head", "head", "-n", n_localita, path, (char *)NULL);
                    perror("head");
                    exit(EXIT_FAILURE);
                }

                close(p2p3[1]);

                if (pipe(p3p4) < 0)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                if ((pid3 = fork()) < 0)
                {
                    perror("fork pid3");
                    exit(EXIT_FAILURE);
                }
                else if (pid3 == 0)
                {
                    close(p3p4[0]);
                    close(ns);
                    close(0);
                    if (dup(p2p3[0]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p2p3[0]);

                    close(1);
                    if (dup(p3p4[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p3p4[1]);

                    execlp("sort", "sort", "-r", (char *)NULL);
                    perror("sort");
                    exit(EXIT_FAILURE);
                }

                close(p2p3[0]);
                close(p3p4[1]);

                if ((pid4 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid4 == 0)
                {
                    char buf[MAX_REQUEST_SIZE], res[1024];
                    int sum = 0, count = 0;
                    float avg;

                    while (read(p3p4[0], buf, sizeof(buf)) > 0)
                    {
                        if (write_all(ns, buf, strlen(buf)) < 0)
                        {
                            perror("write_all");
                            exit(EXIT_FAILURE);
                        }

                        char *token = strtok(buf, ",");
                        if (token)
                            sum += atoi(token);

                        count++;
                    }

                    avg = sum / count;

                    sprintf(res, "MEDIA: %f\n", avg);
                    if (write_all(ns, res, strlen(res)) < 0)
                    {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                }

                close(p3p4[0]);

                wait(&status);
                wait(&status);
                wait(&status);

                /** INVIO FINE RISPOSTA */
                if (write_all(ns, end_request, strlen(end_request)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }

            close(ns);
            exit(EXIT_SUCCESS);
        }

        /* PADRE */
        close(ns);
    }

    close(sd);
    return 0;
}