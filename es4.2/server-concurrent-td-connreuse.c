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
    int sd, err, ns, pid, on;
    struct addrinfo hints, *res;
    struct sigaction sa;

    if (argc != 2)
    {
        fprintf(stdout, "Usage: server-connreuse-td-concurrent <porta>\n");
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
        fprintf(stderr, "Errore Resolution Naming\n");
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
            int pid2, pid3, status;
            int p2p3[2];
            char vine[1024], year[1024];
            char *end_response = "\n--- END RESPONSE ---\n";
            size_t len_vine, len_year;
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
                memset(vine, 0, sizeof(vine));
                len_vine = sizeof(vine) - 1;
                if (rxb_readline(&rxb, ns, vine, &len_vine) < 0)
                {
                    perror("read");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }

#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)vine, len_vine) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_SUCCESS);
                }
#endif

                memset(year, 0, sizeof(year));
                len_year = sizeof(year) - 1;
                if (rxb_readline(&rxb, ns, year, &len_year) < 0)
                {
                    perror("read");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }

#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)vine, len_vine) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_SUCCESS);
                }
#endif

                /** Da QUI parte la logica reale del server */
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
                    /* NIPOTE 1 */
                    close(ns);
                    close(p2p3[0]);
                    close(1);

                    if (dup(p2p3[1]) < 0)
                    {
                        perror("dup p2p3[1]");
                        exit(EXIT_FAILURE);
                    }
                    close(p2p3[1]);

                    execlp("grep", "grep", vine, "magazzino.txt", (char *)NULL);
                    perror("execlp grep 1");
                    exit(EXIT_FAILURE);
                }

                if ((pid3 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid3 == 0)
                {
                    close(p2p3[1]);
                    close(0);
                    if (dup(p2p3[0]) < 0)
                    {
                        perror("dup p2p3[0]");
                        exit(EXIT_FAILURE);
                    }
                    close(p2p3[0]);

                    close(1);
                    if (dup(ns) < 0)
                    {
                        perror("dup ns");
                        exit(EXIT_FAILURE);
                    }
                    close(ns);

                    execlp("grep", "grep", year, (char *)NULL);
                    perror("execlp grep 2");
                    exit(EXIT_FAILURE);
                }

                /** PADRE */
                close(p2p3[0]);
                close(p2p3[1]);

                wait(&status);
                wait(&status);

                if (write_all(ns, end_response, strlen(end_response)) < 0)
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