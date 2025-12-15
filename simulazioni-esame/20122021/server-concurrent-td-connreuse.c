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
    int sd, err, pid, ns, on;
    struct addrinfo hints, *res;
    struct sigaction sa;

    if (argc != 2)
    {
        fprintf(stdout, "Usage: server <port>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;
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
        perror("getaddrinfo");
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
        exit(EXIT_FAILURE);
        perror("listen");
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
            /*PADRE*/
            int status, pid1, pid2;
            int p1p2[2];
            char categoria[1024], nome_produttore[1024], ordinamento[16];
            char *end_request = "\n--- END REQUEST ---\n";
            size_t len_categoria, len_nome_produttore, len_ordinamento;
            rxb_t rxb;

            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_IGN;
            if (sigaction(SIGCHLD, &sa, NULL) == -1)
            {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }

            rxb_init(&rxb, MAX_REQUEST_SIZE);

            for (;;)
            {
                memset(categoria, 0, sizeof(categoria));
                len_categoria = sizeof(categoria) - 1;
                if (rxb_readline(&rxb, ns, categoria, &len_categoria) == -1)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)categoria, len_categoria) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                memset(nome_produttore, 0, sizeof(nome_produttore));
                len_nome_produttore = sizeof(nome_produttore) - 1;
                if (rxb_readline(&rxb, ns, nome_produttore, &len_nome_produttore) == -1)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)nome_produttore, len_nome_produttore) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                memset(ordinamento, 0, sizeof(ordinamento));
                len_ordinamento = sizeof(ordinamento) - 1;
                if (rxb_readline(&rxb, ns, ordinamento, &len_ordinamento) == -1)
                {
                    perror("rxb_readline");
                    rxb_destroy(&rxb);
                    exit(EXIT_FAILURE);
                }
#ifdef USE_LIBUNISTRING
                if (u8_check((uint8_t *)ordinamento, len_ordinamento) != NULL)
                {
                    fprintf(stderr, "Request is not valid UTF-8!\n");
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                /**LOGICA BUSINESS */

                char path[4096];
                sprintf(path, "data/%s.txt", categoria);

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

                    execlp("grep", "grep", nome_produttore, path, (char *)NULL);
                    perror("execlp grep");
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

                    if (strcmp(ordinamento, "crescente") == 0)
                    {
                        execlp("sort", "sort", "-n", (char *)NULL);
                        perror("execlp sort");
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        execlp("sort", "sort", "-n", "-r", (char *)NULL);
                        perror("execlp sort");
                        exit(EXIT_FAILURE);
                    }
                }

                close(p1p2[0]);

                wait(&status);
                wait(&status);

                if (write_all(ns, end_request, strlen(end_request)) < 0)
                {
                    perror("write_all");
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