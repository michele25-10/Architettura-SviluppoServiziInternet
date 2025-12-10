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
    {
        continue;
    }
}

int main(int argc, char **argv)
{
    int sd, err, on, pid, ns;
    struct addrinfo hints, *res;
    struct sigaction sa;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: server-concurrent-td-connreuse <port>\n");
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

    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) == -1)
    {
        fprintf(stderr, "Errore nella risoluzione del naming\n");
        exit(EXIT_FAILURE);
    }

    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(sd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if (listen(sd, SOMAXCONN) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server in ascolto!\n");

    for (;;)
    {
        if ((ns = accept(sd, NULL, NULL)) == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        if ((pid = fork()) == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            char month[7], n[64];
            char *end_request = "\n--- END REQUEST ---\n";
            size_t len_month, len_n;
            int pid1, pid2, status, p1p2[2];
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

            rxb_init(&rxb, MAX_REQUEST_SIZE);

            for (;;)
            {
                memset(month, 0, sizeof(month));
                len_month = sizeof(month) - 1;
                if (rxb_readline(&rxb, ns, month, &len_month) == -1)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)month, len_month) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                memset(n, 0, sizeof(n));
                len_n = sizeof(n) - 1;
                if (rxb_readline(&rxb, ns, n, &len_n) == -1)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)n, len_n) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                /**Ora inizia la logica BUSINESS LOGIC */
                char path[1024];
                sprintf(path, "data/%s/cancelleria.txt", month);
                path[strlen(path)] = '\0';

                if (pipe(p1p2) == -1)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                if ((pid1 = fork()) == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid1 == 0)
                {
                    close(ns);
                    close(p1p2[0]);
                    close(1);

                    if (dup(p1p2[1]) == -1)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[1]);

                    execlp("sort", "sort", "-n", "-r", path, (char *)NULL);
                    exit(EXIT_FAILURE);
                    perror("execlp sort");
                }

                close(p1p2[1]);

                if ((pid2 = fork()) == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0)
                {
                    close(0);
                    if (dup(p1p2[0]) == -1)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[0]);

                    close(1);
                    if (dup(ns) == -1)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(ns);

                    execlp("head", "head", "-n", n, (char *)NULL);
                    perror("head");
                    exit(EXIT_FAILURE);
                }

                wait(&status);
                wait(&status);

                if (write_all(ns, end_request, strlen(end_request)) == -1)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }

            exit(EXIT_SUCCESS);
            close(ns);
        }

        close(ns);
    }

    close(sd);
    return 0;
}